#include "kernel.h"
#include "supernova/syscalls.h"

static inline uint32 DoInt0x80(uint32 eax, uint32 ebx, uint32 ecx, uint32 edx){
	uint32 ret;
	asm volatile("int $0x80;"
					:"=a"(ret)
					: "a"(eax),"b"(ebx),"c"(ecx),"d"(edx)// */
				);
	return ret;
}

void int0_Sleep(uint32 ms){
	DoInt0x80(SYSCALL_WAIT_TIME, ms, 0, 0);
}

//static void do_FreeHandle(HANDLE h_no);

uint32 SysCall_int80(uint32 eax, uint32 ebx, uint32 ecx, uint32 edx){
//	kprintf("syscall %x %x %x %x\n", eax, ebx, ecx, edx);
	switch(eax){
//system
	//	case SYSCALL_EXIT:
//			proc_kill(current_pid, ebx, ecx);
		//	while(1);
		//	CHANGE_PROCESS;
			/* no break */
	//	case SYSCALL_GETPID: return current_pid;
	/*case SYSCALL_PUSHDATA:
			if(current_proc->flags & PROC_FLAG_DRIVER){
				if(ebx == SYSTEMDATA_SCANCODE)LastScanCode = ecx;
			};
			return 0;*/
		/*case SYSCALL_GETCHAR:{
			uint32 ret = LastScanCode;
			LastScanCode = 0;
			return ret;
		}*/
		/*case SYSCALL_GETCURRENTDIR:
			if(!ebx)return 0;
			VFS_node_p node = GetVolumeByPartID(boot_part_id);
			if(!node) return 0;
			char* cd = (char*)ebx;
			strcpy(cd, node->name);
			strcat(cd, ":\\");
			return 0;*/
/*		case SYSCALL_EXECUTEOBJECT:{
			uint32 *PT = (uint32*)ebx;
			if(!PT[0] || !PT[1] || !PT[2])return 1;
			uint32 ret_code = 1;
		//	kprintf("SYSCALL_EXECUTEOBJECT %x %d\n", ebx, PT[2]);
			void* obj = kmalloc(PT[2]);

			memcpy(obj, (void*)PT[1], PT[2]);
			char* pstr = (char*)kmalloc(strlen((char*)PT[0]));
			strcpy(pstr, (char*)PT[0]);
			uint32 npd;

			ret_code = elf_exec(0, obj, current_pid, &npd, pstr);
			if(PT[3])*((uint32*)PT[3]) = npd;
			kfree(pstr);
			kfree(obj);
			return ret_code;
		}*/
/*		case SYSCALL_SUSPENDPROCESS:{
			task_p task = &proctab[ebx];
			if(task->state != PROCESS_STATE_NONE)return 0;
//			kprintf("SYSCALL_SUSPENDPROCESS %d %d\n", ebx, ecx);
			if(ecx == false && task->state == PROC_STATE_SUSPENDED)task->state = PROC_STATE_READY;
			else if(ecx == true && task->state == PROC_STATE_READY)task->state = PROC_STATE_SUSPENDED;
			if(ebx == current_pid)asm volatile("jmp change_proc");
		}return 0;*/
//		case SYSCALL_FREE_HANDLE: do_FreeHandle((HANDLE)ebx); return 0;
		/*case SYSCALL_GET_SYSTEM_INFORMATION:
			if(!ebx)return 0;
			SystemInfo_p si = (SystemInfo_p)ebx;
			memcpy(si, &SystemInformation, sizeof(SystemInfo_t));
			return 1;*/
//time
//		case SYSCALL_GET_TICK_COUNT: return SysTime_mS;
/*		case SYSCALL_WAIT_FOR_HANDLE:{
			handle_p h = CheckProcessHandle((HANDLE)ebx, current_proc);
			if(!h)return 0;
			switch(h->type){
				case HANDLE_TYPE_IRQ:{
					IRQ_Data_p irq = &irq_handlers[h->data.u32];
					if(irq->count){
						irq->count--;
						return 0;
					}
					irq->flags |= IRQ_FLAG_PROCESS_WAITING;
					current_proc->state = PROC_STATE_SLEEP_IRQ;
					break;
				}
				case HANDLE_TYPE_REQUEST:{
					current_proc->state = PROC_STATE_SLEEP_MSG;
					break;
				}
			}
			asm("jmp change_proc");
		}return 0;*/
		case SYSCALL_WAIT_TIME:{
			PutThreadToSleep(CurrentThread, ebx);
			ChangeThread();
			break;
		}
//		case SYSCALL_SET_TIME_EVENT: SetProcessTimer(current_pid, ebx, ecx, edx); return 0;
//		case SYSCALL_KILL_TIME_EVENT: KillProcessTimer(current_pid, ebx, ecx); return 0;
	//	case SYSCALL_GET_SYSTEM_TIME: GetSystemTime((SystemTime_p)ebx); return 0;
//device manager
/*		case SYSCALL_OPEN_DEVICE: return DRV_OpenDevice(GetDeviceByID(ebx), current_pid);
		case SYSCALL_REGISTER_REQUEST: return DRV_RegisterRequest((HANDLE)ebx, current_pid);
		case SYSCALL_SEND_REQUEST: return DRV_SendRequest((HANDLE)ebx, (device_request_p)ecx);*/
//VFS
/*		case SYSCALL_OPEN_VFS:{
			if(!ebx)return INVALID_HANDLE_VALUE;
			VFS_node_p node = 0;
			uint32 h_no;
			handle_p h = GetFreeHandle(current_proc, &h_no);
			if(!h)return INVALID_HANDLE_VALUE;
			h->type = HANDLE_TYPE_VFS_NODE;
			if((node = GetVolumeByName((const char*)ebx))){
				h->flags = HANDLE_FLAGS_OReq;
			}
			else if((node = VFS_GetChildByName(VFS_root, (const char*)ebx))){
				h->flags = HANDLE_FLAG_OPENED;
			} else return INVALID_HANDLE_VALUE;
			h->data.node = node;
			return h_no;
		}*/
//memory manager
		/*case SYSCALL_MEMGET:
			if(ebx > ecx)return 1;
			AllocateHighMemory(ebx, (ecx - ebx) >> 12, PAGE_FLAGS_FOR_USER);
			return 0;*/
	/*	case SYSCALL_MEMFREE:
			if(!ebx)return 0;
			ulong PDi = (ulong)ebx >> 22;
			ulong PTi = (ulong)ebx >> 12 & 0x03FF;
			if(((ulong*)0xFFFFF000)[PDi] == 0)return 0;
			ulong *PT = ((ulong*)0xFFC00000) + (0x400 * PDi) + (PTi * 4);
			if(!*PT)return 0;
			FreePage(*PT);
			*PT = 0;
			return 0;*/
		/*case SYSCALL_READPROCESSMEMORY:{
			uint32 *PT = (uint32*)ebx;
			return CopyProcessMemory(PT[0], PT[1], PT[3], current_pid, PT[2]);
		}*/
		/*case SYSCALL_WRITEPROCESSMEMORY:{
			uint32 *PT = (uint32*)ebx;
	/		return CopyProcessMemory(current_pid, PT[1], PT[3], PT[0], PT[2]);
		}*/
	//	case SYSCALL_INITPROCHEAP: InitProcHeap(current_proc, (void*)ebx, (void*)ecx); return 0;
//libmgr
/*		case SYSCALL_LOADLIBARY:
			if(!ebx)return 0;
			char *libn = strrchr((char*)ebx, '\\');
			if(!libn)libn = (char*)ebx;
			else libn++;
			SharedLibrary_p lib = GetSharedLibrary(libn);
			if(current_pid == 0)return (uint32)lib;
			if(!lib && !(lib = LoadLibFromDisk(libn))) return INVALID_HANDLE_VALUE;
			return (uint32)AttachLibraryToProcess(lib);
		case SYSCALL_GETFUNCTIONADRESS: return (uint32)GetFunctionAddres(ecx, (char*)ebx);
		case SYSCALL_PROCESSIMPORTLIST:{
			if(!ebx)return 10;
			FunctionEntry_p flist = (FunctionEntry_p)ebx;
			while(flist->ptr != 0){
				uint32 addr = (uint32)GetFunctionAddres(ecx, (char*)flist->name);
				if(addr == 0){
					kprintf("Cannot find function %s in library %s!\n", flist->name, ((SharedLibrary_p)ecx)->name);
					return 1;
				}
				*((uint32*)(flist->ptr)) = addr;
				flist++;				
			}
		}return 0;*/
//IPC
	/*	case SYSCALL_ALLOC_MSG_HANDLER:
			if(!ebx)return INVALID_HANDLE_VALUE;
			uint32 h_no;
			handle_p h = GetFreeHandle(current_proc, &h_no);
			if(!h)return INVALID_HANDLE_VALUE;
			h->type = HANDLE_TYPE_MESSAGE_HANDLER;
			h->flags = HANDLE_FLAG_OPENED;
			h->data.u32 = ebx;
			return h_no;
		case SYSCALL_SEND_MSG_RESULT:
			if(!ebx)return 0;
			msg_header_p msg = (msg_header_p)ebx;
			if(msg->sender_handle == INVALID_HANDLE_VALUE)return 0;
			//handle_p h_req = CheckProcessHandle(msg->sender_handle, &ProcessList[msg->sender]);
			process_p p_req = &ProcessList[msg->sender];
			if(p_req->state != PROC_STATE_SLEEP_MSG)return 0;

			p_req->flags |= PROC_FLAG_SLEEP_RESULT;
			p_req->sleep_result = ecx;
			p_req->state = PROC_STATE_READY;
			return 0;*/
//konsola
	/*	case SYSCALL_PUTS: puts((char*)ebx); return 0;
		case SYSCALL_PUTCH: putch(ebx); return 0;
		case SYSCALL_CLEAR: Console_Clear(); return 0;
		case SYSCALL_TEXTCOLOR: Console_SetTextColor(ebx); return 0;
		case SYSCALL_BACKCOLOR: Console_SetBackColor(ebx); return 0;
		case SYSCALL_GOTO: Console_Goto(ebx, ecx); return 0;*/
		//case SYSCALL_CONSOLE_PUSH: Console_Push(); return 0;
	//	case SYSCALL_CONSOLE_POP: Console_Pop(); return 0;
//handles

//system
	/*	case SYSCALL_SEND_MESSAGE:{
			((message_t*)ecx)->flags |= MSG_FLAG_SEND;
			ipc_sendmessage(ebx, (message_t*)ecx);
			return 0;
		}
		case SYSCALL_POST_MESSAGE:{
			((message_t*)ecx)->flags &= ~MSG_FLAG_SEND;
			ipc_sendmessage(ebx, (message_t*)ecx);
			return 0;
		}
//VFS
		case SYSCALL_OPEN_PROCESS:{
			if(!ebx || ebx > highest_task)return INVALID_HANDLE_VALUE;			
			if(proctab[ebx].used != 1)return INVALID_HANDLE_VALUE;
			VFS_node_Ptr node = VFS_GetChildren(VFS_mem);
			if(!node)return INVALID_HANDLE_VALUE;
			node->ownerdata = ebx;			
			HandleStructPtr data = new HandleStruct_t;	
			ZeroMemory(data, sizeof(HandleStruct_t));
			data->ptr = (uint32)node;
			return (uint32)data;
		}*/
//debug
#ifdef _DEBUG_
	/*	case SYSCALL_DEBUG_HEAP:{
			ShowKHeapStructure();
			return 0;
		}*/
#endif
		default: 
			kprintf("wrong int 0x80 (%d):(%x:%x:%x:%x)\n", 0, eax, ebx, ecx, edx);
			break;
	}
	return 0;
}
/*
static void do_FreeHandle(HANDLE h_no){
	handle_p h = CheckProcessHandle(h_no, current_proc);
	if(!h)return;

	switch (h->type){
	case HANDLE_TYPE_IRQ:
		break;
	case HANDLE_TYPE_DMA:
		break;
	case HANDLE_TYPE_LIBRARY:
		break;
	case HANDLE_TYPE_MESSAGE_HANDLER:
		break;
	case HANDLE_TYPE_DEVICE:
		break;
	case HANDLE_TYPE_REQUEST:
		break;
	case HANDLE_TYPE_SHARED_FRAME:
		FreeSharedFrame(h->data.SharedFrame);
		kfree(h->data.SharedFrame);
		break;
	}
	h->flags = 0;
}
*/
