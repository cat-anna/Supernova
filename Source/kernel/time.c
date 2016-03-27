#define _COMPILING_RTC_

#include "kernel.h"

TimeEvent_p TimeEvents, EmptyEvents;

static TimeEvent_p GetTimer(){
	if(EmptyEvents){
		TimeEvent_p ret = EmptyEvents;
		EmptyEvents = ret->Next;
		return ret;
	}
	return kmalloc(sizeof(TimeEvent_t));
}

static void FreeTimer(TimeEvent_p t){
	t->Next = EmptyEvents;
	EmptyEvents = t;
}

bool SetTimeEvent(HANDLE h, uint32 Event, uint32 ms, uint32 o_pid, TimerCallBack CallBackF){
//	kprintf("SetTimeEvent %x %d %d %d\n", TimeEvents, Event, o_pid, ms);
	//bochs_breakpoint();
//	KillTimeEvent(h, Event, o_pid);
	TimeEvent_p NewTimer = GetTimer();
	NewTimer->CallBack = CallBackF;
	NewTimer->Event = Event;
	NewTimer->handle = h;
	NewTimer->Time = SysTime_mS + ms;
	NewTimer->o_pid = o_pid;
	NewTimer->Next = 0;

	if(!TimeEvents){
		TimeEvents = NewTimer;
		return true;
	}

	if(TimeEvents->Time >= NewTimer->Time){
		NewTimer->Next = TimeEvents;
		TimeEvents = NewTimer;
		return true;
	}	
	
	TimeEvent_p lst = TimeEvents;
	while(lst->Next && lst->Time <= NewTimer->Time)	lst = lst->Next;
	NewTimer->Next = lst->Next;
	lst->Next = NewTimer;
	return true;
}

bool KillTimeEvent(HANDLE h, uint32 Event, uint32 o_pid){
	//kprintf("KillTimeEvent %x %d %d\n", TimeEvents, Event, o_pid);
	if(!TimeEvents)return false;
	if(TimeEvents->Event == Event && TimeEvents->o_pid == o_pid){
		TimeEvent_p TE = TimeEvents;
		TimeEvents = TimeEvents->Next;
		FreeTimer(TE);
		return true;
	}

	TimeEvent_p TEList = TimeEvents;
	TimeEvent_p TELast = 0;
	while(TEList && TEList->Event != Event && TEList->o_pid != o_pid && TEList->handle != h){
		TELast = TEList;
		TEList = TEList->Next;
	}
	if(!TEList || !TELast)return false;
	TELast->Next = TEList->Next;	
	FreeTimer(TEList);	
	return true;
}

void RTC_IRQ_handler(){	
	outb(0x70, 0x0C); //select register C
	inb(0x71); //just throw away contents.	
	outb(0xA0, 0x20);
	outb(0x20, 0x20);	
	
	if(!TimeEvents)return;
	if(TimeEvents->Time <= SysTime_mS){
		TimeEvent_p timer = TimeEvents;
		TimeEvents = TimeEvents->Next;
		timer->Next = 0;
//		kprintf("RTC_IRQ_handler %d %d %d\n", timer->Event, timer->o_pid, timer->Time);
//		Breakpoint();
//		uint32 ret =
				timer->CallBack(timer);
//		if(ret > 0)SetTimeEvent(timer->handle, timer->Event, ret, timer->o_pid, TimeEvents->CallBack);
		FreeTimer(timer);				
	}
}

static inline void RTC_SetRate(uint32 rate){
	rate &= 0x0F; //rate must be above 2 and not over 15. (this is a safe-guard to be sure it isn't over 15)
	outb(0x70, 0x0A); //set index to register A
	uint8 prev = inb(0x71); //get initial value of register A
	outb(0x70, 0x0A); //reset index to A
	outb(0x71, (prev & 0xF0) | rate); //write only our rate to A. Note, rate is the bottom 4 bits.
}
/*
static inline uint8 CMOS_Get(uint16 value){
	outb(0x70, value);
	return inb(0x71);
};

static inline uint8 bcd2bin(uint8 bcd){
     return ((bcd >> 4) * 10) + (bcd & 0xf);
};

void GetSystemTime(SystemTime_p st){
	if(!st)return;
//	uint8 breg = CMOS_Get(0xb);
	//while(CMOS_Get(0x0a) & 0x80)Sleep(1);	
	st->year = bcd2bin(CMOS_Get(0x09)) + bcd2bin(CMOS_Get(0x32)) * 100;
	st->month = bcd2bin(CMOS_Get(0x08));
	st->day = bcd2bin(CMOS_Get(0x07));
	st->hour = bcd2bin(CMOS_Get(0x04));
	st->min = bcd2bin(CMOS_Get(0x02));
	st->sec = bcd2bin(CMOS_Get(0x00));
}*/

uint32 RTC_init(void){
	RTC_SetRate(6);
	TimeEvents = 0;
	EmptyEvents = 0;

//enable the RTC IRQ
	outb(0x70, 0x0B); //set the index to register B
	uint8 prev = inb(0x71); //read the current value of register B
	outb(0x70, 0x0B); //set the index again(a read will reset the index to register D)
	outb(0x71, prev | 0x40); //write the previous value or'd with 0x40. This turns on bit 6 of register B

	return SUCCES;
}
