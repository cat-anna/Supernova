#include <string.h>
#include "FDD_driver.h"
#include "fdd.h"
#include <Supernova/DrvCommands.h>

char FD0_NAME[] = "fd0";
char FD1_NAME[] = "fd1";

uint32 DriverID;
volatile unsigned Drv_state;
FDD_t FDD_pri;
FDD_t FDD_sec;
HANDLE DMA_handle;

uint32 DMASetTransfer(uint32 mode, uint32 count){
	return DirectCommand(DMA_handle, DMA_COMMAND_SET_TRANSFER, mode, count);
}

static inline FDD_p PickUpDevice(uint32 DevID){
	if(DevID == FDD_pri.DevInfo.DevID)return &FDD_pri;
	if(DevID == FDD_sec.DevInfo.DevID)return &FDD_sec;
	return 0;
}

/*
static inline void DRV_ProcessJob(FDD_queue_item_p job){
	FDD_p Dev = PickUpDevice(job->DevID);
	switch (job->job){
		case FDD_JOB_WRITE:{
			uint32 ret = 0;
			uint32 TempBuf = (uint32)DMABuffer;
			uint32 Count = job->Count;
			uint32 LBA = job->Source;
			WriteDMABuffer(DMABufChannel, job->Count << 9, job->Buffer, job->src_pid);
			while(Count > 0){		
				CHS_t chs;
				CalcCHS(LBA, &chs);
				//iloœæ sektorów do koñca danego cylindra
				uint8 maxsec = 18 - chs.s + 1;

				uint8 SecToRead = (maxsec < Count)?(maxsec):(Count);
				ret = FDD_ProcessSectors(Dev, (void*)TempBuf, &chs, SecToRead, FDD_WRITE);
				if(ret != 0){
				//	print("ERR ret: %d\n", ret);
					break;
				}
					
				TempBuf += SecToRead * 512;
				LBA += SecToRead;
				Count -= SecToRead;
			}
			if(ret == 0){
				job->result = job->Count << 9;				
			} else {
				job->result = 0;	
			}
			job->status = FDD_JOBSTATUS_DONE;
			SuspendProcess(job->src_pid, false);
		}break;
	}
}*/
/*
uint32 MessageHandler(uint32 message, uint32 hparam, uint32 lparam){
//	printf("MessageHandler %d %d %d\n", message, hparam, lparam);
	switch(message){
	case MSG_TIMER:
		FDD_TurnOffMotor((FDD_p)hparam);
		break;
	}
	return 0;
}
*/

volatile sint32 IRQ6Counter;
uint32 FDD_IRQ6Handler(uint32 irq){
	irq = 0;
	IRQ6Counter++;
	return SUCCES;
}

void FDD_WaitForIRQ(){
	while(IRQ6Counter == 0)Sleep(10);
	if(IRQ6Counter > 0)IRQ6Counter--;
	if(IRQ6Counter < 0)IRQ6Counter = 0;
}

uint32 FDD_Open(uint32 DevID, uint32 *Handle, const char *SubPath, uint32 Mode){
	if(DMA_handle == 0){
		DMA_handle = DirectOpen(DriverID, "/dev/dma/2", ACCES_FULL);
		if(!DMA_handle)return DMA_EC_DMA_NOT_AVAILABLE;
	}
	FDD_p dev = PickUpDevice(DevID);
	if(!dev) return ERRORCODE_FATAL;
	dev->handle = (uint32)Handle;
	*Handle = dev->handle;
//ignore these parameter
	SubPath = 0;
	Mode = 0;
	return SUCCES;
}

uint32 FDD_Close(uint32 DevID, uint32 handle){
	FDD_p dev = PickUpDevice(DevID);
	if(!dev) return ERRORCODE_FATAL;
	dev->handle = 0;
	handle = 0;
	return SUCCES;
}

uint32 FDD_Read(uint32 DevID, uint32 Handle, uint32 LBA, void *Buffer, uint32 Count, uint32* Read){
	if(!Buffer || !Read)return ERRORCODE_WRONG_INPUT;
	FDD_p dev = PickUpDevice(DevID);
	if(!dev) return ERRORCODE_FATAL;
	if(dev->handle != Handle)return ERRORCODE_FATAL;
	AcquireLock(&dev->lock);

	dev->DevInfo.State = DEV_STATE_BUSY;
	uint32 ret = 0;
	uint32 Remain = Count >> 9;
	LBA >>= 9;
	char *Buf = (char*)Buffer;
//	kprintf("FDD read  LBA=%d  Count=%d  Dst=%x\n", LBA, Count, Buffer);
	while(Remain){
		CHS_t chs;
		CHS_calc(&chs, LBA);
		uint32 maxsec = 18 - chs.s + 1;//iloœæ sektorów do koñca danego cylindra
		uint32 SecToRead = (maxsec < Remain)?(maxsec):(Remain);
		ret =  FDD_ProcessSectors(dev, &chs, SecToRead, FDD_READ);
//		kprintf("FDD (%d) SecToRead: %d %d\n", ret, SecToRead, Remain);
		if(ret != SUCCES) break;
		uint32 read;
		DirectSeek(DMA_handle, 0, SEEK_BEGINING);
		ret = DirectRead(DMA_handle, Buffer, SecToRead << 9, &read);
		if(ret != SUCCES && read != SecToRead << 9)break;
		LBA += SecToRead;
		Remain -= SecToRead;
		Buf += SecToRead << 9;
	}
	*Read = Count - (Remain << 9);
	dev->DevInfo.State = DEV_STATE_READY;
//	kprintf("FDD done\n");
	ReleaseLock(&dev->lock);
	if(!Remain)
		return SUCCES;
	else
		return FDD_EC_PROCESS_SECTORS_FAILED;
}

