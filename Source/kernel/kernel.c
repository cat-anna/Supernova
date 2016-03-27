#define _COMPILING_KERNEL_
#include "kernel.h"
#include <headers/Loader.h>

const SystemInfo_t SystemInformation = {
	sizeof(SystemInfo_t),
	{VERSION_MAJOR,	VERSION_MINOR, VERSION_BUILD},
};

uint32 AcquireLock(SPINLOCK *sp){
	asm volatile(
			//"movl $1, %%eax;"
			"lock btsl $0, (%0);"
			"jnc 2f;"
			"1:;"
			"pause;"
			"testl $1, (%0);"
			"jnz 1b;"
			"lock btsl $0, (%0);"
			"jc 1b;"
			"2:;"
			:
			:"b"(sp)
	//		:"eax"
			);
	return 1;
}

void ReleaseLock(SPINLOCK *sp){
	*sp = 0;
}

void SystemHalt(){
//	cli();
	kprintf("\n%4tSystem halted%t\n");
	while(1){};
}

static void InitCheck(uint32 retcode, const char *fun){
	if(retcode == 0)return;
	kprintf("%4tCritical error code: %d while initializing %s%t\n", retcode, fun);
	SystemHalt();
}

static void* LoadFile(const char* file_name, uint32* file_size){
	if(!file_name)return 0;
	if(file_size)*file_size = 0;

	HANDLE H = HandleOpen(file_name, ACCES_READ | OPEN_EXISTING);
	if(H == INVALID_HANDLE_VALUE) return 0;

	uint32 fsize = HandleGetSize(H);
	if(!fsize)return 0;
	void *buf = kmalloc(fsize + 1);
	uint32 read = 0;
	if(HandleRead(H, buf, fsize, &read) != SUCCES){
		kfree(buf);
		return 0;
	}
	if(read != fsize){
		kfree(buf);
		return 0;
	}
	HandleClose(H);
	if(file_size)*file_size = fsize;
	return buf;
}

uint32 SupernovaMainThread(){
	kprintf("\nSupernova initialized\nWaiting for boot volume");
	volume_p BootVol = 0;
	uint32 timeout = 5;//17;
	while(timeout-- && !BootVol){
		if(GetVolumeByID(DataFromLoader->BootPartitionID, &BootVol) == SUCCES) break;
		int0_Sleep(500);
		putch('.');
	}
	putch('\n');
	CriticalCheck(!BootVol);
	kprintf("\tBoot volume name: %s\n", BootVol->name);

	char PathPtr[50];
	strcpy(PathPtr, BootVol->name);
	char *FileNamePtr = strchr(PathPtr, '\0');
	*FileNamePtr++ = ':';
	*FileNamePtr++ = '/';

	kprintf("Loading system.cfg...\n");
	strcpy(FileNamePtr, "system.cfg");
	void* syscfg = LoadFile(PathPtr, 0);
	CriticalCheck(!syscfg);

	kprintf("Loading shell.elf...\n");
	strcpy(FileNamePtr, "shell.elf");
	void* shell = LoadFile(PathPtr, 0);
	CriticalCheck(!shell);

	kprintf("done.\nstarting...\n");
	uint32 pid = 0;
	RunUserProcess(shell, &pid, FileNamePtr);

kprintf("done pid: %d\n", pid);
	kfree(syscfg);
	kfree(shell);
/*
	CriticalCheck(syscfg);
	CriticalCheck(!SLIB_iniFastRead(syscfg, "system", "shell", fn_ptr, 40));
	kfree(syscfg);

	kprintf("Loading shell...\n");
	void *shell = LoadFile(temp, 0);
	CriticalCheck(shell);
	uint32 sh_pid;
	CriticalCheck(!elf_exec(temp, shell, current_pid, &sh_pid, 0));
	ProcessList[sh_pid].flags |= PROC_FLAG_SHELL;
	kfree(shell);*/

	while(1);
	return 0;
}

extern unsigned _end;

uint32 MemoryManager_init(void);
uint32 SharedLibs_init(void);
uint32 RTC_init(void);
uint32 DeviceMgr_init(void);
uint32 Multitasking_init(void);
uint32 VFS_init(void);
uint32 Interrupts_init(void);
uint32 CPU_init(void);
uint32 VolMgr_init(void);
uint32 Heap_init(void);
uint32 IPC_init(void);

void Kernel_Init(unsigned con_position){
	Console_Goto(con_position & 0xFF, con_position >> 8);
	kprintf("Welcome to Supernova %d.%d build %d\n", SystemInformation.version.major, SystemInformation.version.minor, SystemInformation.version.build);
#ifdef _DEBUG_
	if((uint32)&_end >= KERNEL_CODE_DEAD_END){
		//just one of plenty unnecessary checks which I had to add
		kprintf("%4tKernel has grown beyond the provided space!%t\n");
	} else {
		uint32 per = 100 * ((uint32)&_end - KERNEL_CODE_BEGIN) / (KERNEL_CODE_DEAD_END - KERNEL_CODE_BEGIN);
		kprintf("Kernel has %d kb (used %d%%) space to grow\n", (KERNEL_CODE_DEAD_END - (uint32)&_end) / 1024, per);
	}
#endif
	puts("Initializing...\n");
	InitCheck(CPU_init(), "CPU");
	InitCheck(MemoryManager_init(), "Memory manager");
	InitCheck(Heap_init(), "Kernel heap");
	InitCheck(Interrupts_init(), "Interrupts");
	InitCheck(VFS_init(), "VFS");
	InitCheck(RTC_init(), "RTC");
	InitCheck(IPC_init(), "IPC");
	InitCheck(SharedLibs_init(), "SharedLibs");
	InitCheck(Multitasking_init(), "Multitasking");
	InitCheck(DeviceMgr_init(), "DeviceMgr");
	InitCheck(VolMgr_init(), "VolMgr");

	uint32 i;
	for(i = 0; i < DataFromLoader->LoadedFilesCount; ++i){
		char *ext = strrchr(DataFromLoader->LoadedFiles[i].Name, '.');
		if(!ext)continue;
		if(strcasecmp(ext, ".lib"))continue;
		uint32 err = InstallLibrary(DataFromLoader->LoadedFiles[i].FilePtr, DataFromLoader->LoadedFiles[i].Name);
		if(!err)continue;
		kprintf("%4t\tAn error (code: 0x%x) has occur while loading file %s%t\n", err, DataFromLoader->LoadedFiles[i].Name);
	}
}
