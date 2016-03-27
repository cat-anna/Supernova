#ifndef _INCLUDE_SUPERNOVA_SYSCALLS_H
#define _INCLUDE_SUPERNOVA_SYSCALLS_H

enum{

//time
	SYSCALL_WAIT_TIME			= 0x10,

	/*
//system
	SYSCALL_EXIT				= 0,
	SYSCALL_GETPID,
	SYSCALL_PUSHDATA,
	SYSCALL_GETCHAR,
	SYSCALL_GETCURRENTDIR,
	SYSCALL_SETCURRENTDIR,
	SYSCALL_EXECUTEOBJECT,
	SYSCALL_SUSPENDPROCESS,
	SYSCALL_FREE_HANDLE,
	SYSCALL_GET_SYSTEM_INFORMATION,//9
//time
	SYSCALL_GET_TICK_COUNT
	SYSCALL_WAIT_FOR_HANDLE,

	SYSCALL_KILL_TIME_EVENT,
	SYSCALL_SET_TIME_EVENT,
	SYSCALL_GET_SYSTEM_TIME,
//devmgr
	SYSCALL_GETDEVICECOUNT		= 0x20,
	SYSCALL_GETDEVICEINFO,
	SYSCALL_OPEN_DEVICE,
	SYSCALL_REGISTER_REQUEST,
	SYSCALL_SEND_REQUEST,
//	SYSCALL_DEVICE_REQUEST,
//	SYSCALL_GETDEVICEENTRY,
//VFS
	SYSCALL_OPEN_VFS			= 0x30,
//memmgr
	SYSCALL_MEMGET,
	SYSCALL_MEMFREE,
	SYSCALL_READPROCESSMEMORY,
	SYSCALL_WRITEPROCESSMEMORY,
	SYSCALL_INITPROCHEAP,
//VFS
	SYSCALL_OPENVOLUME			= 0x40,
	SYSCALL_REGISTERVOLUME,
	SYSCALL_FINDFIRST,
	SYSCALL_FINDNEXT,
	SYSCALL_FINDCLOSE,
//libmgr
	SYSCALL_LOADLIBARY			= 0x50,
	SYSCALL_GETFUNCTIONADRESS,
	SYSCALL_PROCESSIMPORTLIST,
//IPC
	SYSCALL_ALLOC_MSG_HANDLER	= 0x60,
	SYSCALL_SEND_MSG_RESULT,
//konsola
	SYSCALL_PUTS				= 0x70,
	SYSCALL_PUTCH,
	SYSCALL_CLEAR,
	SYSCALL_TEXTCOLOR,
	SYSCALL_BACKCOLOR,
	SYSCALL_GOTO,
	SYSCALL_CONSOLE_PUSH,
	SYSCALL_CONSOLE_POP,
//debug
	SYSCALL_DEBUG_HEAP			= 0xF0000000,*/
};