uint32 FDD_Write(uint32 DevID, uint32 Handle, void *Buffer, uint32 Src, uint32 Count, uint32* Written){
	kprintf("fdd: %s\n", __FUNCTION__);
	return SUCCES;
}

uint32 FDD_Seek(uint32 DevID, uint32 Handle, uint32 offset, uint32 method, uint32 *NewPos){
	FDD_p dev = PickUpDevice(DevID);
	if(!dev) return ERRORCODE_FATAL;
	if(dev->handle != Handle) return ERRORCODE_FATAL;

	//kprintf("SEEK %d %d %x\n", method, offset, NewPos);

	if(method != SEEK_BEGINING) return DEV_EC_SEEK;
	if(NewPos)*NewPos = offset;

	return SUCCES;
}

unsigned FDD_Initialize(){
	memset(&FDD_pri, 0, sizeof(FDD_t));
	memset(&FDD_sec, 0, sizeof(FDD_t));
	IRQ6Counter = 0;

	if(RegisterIRQ(6, &FDD_IRQ6Handler) != SUCCES){
		return ERRORCODE_FATAL;
	}

	uint8 f0, f1;
	outb(0x70, 0x10);
	f1 = inb(0x71);
	f0 = f1 >> 4; // get the high nibble
	f1 &= 0x0F; // get the low nibble by ANDing out the high nibble

	if(f0){
		AcquireLock(&FDD_pri.lock);
		FDD_pri.Type = f0;
		FDD_pri.Base = FLOPPY_PRIMARY_BASE;
		FDD_pri.FDDNo = 0;
		FDD_pri.State = FDD_NOT_INITIALIZED;
		FDD_pri.DevInfo.Name = FD0_NAME;
		FDD_pri.DevInfo.State = DEV_STATE_READY;
		FDD_pri.DevInfo.DevType = DEVICE_FDD_DRIVE;

		FDD_pri.DevInfo.Functions.Open = &FDD_Open;
		FDD_pri.DevInfo.Functions.Close = &FDD_Close;
		FDD_pri.DevInfo.Functions.Read = &FDD_Read;
		FDD_pri.DevInfo.Functions.Write = &FDD_Write;
		FDD_pri.DevInfo.Functions.Seek = &FDD_Seek;

		RegisterDevice(DriverID, &FDD_pri.DevInfo);
	} else {
		FDD_pri.State = FDD_NOT_EXISTS;
	}

	FDD_sec.State |= FDD_NOT_EXISTS;

	if(FDD_pri.State == FDD_NOT_INITIALIZED){
		InitFDD(&FDD_pri);
		ReleaseLock(&FDD_pri.lock);
	}

	return SUCCES;
}

unsigned FDD_Scan(){
/*	if(DevCount != 0){
		h_DMA = DMAOpenChannel(2);
	} else {
	//	CloseHandle(h_IRQ6);
	//	return 1;
	}*/
//		printf("FDD %x %x %x\n", h_IRQ6, h_DMABuf, h_Msg);
	return SUCCES;
}

uint32 DrvEntry(uint32 DrvID){
	DriverID = DrvID;
	DMA_handle = INVALID_HANDLE_VALUE;

	DriverFunctions_t DrvFun;
	memset(&DrvFun, 0, sizeof(DriverFunctions_t));
	DrvFun.Initialize = &FDD_Initialize;
	DrvFun.Scan = &FDD_Scan;
	RegisterDriver(DrvID, &DrvFun, "FDD", DRIVER_INFO_DEVICE);

	Drv_state = 1;
	while(Drv_state > 0){
		Sleep(1000);
		//kprintf("fdd: %s\n", __FUNCTION__);
	}
	return 0;
}

/*
int main(){
	while(1){};
	RegisterDriver(&DriverFunction, "fdc");
	h_Msg = AllocateMessageHandler(&MessageHandler);
	
	msg_header_p msg;
	while(1){
		if(GetMessage(&msg)){
		//	if(msg->message == MSG_EXIT)return ((message_p)msg)->hparam;
		//	printf("dispatch\n");
			DispatchMessage(msg);
		}
		else
			Sleep(0);
	}*/
	/*message_t msg;
	while(1){
		Sleep(0);
		if(GetMessage(&msg) == 0)
			if(msg.type == MSG_TIMER){
				FDD_p Dev = PickUpDevice(msg.hparam);
				if(Dev)FDD_TurnOffMotor(Dev);
			}
		uint32 i = 0;
		FDD_queue_item_p q = FDD_jobQeueue;
		for(;i < FDD_JOB_QUEUE_SIZE; i++)
			if(q->status == FDD_JOBSTATUS_READY){
				DRV_ProcessJob(q);
				break;
			} else q++;
	}*/
	/*
//	floppy_disk = (floppy_parameters*)malloc(sizeof(floppy_parameters));
//	memcpy((uint8*)floppy_disk, (uint8*)FDD_PARAMETER_ADDRESS, sizeof(floppy_parameters));
}*/
