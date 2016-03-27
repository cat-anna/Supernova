#include "kernel.h"
#include <headers/Loader.h>
#include <Supernova/di.h>

VFSNode_t VFSDevice;
VFSNode_t VFSDriver;
SPINLOCK DrvTables;
SPINLOCK DevTables;

static sint32 GetFreeDriverEntry(driver_p *drv);
static sint32 GetFreeDeviceEntry(device_p *dev);

uint32 GetDeviceName(uint32 DevID, char *NameBuf){
	if(!NameBuf)return ERRORCODE_WRONG_INPUT;
	device_p dev = CheckDevID(DevID);
	if(!dev) return EC_NO_SUCH_DEVICE;
	strcpy(NameBuf, dev->Name);
	return SUCCES;
}

uint32 InternalMessage(uint32 DrvID, uint32 msg, uint32 hparam, uint32 lparam){
	driver_p drv = CheckDrvID(DrvID);
	message_t m;
	m.message = msg;
	m.lparam = lparam;
	m.hparam = hparam;
	//kprintf("InternalMessage %s|%x\n", drv->proc->name, drv->proc->IPCBuf);
	if(drv->proc == CurrentThread->OwnerProcess)
		return IPC_BufferMessage(&m, drv->proc);//sending message to itself, no queuing is needed
	else
		return IPC_QueueMessage(&m, drv->proc);

//	if(state == STATE_SLEEP_MESSAGE)...
}

uint32 DirectSeek(HANDLE handle, uint32 offset, uint32 method){
	handle_p h = CheckHandle(handle, HANDLE_TYPE_DEVICE, 0);
	if(!h)return EC_WRONG_HANDLE;
	device_p dev = h->data.device;
	if(!dev->Functions.Seek)return EC_FUNCTION_NOT_SUPPORTED;
	uint32 NewPos = h->position;

	uint32 ret = dev->Functions.Seek(dev->DevID, h->param, offset, method, &NewPos);
//	kprintf("dev: %s %d\n", __FUNCTION__, NewPos);
	h->position = NewPos;
	return ret;
}

HANDLE DirectOpen(uint32 DrvID, const char* Dev, uint32 Mode){
	driver_p drv = CheckDrvID(DrvID);
	if(!drv)return INVALID_HANDLE_VALUE;

	char *Param;
	VFSNode_p node = VFS_GetNodeByPath(&VFSRoot, Dev, &Param);
	if(!node)return INVALID_HANDLE_VALUE;

	if(node->type != VFS_TYPE_DEVICE) return 0;

	device_p dev = (device_p)node->owner_data;
	if(!dev->Functions.Open)return INVALID_HANDLE_VALUE;

	process_p proc = (process_p)node->owner_pid;
	handle_p h = GetFreeHandleEntry(proc);
	if(!h)return INVALID_HANDLE_VALUE;

	uint32 DevHandle, res;
	res = dev->Functions.Open(dev->DevID, &DevHandle, Param, Mode);
	if(res != SUCCES){
		h->flags = 0;
		return INVALID_HANDLE_VALUE;
	}
	h->flags |= dev->FunctionsDump & FUNCTIONS_MASK;
	h->position = 0;
	h->param = DevHandle;
	h->type = HANDLE_TYPE_DEVICE;
	h->data.device = dev;
	return h;
}

uint32 DirectCommand(HANDLE handle, uint32 command, uint32 hparam, uint32 lparam){
	handle_p h = CheckHandle(handle, HANDLE_TYPE_DEVICE, 0);
	if(!h)return EC_WRONG_HANDLE;
	device_p dev = h->data.device;
	if(!dev->Functions.Command)return EC_FUNCTION_NOT_SUPPORTED;
	return dev->Functions.Command(dev->DevID, h->param, command, hparam, lparam);
}

uint32 DirectRead(HANDLE handle, void* buffer, uint32 count, uint32 *read){
	handle_p h = CheckHandle(handle, HANDLE_TYPE_DEVICE, 0);
	if(!h)return EC_WRONG_HANDLE;
	device_p dev = h->data.device;
	if(!dev->Functions.Read)return EC_FUNCTION_NOT_SUPPORTED;
//	kprintf("dev: %s %d\n", __FUNCTION__, h->position);
	uint32 ret = dev->Functions.Read(dev->DevID, h->param, h->position, buffer, count, read);
	h->position += *read;
	return ret;
}

uint32 RegisterIRQ(uint32 IRQ, IRQHandler_p Handler){
	//check whether requesting process has permission to allocate irq handler
	return SetIRQHandler(IRQ, Handler);
}

