#ifndef IRQ_H
#define IRQ_H

//Interrupts manager have to be rewritten

typedef struct registers{
	uint32 ds;
//Pushed by pushad
	uint32 edi;
	uint32 esi;
	uint32 ebp;
	uint32 esp;
	uint32 ebx;
	uint32 edx;
	uint32 ecx;
	uint32 eax;
//Pushed before isr_handler
	uint32 int_no;
	uint32 err_code;
//Pushed by the processor automatically.
	uint32 eip;
	uint32 cs;
	uint32 eflags;
	uint32 user_esp;
	uint32 ss;
}__attribute((packed)) Interrupt_Registers;

typedef uint32(*IRQFunction_p)(uint32 IRQno);

uint32 SetIRQHandler(uint32 IRQ, IRQFunction_p Handler);

#endif
