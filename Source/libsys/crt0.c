/*
#include <Supernova.h>
*/
void(*printf)(const char* fmt, ...);

void(*memcpy)(void *dest, const void *src, uint32 len);
void(*memset)(void *dest, uint8 val, uint32 len);
char*(*strrchr)(const char *s, int c);
char*(*strchr)(const char *s, int c);
int(*strcmp)(const char *s1, const char *s2);
char*(*strncpy)(char *dest, const char *src, uint32 n);
uint32(*strlen)(const char *s);
char*(*strcpy)(char *s1, const char *s2);
char*(*strcat)(char *s1, const char *s2);
int(*strcmp2delim)(const char *s1, const char *s2, const char delim);
int(*strcmppat)(const char *s, const char *pat);

void*(*malloc)(uint32 size);
void(*free)(void *p);

//uint32 main();
/*
void __sysinit(){
	HANDLE klib = LoadLibrary("Supernova.lib");
	uint32 err = ProcessImportList(klib, Kernel_ImportList);
	if(err != 0){
		DoInt0x80(SYSCALL_EXIT, err, err, 0);
		while(1){};
	}
}*/
/*
__attribute__((noreturn))
void _start(){
	asm volatile("call __sysinit\n\t"
				 "call main\n\t"
				 "movl %eax, %ebx\n\t"
				 "xor %eax, %eax\n\t"
				 "xor %ecx, %ecx\n\t"
				 "int $0x80");
	while(1){};	
}*/