uint32 RegisterDevice(uint32 DriverID, DeviceInfo_p DevInfo){
	if(!DevInfo) return ERRORCODE_FATAL;

	driver_p drv = DriversTable + DriverID;
	if(!(drv->Flags & DRIVER_ENTRY_VALID)){
		//close requesting process
		return ERRORCODE_FATAL;
	}

	device_p dev;
	sint32 DevID = GetFreeDeviceEntry(&dev);
	if(DevID == -1){
		return ERRORCODE_FATAL;
	}
	dev->Flags = DEVICE_ENTRY_VALID;
	dev->DevType = DevInfo->DevType;
	dev->DevID = DevID;
	DevInfo->DevID = DevID;
	dev->Driver = drv;
	strcpy(dev->Name, DevInfo->Name);
	memcpy(&dev->Functions, &DevInfo->Functions, sizeof(DeviceFunction_t));

	TestFunction(dev, Open, dev, FUNCTION_OPEN);
	TestFunction(dev, Close, dev, FUNCTION_CLOSE);
	TestFunction(dev, Read, dev, FUNCTION_READ);
	TestFunction(dev, Write, dev, FUNCTION_WRITE);
	TestFunction(dev, Seek, dev, FUNCTION_SEEK);
	TestFunction(dev, Command, dev, FUNCTION_COMMAND);

	VFSNode_p node = (VFSNode_p)kmalloc(sizeof(VFSNode_t));
	memset(node, 0, sizeof(VFSNode_t));
	node->Name = dev->Name;
	node->flags = VFS_FLAG_SYSTEM;
	node->type = VFS_TYPE_DEVICE;
	node->owner_data = (uint32)dev;
	node->owner_pid = (uint32)drv->proc;
	VFS_InsertChildren(node, &VFSDevice);

	KernelMessage(MSG_NEW_DEVICE, (uint32)dev, 0, DevMgrIPC);

	return SUCCES;
}

uint32 RegisterDriver(uint32 DID, DriverFunctions_p DrvFun, const char* Name, uint32 DrvInfo){
	AcquireLock(&DrvTables);
	driver_p drv = DriversTable + DID;
	if(!(drv->Flags & DRIVER_ENTRY_VALID)){
		ReleaseLock(&DrvTables);
		//close driver
		return ERRORCODE_FATAL;
	}
	ReleaseLock(&DrvTables);
	strcpy(drv->Name, Name);
	memcpy(&drv->Functions, DrvFun, sizeof(DriverFunctions_t));
	drv->Info = DrvInfo;

	TestFunction(drv, Initialize, drv, FUNCTION_INITIALIZE);
	TestFunction(drv, Finalize, drv, FUNCTION_FINALIZE);
	TestFunction(drv, Reset, drv, FUNCTION_RESET);
	TestFunction(drv, Scan, drv, FUNCTION_SCAN);
	TestFunction(drv, Command, drv, FUNCTION_COMMAND);

	//kprintf("d:%s->%x->%x\n", drv->Name, drv->Functions.Initialize, drv->Functions.Scan);

	if(drv->Functions.Initialize)drv->Functions.Initialize();
	if(drv->Functions.Scan)drv->Functions.Scan();

	/*DID = DrvInfo = 0;
	DrvFun = 0;
	Name = 0;*/
	return SUCCES;
}

static void DevMgr_NewDevice(device_p newdev){
	if(!newdev)return;
	if(!CheckFunctions(newdev, FUNCTIONS_OPEN_CLOSE_READ_WRITE_SEEK))return;

	if(newdev->DevType == DEVICE_REMOVABLE_DRIVE){
		char *buf = 0;
		VFSNode_p n = VFSDriver.children;
		for(; n; n = n->next){
			driver_p drv = (driver_p)n->owner_data;
			if(!(drv->Info & DRIVER_INFO_FILE_SYSTEM))continue;
			if(!buf){
				buf = (char*)kmalloc(512);
				uint32 read, handle, result;
				result = newdev->Functions.Open(newdev->DevID, &handle, 0, 0);
				if(result != SUCCES)break;
				uint32 tmp;
				result = newdev->Functions.Seek(newdev->DevID, handle, 0, SEEK_BEGINING, &tmp);
				if(result != SUCCES)break;
				result = newdev->Functions.Read(newdev->DevID, handle, 0, buf, 512, &read);
//				kprintf("READ: %x %s\n", *(uint32*)buf, buf + 0x2B);
				newdev->Functions.Close(newdev->DevID, handle);
				if(result != SUCCES || read != 512)break;
			}
			if(drv->Functions.Command){
				uint32 res = drv->Functions.Command(COMMAND_FILESYSTEM_DETECT,(uint32)buf,newdev->DevID);
				if(res == SUCCES) break;
			}
		}
		if(buf)	kfree(buf);
	}
}

uint32 Supernova_DeviceManager(void){
	uint32 i;
	uint32 max = DataFromLoader->LoadedFilesCount;
//load internal drivers
	for(i = 0; i < max; ++i){
		char *ext = strrchr(DataFromLoader->LoadedFiles[i].Name, '.');
		if(!ext)continue;
		if(strcasecmp(ext, ".drv"))continue;
		uint32 err = InstallDriver(DataFromLoader->LoadedFiles[i].FilePtr, DataFromLoader->LoadedFiles[i].Name);
		if(!err)continue;
		kprintf("%4t\tAn error (code: 0x%x) has occur while loading file %s%t\n", err, DataFromLoader->LoadedFiles[i].Name);
	}

	while(1){
		if(IPC_GetCount(DevMgrIPC) == 0){
			int0_Sleep(100);
			continue;
		}

		message_t msg;
		IPC_GetMessage(&msg, DevMgrIPC);

		switch(msg.message){
		case MSG_NEW_DEVICE:
			DevMgr_NewDevice((device_p)msg.hparam);
			break;
		}
	}
	return SUCCES;
}

