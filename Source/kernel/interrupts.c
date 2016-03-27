#define _COMPILING_INTERRUPTS_MANAGER_

#include "kernel.h"
#include <headers/x86.h>

const char* hardware_exceptions[] = {
	"Divide Error",
	"Debug",
	"NMI Interrupt",
	"Breakpoint",
	"Overflow",
	"BOUND Range Exceeded",
	"Invalid Opcode",
	"Device Not Available",
	"Double Fault",
	"Coprocessor Segment Overrun",
	"Invalid TSS",
	"Segment Not Present",
	"Stack Fault",
	"General Protection Fault",
	"Page Fault",
	"x87 FPU Floating-Point Error",
	"Alignment Check",
	"Machine-Check",
	"SIMD Floating-Point"
};

enum{
	ISR_DIVIDE_ERROR,
	ISR_DEBUG,
	ISR_NMI_INTERRUPT,
	ISR_BREAKPOINT,
	ISR_OVERFLOW,
	ISR_BOUND_RANGE_EXCEEDED,
	ISR_INVALID_OPCODE,
	ISR_DEVICE_NOT_AVAILABLE,
	ISR_DOUBLE_FAULT,
	ISR_COPROCESSOR_SEGMENT_OVERRUN,
	ISR_INVALID_TSS,
	ISR_SEGMENT_NOT_PRESENT,
	ISR_STACK_FAULT,
	ISR_GENERAL_PROTECTION_FAULT,
	ISR_PAGE_FAULT,
	ISR_X87_FPU_FLOATING_POINT_ERROR,
	ISR_ALIGNMENT_CHECK,
	ISR_MACHINE_CHECK,
	ISR_SIMD_FLOATING_POINT,
};

void isr0 ();
void isr1 ();
void isr2 ();
void isr3 ();
void isr4 ();
void isr5 ();
void isr6 ();
void isr7 ();
void isr8 ();
void isr9 ();
void isr10();
void isr11();
void isr12();
void isr13();
void isr14();
void isr15();
void isr16();
void isr17();
void isr18();
void isr19();
void isr20();
void isr21();
void isr22();
void isr23();
void isr24();
void isr25();
void isr26();
void isr27();
void isr28();
void isr29();
void isr30();
void isr31();
void timer_isr();
void irq1 ();
void irq2 ();
void irq3 ();
void irq4 ();
void irq5 ();
void irq6 ();
void irq7 ();
void RTC_irq();
void irq9 ();
void irq10();
void irq11();
void irq12();
void irq13();
void irq14();
void irq15();
void int80_isr();
void unknown_interrupt_isr();

#define idt_ptr		((IDTPtr_p)INTERRUPTS_DESCRIPTORS_POINER)
#define idt_entries	((idt_entry_p)INTERRUPTS_DESCRIPTORS_TABLE)

enum{
	ISR_TRAP		= INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_RING0 | INTERRUPT_DESCRIPTOR_IRQ_GATE | INTERRUPT_DESCRIPTOR_32BIT,
	IRQ_GATE		= INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_RING0 | INTERRUPT_DESCRIPTOR_IRQ_GATE | INTERRUPT_DESCRIPTOR_32BIT,
	USER_CALL_GATE	= INTERRUPT_DESCRIPTOR_PRESENT | INTERRUPT_DESCRIPTOR_RING3 | INTERRUPT_DESCRIPTOR_IRQ_GATE | INTERRUPT_DESCRIPTOR_32BIT,
};

static void idt_init(){
	idt_ptr->limit = sizeof(idt_entry_t) * 256 - 1;
	idt_ptr->base  = (uint32)idt_entries;

	// Remap the irq table.
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);
	outb(0x21, 0x00);
	outb(0xA1, 0x00);
	
	uint32 c;
	for(c = 48; c < 256; c++)//not from 0
		InterruptDescriptor_init(idt_entries + c, (uint32)unknown_interrupt_isr , 0x08, IRQ_GATE);
	
	InterruptDescriptor_init(idt_entries +  0, (uint32)isr0 ,		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries +  1, (uint32)isr1 ,		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries +  2, (uint32)isr2 ,		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries +  3, (uint32)isr3 , 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries +  4, (uint32)isr4 , 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries +  5, (uint32)isr5 , 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries +  6, (uint32)isr6 , 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries +  7, (uint32)isr7 , 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries +  8, (uint32)isr8 , 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries +  9, (uint32)isr9 , 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 10, (uint32)isr10, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 11, (uint32)isr11, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 12, (uint32)isr12, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 13, (uint32)isr13, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 14, (uint32)isr14, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 15, (uint32)isr15, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 16, (uint32)isr16, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 17, (uint32)isr17, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 18, (uint32)isr18, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 19, (uint32)isr19, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 20, (uint32)isr20, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 21, (uint32)isr21, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 22, (uint32)isr22, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 23, (uint32)isr23, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 24, (uint32)isr24, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 25, (uint32)isr25, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 26, (uint32)isr26, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 27, (uint32)isr27, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 28, (uint32)isr28, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 29, (uint32)isr29,	 	0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 30, (uint32)isr30, 		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 31, (uint32)isr31,		0x08, ISR_TRAP);
	InterruptDescriptor_init(idt_entries + 32, (uint32)timer_isr,	0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 33, (uint32)irq1, 		0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 34, (uint32)irq2, 		0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 35, (uint32)irq3, 		0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 36, (uint32)irq4, 		0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 37, (uint32)irq5, 		0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 38, (uint32)irq6,	 	0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 39, (uint32)irq7, 		0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 40, (uint32)RTC_irq,		0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 41, (uint32)irq9,		0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 42, (uint32)irq10, 		0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 43, (uint32)irq11, 		0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 44, (uint32)irq12,		0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 45, (uint32)irq13, 		0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 46, (uint32)irq14, 		0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 47, (uint32)irq15, 		0x08, IRQ_GATE);
	InterruptDescriptor_init(idt_entries + 0x80, (uint32)int80_isr, 0x08, USER_CALL_GATE);
	asm volatile ("lidt (%0)"::"a"((uint32)idt_ptr));
}

