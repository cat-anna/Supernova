#ifndef SYSDEF_TASK_H
#define SYSDEF_TASK_H

#define PROC_SYS_DATA 		(PROC_SYS_ZONE + 0x3000)
#define PROC_STACK3			(PROC_SYS_ZONE + 0x2000)
#define PROC_STACK3_END		(PROC_SYS_ZONE + 0x3000)
#define PROC_STACK			(PROC_SYS_ZONE + 0x1000)
#define PROC_STACK_END		(PROC_SYS_ZONE + 0x2000)
#define PROC_IPC			(PROC_SYS_ZONE)
#define PROC_SYS_ZONE		0xB0000000	

#define PROCESS_CODE_START		0x01000000



typedef struct{
	IPC_Content_info_t ipc;
} ProcSysData_t, *ProcSysData_p;

#define ProcSysData ((ProcSysData_p)(PROC_SYS_DATA))

#endif
