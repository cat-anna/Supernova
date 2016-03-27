#include "kernel.h"

static IPCMsgQueue_p IPCEmpty;

#define QUEUE_SIZE ((0x1000 - sizeof(IPCContentInfo_t)) / sizeof(message_t))

uint32 InternalGetMessage(message_p msg){
	if(!msg)return 0;
	process_p proc = CurrentThread->OwnerProcess;
	return IPC_GetMessage(msg, proc->IPCBuf);
}

uint32 KernelMessage(uint32 msg, uint32 hpar, uint32 lpar, void *Buffer)
{
	message_t m;
	m.message = msg;
	m.lparam = lpar;
	m.hparam = hpar;

/*	void* handle;		//pointer to the message handler function
	uint32 sender;
	uint32 time;
*/

	return IPC_BufferMessage(&m, Buffer);
}

uint32 IPC_GetMessage(message_p msg, void* buf){
	if(!buf)return ERRORCODE_FATAL;
	IPCContentInfo_p ipcbuf = (IPCContentInfo_p)buf;
	if(ipcbuf->count == 0)return ERRORCODE_FATAL;

	message_p firstmsg = (message_p)((uint32)ipcbuf + ipcbuf->first);

	memcpy(msg, firstmsg, sizeof(message_t));
	ipcbuf->first += sizeof(message_t);
	--ipcbuf->count;

	if(ipcbuf->first == ipcbuf->last){
		ipcbuf->first = sizeof(IPCContentInfo_t);
		ipcbuf->last = sizeof(IPCContentInfo_t);
	}

	return SUCCES;
}



uint32 IPC_BufferMessage(message_p msg, void* buf){
	if(!buf)return ERRORCODE_FATAL;
	IPCContentInfo_p ipcbuf = (IPCContentInfo_p)buf;
	if(ipcbuf->count >= QUEUE_SIZE)
		return ERRORCODE_IPC_IS_FULL;
	message_p msgbuf = (message_p)((uint32)ipcbuf + ipcbuf->last);
	memcpy(msgbuf, msg, sizeof(message_t));
	ipcbuf->last += sizeof(message_t);
	++ipcbuf->count;
	return SUCCES;
}

uint32 IPC_UpdateBuffer(process_p proc){
	if(!proc->IPCBuf) AllocIPCBuffer(proc);
	IPCMsgQueue_p buf = proc->IPCQueue;
	proc->IPCQueue = 0;
	while(buf){
		IPC_BufferMessage(&buf->msg, proc->IPCBuf);
		IPCMsgQueue_p p = buf;
		buf = buf->next;
		p->next = IPCEmpty;
		IPCEmpty = p;
	}
	return SUCCES;
}

uint32 IPC_QueueMessage(message_p msg, process_p proc){
	IPCMsgQueue_p buf;
	if(IPCEmpty){
		buf = IPCEmpty;
		IPCEmpty = IPCEmpty->next;
	} else {
		buf = kmalloc(sizeof(IPCMsgQueue_t));
	}
	buf->next = 0;
	memcpy(&buf->msg, msg, sizeof(message_t));

	if(!proc->IPCQueue)
		proc->IPCQueue = buf;
	else {
		IPCMsgQueue_t *t = proc->IPCQueue;
		while(t->next)t = t->next;
		t->next = buf;
	}
	return SUCCES;
}

uint32 IPC_GetCount(void *IPC){
	IPCContentInfo_p ipcbuf = (IPCContentInfo_p)IPC;
	return ipcbuf->count;
}

uint32 InitIPCBuffer(IPCContentInfo_p ipcbuf){
	memset(ipcbuf, 0, sizeof(IPCContentInfo_t));
	ipcbuf->size = sizeof(IPCContentInfo_t);
	ipcbuf->buffer_size = 0x1000;
	ipcbuf->first = sizeof(IPCContentInfo_t);
	ipcbuf->last = sizeof(IPCContentInfo_t);
	return SUCCES;
}

uint32 AllocIPCBuffer(process_p proc){
	IPCContentInfo_t ipc;
	InitIPCBuffer(&ipc);
	proc->IPCBuf = (void*)PROCESS_IPC_BUFFER;
	return AllocAndWriteMemory(proc->cr3, PROCESS_IPC_BUFFER, &ipc, sizeof(IPCContentInfo_t), MEM_FLAGS_Pr_Wr);
}

uint32 IPC_init(void){
	IPCEmpty = 0;
	return SUCCES;
}