/*
#define MakeParamTable_4p(A, B, C, D) uint32 ParamTable[4] = {(uint32)(A), (uint32)(B), (uint32)(C), (uint32)(D)};
#define ParamTab (uint32)ParamTable

//system
SYSCALL uint32 GetPid(){
	return DoInt0x80(SYSCALL_GETPID, 0, 0, 0);
}	
SYSCALL uint32 SystemPushData(uint32 which, uint32 data){
	return DoInt0x80(SYSCALL_PUSHDATA, which, data, 0);
}	
SYSCALL uint32 GetChar(){
	return DoInt0x80(SYSCALL_GETCHAR, 0, 0, 0);
}	
SYSCALL uint32 GetCurrentDirectory(char* cd){
	return DoInt0x80(SYSCALL_GETCURRENTDIR,(uint32)cd,0,0);
}	
SYSCALL uint32 ExecuteObject(char* pstr, void* obj, uint32 obj_size, uint32* obj_pid){
	MakeParamTable_4p(pstr, obj, obj_size, obj_pid);
	return DoInt0x80(SYSCALL_EXECUTEOBJECT, ParamTab, 0, 0);
}	
SYSCALL void SuspendProcess(uint32 pid, bool resume){
	DoInt0x80(SYSCALL_SUSPENDPROCESS, pid, resume, 0);
}	
SYSCALL void CloseHandle(HANDLE h){
	DoInt0x80(SYSCALL_FREE_HANDLE, (uint32)h, 0, 0);
}
//--------------------------------------------------------------------------------------------------
//time
SYSCALL uint32 GetTickCount(){
	return DoInt0x80(SYSCALL_GET_TICK_COUNT,0,0,0);
}
SYSCALL uint32 WaitForObject(HANDLE obj){
	return DoInt0x80(SYSCALL_WAIT_FOR_HANDLE, (uint32)obj, 0, 0);
}
SYSCALL void Sleep(uint32 ms){
	DoInt0x80(SYSCALL_WAIT_TIME, ms, 0, 0);
}
SYSCALL void TimeSetEvent(HANDLE h, uint32 Event, uint32 ms){
	DoInt0x80(SYSCALL_SET_TIME_EVENT,h,Event,ms);
}
SYSCALL void TimeKillEvent(HANDLE h, uint32 Event){
	DoInt0x80(SYSCALL_KILL_TIME_EVENT,h,Event,0);
}
SYSCALL void GetSystemTime(SystemTime_p st){
	DoInt0x80(SYSCALL_GET_SYSTEM_TIME,(uint32)st,0,0);
}
//devmgr
SYSCALL HANDLE OpenDevice(uint32 DevID){
	return DoInt0x80(SYSCALL_OPEN_DEVICE,DevID,0,0);
}
SYSCALL HANDLE RegisterRequest(HANDLE h_dev){
	return (HANDLE)DoInt0x80(SYSCALL_REGISTER_REQUEST,(uint32)h_dev,0,0);
}
SYSCALL uint32 SendRequest(HANDLE h_req, device_request_p req){
	return DoInt0x80(SYSCALL_SEND_REQUEST,(uint32)h_req,(uint32)req,0);
}
//memmgr
SYSCALL uint32 ReadProcessMemory(uint32 pid, uint32 src, uint32 dst, uint32 count){
	MakeParamTable_4p(pid, src, dst, count);
	return DoInt0x80(SYSCALL_READPROCESSMEMORY,ParamTab,0,0);
};
SYSCALL uint32 WriteProcessMemory(uint32 pid, uint32 src, uint32 dst, uint32 count){
	MakeParamTable_4p(pid, src, dst, count);
	return DoInt0x80(SYSCALL_WRITEPROCESSMEMORY,ParamTab,0,0);
};
//VFS
static inline uint32 OpenVolume(char* vName, VolumeReg_p Adata){
	return DoInt0x80(SYSCALL_OPENVOLUME,(uint32)vName,(uint32)Adata,0);
};
//libmgr
SYSCALL HANDLE LoadLibrary(char* lName){
	return DoInt0x80(SYSCALL_LOADLIBARY,(uint32)lName,0,0);
};		
SYSCALL void* GetFunctionAddress(char* fName, HANDLE libH){
	return (void*)DoInt0x80(SYSCALL_GETFUNCTIONADRESS,(uint32)fName,libH,0);
};		
SYSCALL uint32 ProcessImportList(HANDLE libH, FunctionEntry_p fl){
	return DoInt0x80(SYSCALL_PROCESSIMPORTLIST,(uint32)fl,libH,0);
};
//IPC
SYSCALL HANDLE AllocateMessageHandler(msg_handler_p mh){
	return DoInt0x80(SYSCALL_ALLOC_MSG_HANDLER,(uint32)mh,0,0);
}
SYSCALL void SendMessageResult(msg_header_p msg, uint32 result){
	DoInt0x80(SYSCALL_SEND_MSG_RESULT, (uint32)msg, result, 0);
}
//konsola
static inline void puts(const char* s){
	DoInt0x80(SYSCALL_PUTS,(uint32)s,0,0);
};
static inline void putch(uint8 s){
	DoInt0x80(SYSCALL_PUTCH,(uint32)s,0,0);
};
static inline void Console_Clear(){
	DoInt0x80(SYSCALL_CLEAR,0,0,0);
};
static inline void Console_SetTextColor(uint8 c){
	DoInt0x80(SYSCALL_TEXTCOLOR,c,0,0);
};
static inline void Console_SetBackColor(uint8 c){
	DoInt0x80(SYSCALL_BACKCOLOR,c,0,0);
};
static inline void Console_Goto(uint32 x, uint32 y){
	DoInt0x80(SYSCALL_GOTO,x,y,0);
};
static inline void Console_Push(){
	DoInt0x80(SYSCALL_CONSOLE_PUSH,0,0,0);
};
static inline void Console_Pop(){
	DoInt0x80(SYSCALL_CONSOLE_POP,0,0,0);
};

#undef MakeParamTable_4p
#undef ParamTab*/

#endif
