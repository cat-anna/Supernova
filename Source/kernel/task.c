#define _COMPILING_MILTITASKING_
#include "kernel.h"
#include <headers/elf.h>

static void Clock_Init(/*uint16 hz*/);

volatile thread_p CurrentThread;
volatile uint32 KernelAreaVersion;

//Threads Queuing
static thread_p DequeueThread();
static void EnqueueThread(thread_p th);

//Process | Thread initialization
static int GetFreeProcessEntry(process_p *proc);
static int GetFreeThreadEntry(thread_p *thread);
static uint32 ProcessAllocateStack(process_p proc);

uint32 InitStackRing0(thread_p th, uint32 ProcessIOPL, uint32 entry, uint32 Selectors, uint32 ParamCount, uint32 *ParamTable);

typedef struct ThreadPriorityQueue_s{
	thread_p First;
	thread_p Last;
}ThreadPriorityQueue_t, *ThreadPriorityQueue_p;

ThreadPriorityQueue_t ThreadQueues[PRIORITY_AMOUNT];

SPINLOCK ThreadQueueLock;

static thread_p DequeueThread(){
	AcquireLock(&ThreadQueueLock);
	uint32 i;
	for(i = 0; i < PRIORITY_AMOUNT; ++i){
		ThreadPriorityQueue_p pq = ThreadQueues + i;
		if(pq->First){
			thread_p th = pq->First;
			pq->First = th->Next;
			if(!pq->First) pq->Last = 0;
			ReleaseLock(&ThreadQueueLock);
			return th;
		}
		else
			continue;
	}
	ReleaseLock(&ThreadQueueLock);
	return 0;
}

static void EnqueueThread(thread_p th){
	AcquireLock(&ThreadQueueLock);
//	kprintf("EnqueueThread %s\n", th->OwnerProcess->name);
	ThreadPriorityQueue_p pq = ThreadQueues + th->priority;
	th->Next = 0;
	if(!pq->First){
		pq->First = th;
	} else {
		pq->Last->Next = th;
	}
	pq->Last = th;
/*	thread_p t = pq->First;
	puts("EnqueueThread ");
	while(t){
		kprintf("%s->", t->OwnerProcess->name);
		t = t->Next;
	}
	puts("0\n");*/
	ReleaseLock(&ThreadQueueLock);
}

void timer_handler(){
	if(CurrentThread->state == STATE_READY)
		EnqueueThread(CurrentThread);
	CurrentThread = DequeueThread();
//	*((short*)0xC00B8000) = 0x2030 + GetCurrentPID();
}

__attribute((noreturn))
void ChangeThread(){
	timer_handler();
	asm volatile("jmp RestoreThread");
	while(1);
}

__attribute((noreturn))
void CPU_Idle(){
//	puts("CPU Idle\n");
	while(1){
	//	asm("hlt");
	}
}

void UpdateProcess(process_p proc){
	if(proc->IPCQueue)
		IPC_UpdateBuffer(proc);
	if(proc->cr3_version != KernelAreaVersion)
		UpdateProcessKernelArea(proc);
}