uint32 InstallDriver(void *Data, const char *Name){
	process_p proc;
	uint32 Drv_PD = CreateSecuredPageDirectory();
	BeginProcess(&proc, Name, Drv_PD);
	thread_p th;
	BeginThread(&th, proc, PRIORITY_DRIVER);
	driver_p drv = 0;
	sint32 DriverID = GetFreeDriverEntry(&drv);

	uint32 ret = Elf32_LoadImage(Data, DRIVERS_MEMORY_BEGIN + DriverID * 0x100000, PAGE_FLAGS_Pr_Wr_Gl, &proc->Image);
	if(ret != SUCCES)return ret;
	ret = Elf32_ProcessDynamicSection(proc);

	uint32 par[2];
	par[0] = proc->Image.Entry;
	par[1] = DriverID;
	InitStackRing0(th, 0, (uint32)SafeLaunch, SEGMENT_SELECTORS_RING0, 2, par);

	drv->Flags |= DRIVER_ENTRY_VALID;
	drv->proc = proc;
	strcpy(drv->Name, Name);

	proc->flags |= PROCESS_FLAG_DRIVER;
	proc->state = STATE_READY;
	th->flags |= PROCESS_FLAG_DRIVER;

	VFSNode_p node = (VFSNode_p)kmalloc(sizeof(VFSNode_t));
	memset(node, 0, sizeof(VFSNode_t));
	node->Name = drv->Name;
	node->flags = VFS_FLAG_SYSTEM;
	node->type = VFS_TYPE_DRIVER;
	node->owner_data = (uint32)drv;
	node->owner_pid = (uint32)proc;
	VFS_InsertChildren(node, &VFSDriver);

	RunThread(th);
	Data = 0;
	Name = 0;
	return SUCCES;
}

const char VFSDriver_name[] = "drv";
const char VFSDevice_name[]	= "dev";

uint32 DeviceMgr_init(void){
#ifdef _DEBUG_
//check whether there is not space for desired tables count
	if(DRIVERS_TABLE_SIZE < sizeof(driver_t) * DRIVERS_TABLE_COUNT){
		kprintf("%4tRequested Drivers Table Count does not fit in allocated space%t\n");
	}
	if(DEVICES_TABLE_SIZE < sizeof(device_t) * DEVICES_TABLE_COUNT){
		kprintf("%4tRequested Devices Table Count does not fit in allocated space%t\n");
	}
#endif
	memset(DriversTable, 0, DRIVERS_TABLE_SIZE);
	memset(DevicesTable, 0, DEVICES_TABLE_SIZE);

	memset(&VFSDevice, 0, sizeof(VFSNode_t));
	VFSDevice.Name = &VFSDevice_name[0];
	VFSDevice.flags = FILE_FLAG_DIR | VFS_FLAG_SYSTEM;
	VFSDevice.date = SFS_BuildDateTime;
	VFSDevice.type = VFS_TYPE_FOLDER;
	VFS_InsertChildren(&VFSDevice, &VFSRoot);

	memset(&VFSDriver, 0, sizeof(VFSNode_t));
	VFSDriver.Name = &VFSDriver_name[0];
	VFSDriver.flags = FILE_FLAG_DIR | VFS_FLAG_SYSTEM;
	VFSDriver.date = SFS_BuildDateTime;
	VFSDriver.type = VFS_TYPE_FOLDER;
	VFS_InsertChildren(&VFSDriver, &VFSRoot);

	return SUCCES;
}

static sint32 GetFreeDriverEntry(driver_p *drv){
	AcquireLock(&DrvTables);
	sint32 i = 0;
	for (; i < DRIVERS_TABLE_COUNT; ++i){
		if(DriversTable[i].Flags & DRIVER_ENTRY_VALID) continue;
		if(drv) *drv = DriversTable + i;
		memset(DriversTable + i, 0, sizeof(driver_t));
		ReleaseLock(&DrvTables);
		return i;
	}
	ReleaseLock(&DrvTables);
	return -1;
}

static sint32 GetFreeDeviceEntry(device_p *dev){
	AcquireLock(&DevTables);
	sint32 i = 0;
	for (; i < DEVICES_TABLE_COUNT; ++i){
		if(DevicesTable[i].Flags & DEVICE_ENTRY_VALID) continue;
		if(dev) *dev = DevicesTable + i;
		memset(DevicesTable + i, 0, sizeof(device_t));
		ReleaseLock(&DevTables);
		return i;
	}
	ReleaseLock(&DevTables);
	return -1;
}

