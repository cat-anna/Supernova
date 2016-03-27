#ifndef _KERNEL_H_
#define _KERNEL_H_

#define _DEBUG_
#define _BUILDING_KERNEL_
#define _PRINT_INIT_INFO_
#define _IA32_

enum{
	KERNEL_CODE_BEGIN			= 0xC0010000,
	KERNEL_CODE_DEAD_END		= 0xC0030000,

	SEGMENTS_DESCRIPTORS_POINTER	= KERNEL_CODE_DEAD_END,
	SEGMENTS_DESCRIPTORS_TABLE		= SEGMENTS_DESCRIPTORS_POINTER + 0x10,
	SEGMENTS_DESCRIPTORS_SIZE		= 0x80, //128bytes

	INTERRUPTS_DESCRIPTORS_POINER	= KERNEL_CODE_DEAD_END + 0x80,
	INTERRUPTS_DESCRIPTORS_TABLE	= INTERRUPTS_DESCRIPTORS_POINER + 0x10,
	INTERRUPTS_DESCRIPTORS_SIZE		= 256 * 8 + 0x80, //2176 bytes

	SHAREDLIBS_TABLE_LOCATION	= KERNEL_CODE_DEAD_END + 0x0900,
	SHAREDLIBS_TABLE_SIZE		= 0x00000700,
	SHAREDLIBS_TABLE_COUNT		= 10,

	DRIVERS_TABLE_LOCATION		= KERNEL_CODE_DEAD_END + 0x1000,
	DRIVERS_TABLE_SIZE			= 0x00000800,
	DRIVERS_TABLE_COUNT			= 10,
	DEVICES_TABLE_LOCATION		= KERNEL_CODE_DEAD_END + 0x1800,
	DEVICES_TABLE_SIZE			= 0x00000800,
	DEVICES_TABLE_COUNT			= 20,

	PROCESS_TABLE_LOCATION		= KERNEL_CODE_DEAD_END + 0x2000,
	PROCESS_TABLE_SIZE			= 0x00002000,
	PROCESS_TABLE_COUNT			= 20,
	THREADS_TABLE_LOCATION		= KERNEL_CODE_DEAD_END + 0x4000,
	THREADS_TABLE_SIZE			= 0x00002000,
	THREADS_TABLE_COUNT			= 100,

	DEVMGR_IPC_LOCATION			= KERNEL_CODE_DEAD_END + 0x6000,// - 0xC0000000,
	DEVMGR_IPC_SIZE				= 0x1000,

	LOW_HEAP_LOCATION_VIRTUAL	= KERNEL_CODE_DEAD_END + 0x10000,
	LOW_HEAP_LOCATION_PHYSICAL	= LOW_HEAP_LOCATION_VIRTUAL & 0xFFFFF,
	LOW_HEAP_SIZE				= 0x00020000,

	DRIVERS_MEMORY_BEGIN		= 0xA0000000,
	DRIVERS_MEMORY_END			= 0xC0000000,

	SHAREDLIBS_MEMORY_BEGIN		= 0x80000000,
	SHAREDLIBS_MEMORY_END		= 0xA0000000,

	KERNEL_AREA_BEGIN			= DRIVERS_MEMORY_BEGIN,
	KERNEL_AREA_END				= 0xF0000000,
};

#define HANDLES_PER_PROCESS	5
#define MAX_DMA_BUFFERS		2
#define DEFAULT_NAME_SIZE	32

//from NewLib
#include <string.h>
#include <time.h>
//from Global include folder
#include <headers/x86.h>
#include <inlines.h>
#include <Supernova.h>
#include <Supernova/di.h>
//Kernel typedefs
typedef struct handle_s *handle_p;
typedef struct thread_s *thread_p;
typedef struct process_s *process_p;
typedef struct driver_s *driver_p;
typedef struct device_s *device_p;
typedef struct IPCMsgQueue_s *IPCMsgQueue_p;

//Kernel headers
#include "headers/version.h"
#include "headers/Exec.h"
#include "headers/cpu.h"
#include "headers/klibc.h"
#include "headers/vfs.h"
#include "headers/interrupts.h"
#include "headers/memmgr.h"
#include "headers/console.h"
#include "headers/DeviceMgr.h"
#include "headers/time.h"
#include "headers/libmgr.h"
#include "headers/ipc.h"
#include "headers/task.h"

#define SFS_BuildDateTime EncodeDate(COMPILE_DATE_YEAR, COMPILE_DATE_MONTH, COMPILE_DATE_DAY, COMPILE_TIME_HOUR, COMPILE_TIME_MINUTE)

//kernel.c
uint32 SupernovaMainThread();
void SystemHalt();
extern const SystemInfo_t SystemInformation;
void ReleaseLock(SPINLOCK *sp);
uint32 AcquireLock(SPINLOCK *sp);

//start.asm
void CPU_Reset();

//int0x80.c
void int0_Sleep(uint32 ms);

#define TestFlags(A,F) (((A) & (F)) == F)
#define CriticalCheck(A)	\
	if(A){					\
		kprintf("%4tCritical error '%s' (in %s at %d line)%t", #A, __FILE__, __LINE__);	\
		SystemHalt();		\
	}						\


#endif 