uint32 Multitasking_init(){
//make sure that tables are zeroed
	memset(ProcessTable, 0, PROCESS_TABLE_SIZE);
	memset(ThreadsTable, 0, THREADS_TABLE_SIZE);
	memset(ThreadQueues, 0, sizeof(ThreadPriorityQueue_t) * PRIORITY_AMOUNT);
#ifdef _DEBUG_
//check whether there is not space for desired tables count
	if(PROCESS_TABLE_SIZE < sizeof(process_t) * PROCESS_TABLE_COUNT){
		kprintf("%4tRequested Processes Table Count does not fit in allocated space%t\n");
	}
	if(THREADS_TABLE_SIZE < sizeof(thread_t) * THREADS_TABLE_COUNT){
		kprintf("%4tRequested Threads Table Count does not fit in allocated space%t\n");
	}
#endif
//initialize clock
	Clock_Init(/*100*/);
//initialize CPU idle process and thread(s)
	{
		process_p idle_proc;
		uint32 Iddle_PD = CreateSecuredPageDirectory();
		BeginProcess(&idle_proc, "CPU-idle", Iddle_PD);
		idle_proc->state = STATE_READY;
		thread_p idle_thread;
		BeginThread(&idle_thread, idle_proc, PRIORITY_IDLE);
		uint32 Idle_stack[1] = {(uint32)CPU_Idle};
		InitStackRing0(idle_thread, 0, (uint32)SafeLaunch, SEGMENT_SELECTORS_RING0, 1, Idle_stack);
		RunThread(idle_thread);
	}
//Initialize Supernova process and thread(s)
	{
		process_p Supernova_proc;
		BeginProcess(&Supernova_proc, "Supernova.elf", Kernel_CR3);
		Supernova_proc->state = STATE_READY;
		Supernova_proc->flags |= PROCESS_FLAG_KERNEL;
	//kernel thread
		thread_p Supernova_thread;
		BeginThread(&Supernova_thread, Supernova_proc, PRIORITY_NORMAL);
		uint32 SN_stack[1] = {(uint32)SupernovaMainThread};
		InitStackRing0(Supernova_thread, 0, (uint32)SafeLaunch, SEGMENT_SELECTORS_RING0, 1, SN_stack);
		Supernova_thread->flags |= PROCESS_FLAG_KERNEL;
		RunThread(Supernova_thread);
	//device manager thread
		thread_p DeviceManager_thread;
		BeginThread(&DeviceManager_thread, Supernova_proc, PRIORITY_NORMAL);
		uint32 Dev_stack[1] = {(uint32)Supernova_DeviceManager};
		InitStackRing0(DeviceManager_thread, 0, (uint32)SafeLaunch, SEGMENT_SELECTORS_RING0, 1, Dev_stack);
		DeviceManager_thread->flags |= PROCESS_FLAG_KERNEL;
		RunThread(DeviceManager_thread);
		InitIPCBuffer((IPCContentInfo_p)DEVMGR_IPC_LOCATION);
	}
	CurrentThread = ThreadsTable + THREADS_TABLE_COUNT - 1;		//set a dummy pointer
	KernelAreaVersion = 1;
	return SUCCES;
}

sint32 BeginProcess(process_p *proc, const char *Name, uint32 PageDir){
	if(!proc || !Name)return -1;
	process_p NewProc;
	sint32 pid = GetFreeProcessEntry(&NewProc);
	if(pid < 0)return -1;
	strcpy(NewProc->name, Name);
	NewProc->cr3 = PageDir;
	NewProc->Image.cr3 = PageDir;
	*proc = NewProc;
	return pid;
}

sint32 BeginThread(thread_p *th, process_p proc, uint32 Priority)
{
	if(!th || !proc)return -1;
	thread_p NewThread;
	sint32 tid = GetFreeThreadEntry(&NewThread);
	if(tid < 0) return -1;
	if(!proc->MainThread)proc->MainThread = NewThread;
	++proc->ThreadsCount;
	NewThread->OwnerProcess = proc;
	NewThread->priority = Priority;
	*th = NewThread;
	return tid;
}

uint32 RunThread(thread_p th){
	if(!th) return ERRORCODE_WRONG_INPUT;
	if(!th->OwnerProcess){
		puts("RunThread ERRORCODE_FATAL 1\n");
		return ERRORCODE_FATAL;
	}
	if(th->OwnerProcess->state != STATE_READY){
		puts("RunThread ERRORCODE_FATAL 2\n");
		return ERRORCODE_FATAL;
	}
//kprintf("RunThread %s\n", th->OwnerProcess->name);
	th->state = STATE_READY;
	EnqueueThread(th);
	return SUCCES;
}

