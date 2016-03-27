#ifndef TASK_H
#define TASK_H

typedef struct handle_s {
	uint32 type;
	uint32 flags;
	uint32 param;
	uint32 position;
	union {
		uint32 value;
		device_p device;
		driver_p driver;
		volume_p volume;
		SharedLibrary_p lib;
	} data;
}handle_t;

typedef struct process_s{
	uint32 state;
	uint32 cr3;
	uint32 cr3_version;
	uint32 flags;
	uint32 owner;
	uint32 sleep_result;

	ExecImage_t Image;

	thread_p MainThread;
	uint32 ThreadsCount;

	char name[32];

	void* IPCBuf;
	IPCMsgQueue_p IPCQueue;

	uint32 StackBitmap;

	handle_t HandleTable[HANDLES_PER_PROCESS];
}process_t;

typedef struct thread_s{
	uint32 state;
	uint32 flags;
	uint32 stack;
	uint32 stack3;
	uint32 result;
	process_p OwnerProcess;
	uint32 priority;

	uint32 Stack0Start;
	uint32 Stack0Size;

	uint32 Stack3Start;
	uint32 Stack3Size;

	thread_p Next;		//This if for threads queuing
}thread_t;

#define ProcessTable  ((process_p) PROCESS_TABLE_LOCATION)
#define ThreadsTable  ((thread_p) THREADS_TABLE_LOCATION)

enum {
	STATE_NONE		= 0x00,
	STATE_READY		= 0x01,
	STATE_SUSPENDED	= 0x10,			//suspended without any wakening event
	STATE_SLEEP_TIME,				//sleep specified amount of milliseconds
	STATE_SLEEP_MESSAGE,			//sleep until a message is queued

	STATE_INIT		= 0x20,			//entry is used but not ready to launch

	PRIORITY_DRIVER = 0,
	PRIORITY_HIGH,
	PRIORITY_NORMAL,
	PRIORITY_LOW,
	PRIORITY_IDLE,

	PRIORITY_AMOUNT = PRIORITY_IDLE + 1,	//this is not a priority, this is queuing table size

	DEFAULT_STACK_SIZE		= 0x100000,		//1mb
	INIT_STACK_SIZE			= 0x001000,		//Stack is initialized with this size

	PROCESS_FLAG_DRIVER		= (1 << 20),
	PROCESS_FLAG_KERNEL		= (1 << 31),
};

enum {
	HANDLE_FLAG_OPENED		= (1 << 0),	//handle is ready to use
	HANDLE_FLAG_READ		= (1 << 1),	//handle can be read
	HANDLE_FLAG_WRITE		= (1 << 2),	//handle could be written

	HANDLE_FLAGS_ORW		= HANDLE_FLAG_OPENED | HANDLE_FLAG_READ | HANDLE_FLAG_WRITE,

	HANDLE_TYPE_UNKNOWN		= 0,		//unknown handle or handle is not opened
	HANDLE_TYPE_DEVICE,
	HANDLE_TYPE_DRIVER,
	HANDLE_TYPE_FILE,
	HANDLE_TYPE_MEMPRY_MAP,
	HANDLE_TYPE_SHARED_LIBARY,

};

extern volatile thread_p CurrentThread;

sint32 BeginProcess(process_p *proc, const char *Name, uint32 PageDir);
sint32 BeginThread(thread_p *th, process_p proc, uint32 Priority);
uint32 InitStackRing0(thread_p th, uint32 ProcessIOPL, uint32 entry, uint32 Selectors, uint32 ParamCount, uint32 *ParamTable);

uint32 RunThread(thread_p th);

void PutThreadToSleep(thread_p th, uint32 sleeptime);
__attribute((noreturn)) void ChangeThread();

static inline process_p CheckPID(uint32 pid)
{
	if(pid > PROCESS_TABLE_COUNT || ProcessTable[pid].state == STATE_NONE)return 0;
	return &ProcessTable[pid];
}

static inline thread_p CheckTID(uint32 thid)
{
	if(thid > THREADS_TABLE_SIZE || ThreadsTable[thid].state == STATE_NONE)return 0;
	return &ThreadsTable[thid];
}

static inline uint32 GetCurrentPID()
{
	return ((uint32)CurrentThread->OwnerProcess - PROCESS_TABLE_LOCATION) / sizeof(process_t);
}

static inline uint32 GetCurrentTID()
{
	return ((uint32)CurrentThread - THREADS_TABLE_LOCATION) / sizeof(thread_p);
}

handle_p GetFreeHandleEntry(process_p proc);
handle_p CheckHandle(HANDLE H, uint32 htype, uint32 flags);

uint32 RunUserProcess(void *Data, uint32 *PID, const char *Name);

extern void SafeLaunch();
extern volatile uint32 SysTime_mS;
extern volatile uint32 SysTime_mS_high;
extern volatile uint32 KernelAreaVersion;

enum {
	PROCESS_IPC_BUFFER				= 0x60000000,
	PROCESS_IPC_BUFFER_MAX_SIZE		= 0x00004000,	//16kb

	PROCESS_STACK_AREA_BEGIN		= 0x70000000,
	PROCESS_STACK_SIZE				= 0x00040000,	//4mb

	PROCESS_CODE_START				= 0x01000000,	//16mb
};

//All lines below this are deprecated
//DO NOT USE THEM
/*
uint32 BeginTask(char* Name, uint32 PD, uint32 entry, uint32 owner, uint32 CodeSize, uint32 PCount, 
				uint32 *PTable, bool suspended, uint32 Flags);

void proc_kill(uint32 pid, uint32 return_code, uint32 error_code1);

void Task_EventHappen(uint32 PID, uint32 event);

void SetProcessTimer(uint32 pid, HANDLE h, uint32 ev, uint32 ms);
void KillProcessTimer(uint32 pid, HANDLE h, uint32 ev);

void ProcessEnterCriticalSection(int Requested_time);
void ProcessLeaveCriticalSection();

void* GetMessageHandler(uint32 pid, HANDLE H);
*/
#endif
