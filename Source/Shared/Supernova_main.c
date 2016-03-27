#include <Supernova.h>
#include <Supernova/syscalls.h>
#include <Supernova/di.h>

//#include <string.h>
#include <utils/string.c>

static inline uint32 DoInt0x80(uint32 eax, uint32 ebx, uint32 ecx, uint32 edx){
	uint32 ret;
	asm volatile("push %%ebx;"
				 "mov %%esi, %%ebx;"
				 "int $0x80;"
				 "pop %%ebx;"
					:"=a"(ret)
					: "a"(eax),"S"(ebx),"c"(ecx),"d"(edx)// */
				);
	return ret;
}

void Sleep(uint32 ms){
	DoInt0x80(SYSCALL_WAIT_TIME, ms, 0, 0);
}

void putf(const char* c){
	DoInt0x80(5000, (uint32)c, 0, 0);
}

/* reverse:  reverse string s in place */
void reverse(char s[])
{
	int i, j;
	char c;

	for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/* itoa:  convert n to characters in s */
 void itoa(int n, char s[], int base)
 {
     int i, sign;

     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = "0123456789abcdefghij"[n % base];
     } while ((n /= base) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
 }

/*
#include "pawlos_files.h"
#include "pawlos_task.h"

#include "pawlos_msg.h"
#include "Supernova/di.h"
#include "Supernova/syscalls.h"

#define NO_MALLOC_HEAP_INIT
#include "libc/malloc.c"
*/
/*
void PostQuitMessage(uint32 retCode){
	IPC_Content_info_p ipc = &((ProcSysData_p)0xB0003000)->ipc;
	message_t msg;
	FillMessage(&msg, MSG_EXIT, GetPid(), retCode, 0, 0);
	if(ipc->count > 0)ipc->last += sizeof(message_t);
	memcpy((uint32*)(ipc->last), &msg, sizeof(message_t));
	ipc->count++;
	retCode = 0;
}*/
/*
bool ExecuteFile(char* name, char* paramstr, bool waitforend){
	if(!name)return false;
	char* pstr = (char*)malloc(strlen(name) + 1 + strlen(paramstr) + 1);
	strcpy(pstr, name);
	if(paramstr){
		strcat(pstr, " ");
		strcat(pstr, paramstr);
	}
	
	void* fdata = 0;
	HANDLE H = CreateFile(name, FILE_ACCES_READ, FILE_OPEN_EXISTING);
	uint32 size = 0;
	if(H != INVALID_HANDLE_VALUE){
		size = GetFileSize(H);
		fdata = malloc(size + 1);
		ZeroMemory(fdata, size + 1);
		if(ReadFile(H, fdata, size) != size){
			free(fdata);
			fdata = 0;
		}	
	}
	CloseHandle(H);
	
	uint32 npid = 0;
	uint32 ret = ExecuteObject(pstr, fdata, size, &npid);
	bool succes = (fdata)?(ret == 0):(false);	
	free(fdata);
	free(pstr);
	if(succes && waitforend){
		WaitForEventEx(SLEEP_EVENT_PROCESS, npid);
		return true;
	}
	return false;
}*/
/*
bool FileExists(char* path){
	bool ret = false;
	SearchRec_t sr;
	HANDLE H = FindFirst(path, &sr);
	if(H != INVALID_HANDLE_VALUE)
		ret = !(sr.flags && FILE_FLAG_DIR);
	FindClose(H);	
	return ret;
}
*//*
bool DirectoryExists(char* path){
	bool ret = false;
	SearchRec_t sr;
	HANDLE H = FindFirst(path, &sr);
	if(H != INVALID_HANDLE_VALUE)
		ret = (sr.flags && FILE_FLAG_DIR);
	FindClose(H);	
	return ret;
}*/
/*
uint32 GetMessage(msg_header_p *msg){
	if(!msg)return 0;
	*msg = 0;
	IPC_Content_info_p ipc = &ProcSysData->ipc;
	if(ipc->count == 0) return 0;
	*msg = (msg_header_p)ipc->first;
//	printf("GetMessage %x\n", *msg);
	ipc->count--;
	if(ipc->count > 0)ipc->first += ((msg_header_p)*msg)->size;
	if(ipc->count == 0 && ipc->first == ipc->last)
	ipc->first = ipc->last = 0xB0000000;
	return 1;
}*/
/*
uint32 DispatchMessage(msg_header_p header){
	if(!header)return 0;//&& MakeError();
	uint32 result = 0;
	switch(header->type){
	case MSG_TYPE_MESSAGE:{
		message_p msg = (message_p)header;
//		printf("DispatchMessage %x\n", header->handle);
		result = ((msg_handler_p)header->handle)(header->message, msg->hparam, msg->lparam);
		break;
	}
	case MSG_TYPE_DRV_REQ:{
		message_request_p req = (message_request_p)header;
		result = ((DRV_Request)header->handle)(&req->request);
		break;
	}
	}
//	printf("Dispatch %x %d\n", header->handle, header->size);
	SendMessageResult(header, result);
	return result;
}*/
/*
uint32 DeviceRequest(HANDLE h_dev, device_request_p req, uint32 *result){
	if(h_dev == INVALID_HANDLE_VALUE || !req)return 1;
	HANDLE h_req = RegisterRequest(h_dev);
	if(h_req == INVALID_HANDLE_VALUE)return 2;
	if(SendRequest(h_req, req) != 0)return 3;
	uint32 r = WaitForObject(h_req);
	CloseHandle(h_req);
	if(result)*result = r;
	return 0;
}*/
/*
uint32 gets(char* s){
	if(!s)return 0;
	uint32 ptr = 0;
	while(1){
		Sleep(0);
		uint32 sc = GetChar();		
		if(sc == 0)continue;
		s[ptr] = sc;
							
		if(sc == 10){//enter
			s[ptr] = 0;	
			putch(sc);
			return ptr;
		} else
		if(sc == 8){//backspace
			s[ptr] = 0;
			if(ptr != 0)s[--ptr] = 0;			
			else continue;
		} else
			s[++ptr] = 0;	
		if(!(sc & 0x80))putch(sc);
	}
	return 0;
}*/
/*
#include "Supernova_handles.c"
#include "Supernova_ini.c"
#include "Supernova_sys.c"
*/

int LibMain(int pid, int tid, int reason){
	pid = reason = tid = 0;
	return 0;
}