typedef struct stack_r0{
	uint32 ds;				//other data selectors
	uint32 edi;				//  ||
	uint32 esi;				//  ||
	uint32 ebp;				//  ||
	uint32 esp;				//  ||	Pushed by pushad
	uint32 ebx;				//  ||
	uint32 edx;				//  ||
	uint32 ecx;				//  ||
	uint32 eax;				//  ||
	uint32 ReturnAddres;	//or the beginning of the code
	uint32 cs;				//cs for ring 3
	uint32 eflags;
	uint32 esp3;			//esp for ring 3
	uint32 ss3;				//ss for ring 3
}stack_r0_t, *stack_r0_p;

uint32 InitStackRing0(thread_p th, uint32 ProcessIOPL, uint32 entry, uint32 Selectors, uint32 ParamCount, uint32 *ParamTable){
	uint32 data_s = Selectors & 0xFFFF;
	uint32 StackLocation = ProcessAllocateStack(th->OwnerProcess);
	if(!StackLocation)return ERRORCODE_FATAL;
	th->Stack0Start = StackLocation + PROCESS_STACK_SIZE;
	th->Stack0Size = INIT_STACK_SIZE;

	uint32 newStack[30];
	memset(newStack, 0, 30 * sizeof(uint32));

	uint32 *stack = newStack + 30;

	if(ParamCount > 0){
		int i;
		for(i = ParamCount - 1; i >= 0; --i){
			--stack;
			*stack = ParamTable[i];
		}
	}

	uint32 NoR3Ret = ParamCount > 0 || th->stack3 == 0;
	if(NoR3Ret)	stack += 2;		//skip esp3 and ss3
	stack_r0_p int_stack = (stack_r0_p)((uint32)stack - sizeof(stack_r0_t));
	if(!NoR3Ret){
		int_stack->ss3 			= data_s;
		int_stack->esp3			= th->stack3;
	}
	int_stack->eflags		= 0x0202 | (ProcessIOPL << 12);
	int_stack->cs			= (Selectors >> 16) & 0xFFFF;
	int_stack->ReturnAddres	= entry;
	int_stack->ds			= data_s;

	uint32 StackDataSize = sizeof(uint32) * 30 - ((uint32)int_stack - (uint32)newStack);
//	kprintf("InitStackRing0 %x->%x %x\n", th->OwnerProcess->cr3, StackDataSize, th->Stack0Start - StackDataSize);

	th->stack = th->Stack0Start - StackDataSize;

	AllocAndWriteMemory(th->OwnerProcess->cr3, th->Stack0Start - StackDataSize, int_stack, StackDataSize, MEM_FLAGS_Pr_Wr);
	return SUCCES;
}

uint32 InitStackRing3(thread_p th){
	uint32 s3 = ProcessAllocateStack(th->OwnerProcess);
	if(!s3) return ERRORCODE_FATAL;

	th->Stack3Start = s3 + PROCESS_STACK_SIZE;
	th->Stack3Size = INIT_STACK_SIZE;
	uint32 tmp = 0;
	AllocAndWriteMemory(th->OwnerProcess->cr3, th->Stack3Start - sizeof(uint32), &tmp, sizeof(uint32), MEM_FLAGS_Pr_Wr_Usr);

//	AllocateMemory(th->Stack3Start - INIT_STACK_SIZE, th->Stack3Start, PAGE_FLAGS_Pr_Wr);
	th->stack3 = th->Stack3Start - sizeof(uint32);

	return SUCCES;
}

uint32 Thread_WakeUp(TimeEvent_p timer){
	thread_p th = (thread_p)timer->Event;
//	kprintf("Task_WakeUp %x\n", timer->Event);
	if(th->state != STATE_SLEEP_TIME)return 0;
	th->state = STATE_READY;
	EnqueueThread(th);
	return 0;
}

void PutThreadToSleep(thread_p th, uint32 sleeptime){
	if(sleeptime == 0) return;
	th->state = STATE_SLEEP_TIME;
	SetTimeEvent(0, (uint32)th, sleeptime, 0, &Thread_WakeUp);
}

