bits 32
section .text

;-----------------------------------MACROS--------------------------------

%macro debug 0
	pusha
	mov dx, 0x8A00 
	mov ax, dx
	out dx, ax	
	mov ax, 0x8aE0
	mov dx, 0x8a00
	out dx, ax	
	popa
%endmacro

%macro PUSH_REGS 0
	pushad
	push ds
	push eax
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov gs, ax
	mov fs, ax
	pop eax
%endmacro

%macro POP_REGS 0
	pop eax
	mov ds, ax
	mov es, ax
	mov gs, ax
	mov fs, ax
	popad
%endmacro

%macro STORE_ESP 0
	mov edi, [CurrentThread]	
	mov [edi + thread_t.stack], esp
%endmacro

%macro UPDATE_TSS 0
	mov [tss+4], esp
	add long [tss+4], 20
%endmacro

;----------------------STRUCTURES-------------------------------------

STRUC	thread_t
	.state			resd 1
	.flags			resd 1
	.stack			resd 1
	.stack3			resd 1
	.result			resd 1
	.OwnerProcess	resd 1
	.priority		resd 1
	
	.s0start		resd 1
	.s0size			resd 1
	.s3start		resd 1
	.s3size			resd 1
	;and much more
ENDSTRUC

STRUC process_t
	.state			resd 1
	.cr3			resd 1
	.cr3_version	resd 1
	.flags			resd 1
ENDSTRUC

;-----------------------IMPORT----------------------------------

extern CurrentThread
extern tss

extern IRQ0_fractions
extern IRQ0_mS
extern SysTime_fractions
extern SysTime_mS
extern KernelAreaVersion

extern timer_handler
extern RTC_IRQ_handler
extern irq_handler
extern isr_handler
extern UpdateProcess

extern SysCall_int80

;-----------------------------CODE------------------------------------

global timer_isr
timer_isr:
;	cli
;	debug
	PUSH_REGS
	STORE_ESP
	
	mov al , 0x20
	out 0x20 , al	
	
	mov eax, [IRQ0_fractions]
	mov ebx, [IRQ0_mS]               ; eax.ebx = amount of time between IRQs
	add [SysTime_fractions], eax     ; Update system timer tick fractions
	adc [SysTime_mS], ebx            ; Update system timer tick milli-seconds
;	adc dword [SysTime_mS_high], 0

;extern CriticalSectionTime
;	mov eax, [CriticalSectionTime]
;	test eax, eax
;	jz No_Crit_Sect
;	dec eax
;	mov [CriticalSectionTime], eax
;	jmp Skip_Proc_Change
	
.No_Crit_Sect:
	call timer_handler

global RestoreThread
RestoreThread:
	mov edi, [CurrentThread]	
	
	mov ebx, [edi + thread_t.stack3]
	test ebx, ebx
	jz .nos3
	debug
.nos3:
	
	mov esi, [edi + thread_t.OwnerProcess]
	mov ebx, [esi + process_t.cr3]
	mov cr3, ebx
	mov esp, [edi + thread_t.stack]	
;	debug	
	push esi
	call UpdateProcess
	add esp, 4
	
	btr dword [edi + thread_t.flags], 21 ;declared in task.h [PROC_FLAG_SLEEP_RESULT]
		;BIT 21->CF && clear
	jnc .Skip_Proc_Result
	;debug
	mov eax, [edi + thread_t.result]
	mov [esp + 0x20], eax
	
.Skip_Proc_Result:
	POP_REGS
	UPDATE_TSS		
;	sti
;	debug
	iret	

[GLOBAL int80_isr]
int80_isr:
	PUSH_REGS
	STORE_ESP
;	debug
	push edx
	push ecx
	push ebx
	push eax
	call SysCall_int80
;	add esp, 16
	
	mov edi, [CurrentThread]
	mov esp, [edi + thread_t.stack]

	mov [esp + 0x20], eax
	
	POP_REGS
	UPDATE_TSS
	iret

	
global unknown_interrupt_isr
unknown_interrupt_isr:
	iret

%macro IRQ_ISR 1
global irq%1
irq%1:
	cli
	PUSH_REGS
	STORE_ESP
	;debug
	push dword %1
	call irq_handler
	add esp, 4
	
	mov edi, [CurrentThread]
	mov esp, [edi + thread_t.stack]

	POP_REGS
	UPDATE_TSS
	
	sti
	iret
%endmacro

global RTC_irq
RTC_irq:
;	cli
	PUSH_REGS
	STORE_ESP
	
	call RTC_IRQ_handler

	mov edi, [CurrentThread]
	mov esp, [edi + thread_t.stack]

	POP_REGS
	UPDATE_TSS
;	sti
	iret

IRQ_ISR 1
IRQ_ISR 2
IRQ_ISR 3
IRQ_ISR 4
IRQ_ISR 5
IRQ_ISR 6
IRQ_ISR 7
;IRQ_ISR 8 RTC
IRQ_ISR 9
IRQ_ISR 10
IRQ_ISR 11
IRQ_ISR 12
IRQ_ISR 13
IRQ_ISR 14
IRQ_ISR 15
IRQ_ISR 16

%macro ISR_NOERRCODE 1
  GLOBAL isr%1
  isr%1:
    cli                         ; Disable interrupts firstly.
    push byte 0                 ; Push a dummy error code.
    push byte %1                ; Push the interrupt number.
    jmp isr_asm_handler         ; Go to our common handler code.
%endmacro

%macro ISR_ERRCODE 1
  GLOBAL isr%1
  isr%1:
    cli                         ; Disable interrupts.
    push byte %1                ; Push the interrupt number
    jmp isr_asm_handler
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

isr_asm_handler:
	PUSH_REGS
	debug
	nop
	nop
	nop
	mov eax, esp	
	push eax

    call isr_handler
	add esp, 4
	
	mov edi, [CurrentThread]	
	mov [edi + thread_t.stack], esp
	
	POP_REGS
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number
    sti
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP
