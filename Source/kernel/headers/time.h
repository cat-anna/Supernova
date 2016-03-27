#ifndef TIME_H
#define TIME_H

struct TimeEvent;
typedef struct TimeEvent* TimeEvent_p;
typedef uint32(*TimerCallBack)(TimeEvent_p timer_info);

typedef struct TimeEvent{
	TimerCallBack CallBack;	
	HANDLE handle;
	uint32 Event;
	uint32 o_pid;	
	uint32 Time;	
	void* Next;
} TimeEvent_t;

bool SetTimeEvent(HANDLE h, uint32 Event, uint32 ms, uint32 o_pid, TimerCallBack CallBackF);
bool KillTimeEvent(HANDLE h, uint32 Event, uint32 o_pid);

//void GetSystemTime(SystemTime_p syst);

#endif