/*
	ls -R1 /path |
  	  	  while read l; do case $l in *:) d=${l%:};; "") d=;; *) echo "$d/$l";; esac; done
*/

uint32 RunUserProcess(void *Data, uint32 *PID, const char *Name){
	if(!Data)return ERRORCODE_FATAL;

	Elf32_Ehdr *elf = (Elf32_Ehdr*)Data;
	if(!IS_ELF(elf))return ERRORCODE_WRONG_INPUT;

	uint32 NewPD = CreateSecuredPageDirectory();
	process_p proc;
	uint32 pid = BeginProcess(&proc, Name, NewPD);
	if(PID)*PID = pid;
	thread_p th;
	BeginThread(&th, proc, PRIORITY_NORMAL);

	uint32 tmp_stack = CurrentThread->Stack0Start - CurrentThread->Stack0Size;
	HANDLE H_map = MapOnePageTable(tmp_stack, NewPD, tmp_stack);
	if(H_map == INVALID_HANDLE_VALUE){
		return ERRORCODE_FATAL;
	}


	uint32 tmp_cr3 = get_cr3();
	ldcr3(NewPD);
	Elf32_LoadImage(Data, PROCESS_CODE_START, MEM_FLAGS_Pr_Wr_Usr, &proc->Image);
	Elf32_ProcessDynamicSection(proc);
	ldcr3(tmp_cr3);
	UnMapPageTable(H_map);

	InitStackRing3(th);
	InitStackRing0(th, 3, proc->Image.Entry, SEGMENT_SELECTORS_RING3, 0, 0);

	proc->state = STATE_READY;
	RunThread(th);
	return SUCCES;
}

static uint32 ProcessAllocateStack(process_p proc){
	uint32 stid = 0;
	uint32 i;
	uint32 stack = 0;
	static SPINLOCK ThisLock;
	AcquireLock(&ThisLock);
	for(i = 1; stid < 32; i <<= 1, ++stid)
		if(!(proc->StackBitmap & i)) break;
//	kprintf("STACK id:%d b:%8x i:%d n:%s\n", stid, proc->StackBitmap,i, proc->name);
	if(stid < 32){
		proc->StackBitmap |= i;
		stack = PROCESS_STACK_AREA_BEGIN + stid * PROCESS_STACK_SIZE;
//		kprintf("\ts:%8x\n",stack);
	}
	ReleaseLock(&ThisLock);
	return stack;
}

/*
void proc_kill(uint32 pid, uint32 return_code, uint32 error_code1){
	process_p proc = &proctab[pid];
	if(pid != current_pid)ldcr3(proc->cr3);
	proc->state = PROC_STATE_KILLED;

	uint32 *PD = (uint32*)0xFFFFF000;
	while((uint32)PD < 0xFFFFFC00){//clean 0-767 PD entry's
		if(*PD != 0){//if PD entry exists
			uint32 *PT = (uint32*)(0xFFC00000 + (0x1000 * (((uint32)PD >> 2) & 0x3FF)));
			uint32 i;
			for(i = 0; i < 0x100; i++, PT++)//clean PT for existing PD entry
				if(*PT != 0){
					FreePage(*PT);
					*PT = 0;
				}
			FreePage(*PD);
			*PD = 0;
		}
		PD++;
	}

	if(pid != current_pid)ldcr3(proctab[current_pid].cr3);
	else ldcr3(Kernel_Phys);
	FreePage(proc->cr3);

	if(proc->flags & PROC_FLAG_OWNER_WAITING){
		proctab[proc->owner].state = PROC_STATE_READY;
		proctab[proc->owner].sleep_result = return_code;
		proctab[proc->owner].flags |= PROC_FLAG_SLEEP_RESULT;
	}

	//if(proc->flags & PROC_FLAG_DRIVER){

	//}
//	printf("process %d is exiting\n", pid);
	if(proc->flags & PROC_FLAG_SHELL){
		kprintf("Shell has exit with return code %d\n", return_code);
	}

	if(error_code1){
		kprintf("%u%4fProcess %s (PID:%d) caused an initialization error (code:%d)\nProcess has been closed%o\n\n\n", ProcessList[current_pid].name, current_pid, error_code1);
	}

	ZeroMemory(&proctab[pid], sizeof(task_t));
}*/