static inline void page_fault(Interrupt_Registers *regs){
    uint32 faulting_address;
	asm volatile("mov %%cr2, %0" : "=r" (faulting_address));
    // The error code gives us details of what happened.
    int present = !(regs->err_code & 0x1); 		// Page not present
    int rw = regs->err_code & 0x2;           	// Write operation?
    int us = regs->err_code & 0x4;          	// Processor was in user-mode?
    //int reserved = regs->err_code & 0x8;    	// Overwritten CPU-reserved bits of page entry?
 //   int id = regs->err_code & 0x10;       	// Caused by an instruction fetch?
	
    kprintf("Page Fault flags: ");
    if (present) {kprintf("present ");}
    if (rw) {kprintf("read-only ");}
    if (us) {kprintf("user-mode ");}
 //   if (reserved) {print("reserved ");}

    kprintf("\nAddress:0x%x From:0x%x\n", faulting_address, regs->eip);

	/*if(current_proc->flags & PROC_FLAG_KERNEL){
	} else
	if(current_proc->flags & PROC_FLAG_DRIVER){	
	} else {
		kprintf("Process has been closed.%o\n\n\n\n");
//		proc_kill(current_pid, 0, regs->int_no);
		while(1);
		asm volatile("jmp change_proc");
	}*/
	while(1);
}

void isr_handler(Interrupt_Registers *regs){    
	uint32 ino = regs->int_no;
	kprintf("\n%u%4fProcess (?) caused an %s (ISR:%d)\n", hardware_exceptions[ino], ino);
	switch(ino){
		/*case ISR_DIVIDE_ERROR:
			proc_kill(current_task, 0, regs->int_no);
			kprintf("Process has been closed.%o\n\n");
			asm volatile("jmp change_proc");	*/	
//		case ISR_DEBUG:
//		case ISR_NMI_INTERRUPT:
//		case ISR_BREAKPOINT:
//		case ISR_OVERFLOW:
//		case ISR_BOUND_RANGE_EXCEEDED;
//		case ISR_INVALID_OPCODE:
//		case ISR_DEVICE_NOT_AVAILABLE:
//		case ISR_DOUBLE_FAULT:
//		case ISR_COPROCESSOR_SEGMENT_OVERRUN:
//		case ISR_INVALID_TSS:
//		case ISR_SEGMENT_NOT_PRESENT:
//		case ISR_STACK_FAULT:
//		case ISR_GENERAL_PROTECTION_FAULT:
		case ISR_PAGE_FAULT:
			page_fault(regs);
			break;
//		case ISR_X87_FPU_FLOATING_POINT_ERROR:
//		case ISR_ALIGNMENT CHECK:
//		case ISR_MACHINE_CHECK:
//		case ISR_SIMD_FLOATING_POINT:
//		default:
	}
	SystemHalt();
}

typedef struct IRQData_s {
	process_p proc;
	uint32 flags;
	IRQFunction_p Handler;
}IRQData_t, *IRQData_p;

enum {
	IRQ_FLAG_USED	 = (1 << 0),
};

IRQData_t irq_handlers[17];

void irq_handler(uint32 irq){	
	if(irq >= 8)outb(0xA0, 0x20);
	outb(0x20, 0x20);

	IRQData_p irq_d = irq_handlers + irq;
	if(irq_d->flags & IRQ_FLAG_USED){
//		kprintf("irq_handler %x\n", irq_d->Handler);
		irq_d->Handler(irq);
	}
	else
		kprintf("unhandled irq %d\n", irq);
}

uint32 SetIRQHandler(uint32 IRQ, IRQFunction_p Handler)
{
	if(IRQ >= 17)return ERRORCODE_WRONG_INPUT;
	IRQData_p irq = irq_handlers + IRQ;
	if(irq->flags & IRQ_FLAG_USED)return EC_IRQ_NOT_AVAILABLE;

	irq->flags = IRQ_FLAG_USED;
	irq->Handler = Handler;
	irq->proc = CurrentThread->OwnerProcess;

	return SUCCES;
}

/*
uint32 IRQ_GetStatus(HANDLE H){
	handle_p h = CheckProcessHandle(H, current_proc);
	if(!h || h->type != HANDLE_TYPE_IRQ || h->data.u32 >= 17) return 0;
	IRQ_Data_p irq = &irq_handlers[h->data.u32];
	if(irq->pid != current_pid)return 0;
	uint32 c = irq->count;
	irq->count = 0;
	return c;
}*/

//Interrupts manager have to be rewritten
uint32 Interrupts_init(){
	idt_init();
	memset((void*)irq_handlers, 0, 17 * sizeof(IRQData_t));
	return SUCCES;
}
