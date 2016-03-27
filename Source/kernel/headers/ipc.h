#ifndef _IPC_H_
#define _IPC_H_

typedef struct IPCContentInfo_s {
	uint32 size;		//of this structure always 0x20
	uint32 buffer_size;	//size of IPC Buffer, right now always 0x1000
	uint32 flags;		//space for possible flags
	uint32 count;		//count of messages queued
	uint32 first;		//offset to the first message
	uint32 last;		//offset to the last message
	uint32 unused1;		//space for the future
	uint32 unused2;		//space for the future
} IPCContentInfo_t, *IPCContentInfo_p;

typedef struct IPCMsgQueue_s {
	message_t msg;
	IPCMsgQueue_p next;
} IPCMsgQueue_t;

uint32 AllocIPCBuffer(process_p proc);//allocates memory IPC and set it up
uint32 InitIPCBuffer(IPCContentInfo_p ipcbuf);//sets content of IPCContentInfo_t

uint32 IPC_QueueMessage(message_p msg, process_p proc);//Add message to queue
uint32 IPC_UpdateBuffer(process_p proc);//Moves all messages from queue to buffer
uint32 IPC_BufferMessage(message_p msg, void* buf);//Adds message to buffer
uint32 IPC_GetCount(void *IPC);//get count of messages in buffer


uint32 IPC_GetMessage(message_p msg, void* buf);
	//buf has to be in current PD


uint32 KernelMessage(uint32 msg, uint32 hpar, uint32 lpar, void *Buffer);

uint32 PerformMessage(uint32 msg, uint32 hparam, uint32 lparam);

#endif