/*
void* GetMessageHandler(uint32 pid, HANDLE H){
	if(!CheckPID(pid))return 0;
	handle_p h = CheckProcessHandle(H, &ProcessList[pid]);
	if(!h || h->type != HANDLE_TYPE_MESSAGE_HANDLER){
		uint32 i = 0;
		h = ProcessList[pid].handles;
		for(; i < HANDLES_PER_PROCESS; i++, h++)
			if(h->type == HANDLE_TYPE_MESSAGE_HANDLER && (h->flags & HANDLE_FLAG_OPENED))
				return (void*)h->data.u32;
		return 0;
	}
	return (void*)h->data.u32;
}
*/
/*
uint32 Task_DoTimer(TimeEvent_p timer){
	DECLARE_MESSAGE_T(msg)
	msg.header.message = MSG_TIMER;
	msg.header.sender = timer->o_pid;
	msg.header.sender_handle = INVALID_HANDLE_VALUE;
	msg.hparam = timer->Event;

	ipc_sendmessage(timer->o_pid, GetMessageHandler(timer->o_pid, timer->handle), (msg_header_p)&msg);
	return 0;
}*/
/*
void SetProcessTimer(uint32 pid, HANDLE h, uint32 ev, uint32 ms){
	SetTimeEvent(h, ev, ms, pid, &Task_DoTimer);
}

void KillProcessTimer(uint32 pid, HANDLE h, uint32 ev){
	KillTimeEvent(h, ev, pid);
}

void ProcessEnterCriticalSection(int Requested_time){
	//here should be some checks
	if(Requested_time < 0) CriticalSectionTime = 0xFFFFFFFF;
	else CriticalSectionTime = Requested_time;
}

void ProcessLeaveCriticalSection(){
	CriticalSectionTime = 0;
}
*/
/*
handle_p CheckProcessHandle(HANDLE H, process_p proc){
	if(H >= HANDLES_PER_PROCESS)return 0;
	handle_p h = &proc->handles[H];
	if(!(h->flags & HANDLE_FLAG_OPENED))return 0;
	return h;
}*/

/*
uint32 proc_getmem(uint32 pid, uint32 address)
{
	uint32 *cr3 = (uint32*)proctab[pid].cr3;
	uint32 *PT;
	PT = (uint32*)(cr3[address >> 22] & 0xFFFFF000);
	if((uint32)PT == 0)	return 0;
	uint32 PTi = (address >> 12) & 0x3FF;
	if(PT[PTi] == 0)return 0;
	return ( PT[PTi] & 0xFFFFF000 ) | ( address & 0xFFF );
}*/
/*
uint32 proc_allocmem(uint32 pid, uint32 address)
{
	uint32 *cr3 = (uint32*)proctab[pid].cr3;

	uint32 *PT;
	PT = (uint32*)(cr3[address >> 22] & 0xFFFFF000);
	if((uint32)PT == 0){//PageTable nie istnieje - tworzymy go
		cr3[address >> 22] = AllocatePage() | 7;// user mode, read/write, present
		PT = (uint32*)(cr3[address >> 22] & 0xFFFFF000);
		proctab[pid].size++;
		memset(PT, 0, 0x1000);
	}
	uint32 PTi = (address >> 12) & 0x3FF;
	if(PT[PTi] == 0){
		PT[PTi] = AllocatePage() | 7;
		proctab[pid].size++;
		memset((uint32*)(PT[PTi] & 0xFFFFF000), 0, 0x1000);
	}
	return ( PT[PTi] & 0xFFFFF000 ) | ( address & 0xFFF );
}*/
/*
void proc_freemem(uint32 pid, uint32 address)
{
	uint32 *cr3 = (uint32*)proctab[pid].cr3;

	uint32 *PT = (uint32*)(cr3[address >> 22] & 0xFFFFF000);
//jeœli PageTable nie istnieje - nie ma nic do roboty
	if((uint32)PT == 0)return;

	uint32 PTi = (address >> 12) & 0x3FF;
//jeœli wpis w PT nie istnieje nie mamy nic do roboty
	if(PT[PTi] == 0)return;

//pozbywamy siê page'a z PT
	FreePage(PT[PTi] & 0xFFFFF000);
	PT[PTi] = 0;
	proctab[pid].size--;
}*/

