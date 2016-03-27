#include "serial.h"

char com_NAME[] = "comX";

HANDLE h_Msg;

uint32 DriverFunction(device_request_p req){
//	printf("FDD: DriverFunction %x\n",req->request);
	switch (req->request){

	}
	return 0;
}

uint32 MessageHandler(uint32 message, uint32 hparam, uint32 lparam){
//	printf("MessageHandler %d %d %d\n", message, hparam, lparam);
	message = hparam = lparam;
	return 0;
}
/*
static inline void outb(unsigned short port, unsigned char val){
   asm volatile("outb %0,%1"::"a"(val), "Nd" (port));
}

static inline unsigned char inb(unsigned short port){
   unsigned char ret;
   asm volatile ("inb %1,%0":"=a"(ret):"Nd"(port));
   return ret;
}*/

int main(){
	RegisterDriver(&DriverFunction, "serial");
	h_Msg = AllocateMessageHandler(&MessageHandler);
//	printf("serial\n");
	msg_header_p msg;
	while(1){
		if(GetMessage(&msg)){
			DispatchMessage(msg);
		}
		else
			Sleep(0);
	}
}