handle_p CheckHandle(HANDLE H, uint32 htype, uint32 flags){
	uint32 hval = (uint32)H;
	if(hval <= PROCESS_TABLE_LOCATION || hval > PROCESS_TABLE_LOCATION + PROCESS_TABLE_SIZE)return 0;
	handle_p h = (handle_p)H;
	if(htype && h->type != htype)return 0;
	flags |= HANDLE_FLAG_OPENED;
	if((h->flags & flags) != flags)return 0;
	return h;
}

handle_p GetFreeHandleEntry(process_p proc){
	if(!proc)return 0;
	int i = 0;
	handle_p h = proc->HandleTable;
	for(; i < HANDLES_PER_PROCESS; ++i, ++h){
		if(!(h->flags & HANDLE_FLAG_OPENED)){
			memset(h, 0, sizeof(handle_t));
			h->flags = HANDLE_FLAG_OPENED;
			return h;
		}
	}
	return 0;
}

static int GetFreeProcessEntry(process_p *proc){
	uint32 i = 0;
	for (; i < PROCESS_TABLE_COUNT; ++i)
		if (ProcessTable[i].state == STATE_NONE){
			if(proc) *proc = ProcessTable + i;
			memset(ProcessTable + i, 0, sizeof(process_t));
			ProcessTable[i].state = STATE_INIT;
			return i;
		}
	return -1;
}

static int GetFreeThreadEntry(thread_p *thread){
	uint32 i = 0;
	for (; i < THREADS_TABLE_COUNT; ++i)
		if (ThreadsTable[i].state == STATE_NONE){
			if(thread) *thread = ThreadsTable + i;
			memset(ThreadsTable + i, 0, sizeof(thread_t));
			ThreadsTable[i].state = STATE_INIT;
			return i;
		}
	return -1;
}

uint16 PIT_reload_value;				// Current PIT reload value
volatile uint32 SysTime_fractions;		// Fractions of 1 mS since timer initialized
volatile uint32 SysTime_mS;				// Number of whole mS since timer initialized
//uint32 volatile SysTime_mS_high;		//this value is unsused right now
uint32 IRQ0_fractions;					// Fractions of 1 mS between IRQs
uint32 IRQ0_mS;							// Number of whole mS between IRQs
//uint32 IRQ0_frequency;				// Actual frequency of PIT

static void Clock_Init(/*uint16 hz*/){
//	float pit_fq = 3579545 / 3;
	/*
	float reload = pit_fq /(float)hz * 0;
*/
	PIT_reload_value = 11931;//(uint16)reload;
/*	printf("reload: %d\n", PIT_reload_value);
	float t = (uint32)reload / pit_fq * 1000.0;
	printf("t: %d\n", (uint32)(t*10000));
	float frac = (t - (uint32)t);*/

	//printf("frac: %d\n", (uint32)(frac*10000));
	IRQ0_fractions = 0xFFD324ED;//(uint32)(0xFFFFFFFF * frac);
	IRQ0_mS = 9;//(uint32)t;
//	printf("IRQ0_fractions: %d\tIRQ0_mS: %d\n", IRQ0_fractions, IRQ0_mS);
	outb(0x36, 0x43);
	outb(PIT_reload_value & 0xff, 0x40);
	outb((PIT_reload_value >> 8) & 0xff, 0x40);
}
