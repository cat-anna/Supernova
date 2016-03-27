/*
 * IA-32.h
 *
 * Declarations for IA-32 Architecture
 *
 *  Created on: 10-03-2012
 *      Author: P
 */

#ifndef _X86_H_
#define _X86_H_

#ifndef _IA32_
#error IA-32 architecture is required
#endif

typedef unsigned long long	x86_u64;
typedef unsigned 			x86_u32;
typedef unsigned short		x86_u16;
typedef unsigned char		x86_u8;

typedef unsigned long		x86_ulong;
typedef	signed	 long		x86_slong;

//-------------------------------------MEMORY-MAP-------------------------------

typedef struct {
	x86_u64 base;
	x86_u64 length;
	x86_u32 type;
	x86_u32 ext;
}__attribute((packed)) x86_MemoryMapEntry_t;

enum{
	MEM_TYPE_FREE 			= 1,//Type 1: Usable (normal) RAM
	MEM_TYPE_RESERVED 		= 2,//Type 2: Reserved - unusable
	MEM_TYPE_ACPI_RECLAIM 	= 3,//Type 3: ACPI reclaimable memory
	MEM_TYPE_ACPI_NVS 		= 4,//Type 4: ACPI NVS memory
	MEM_TYPE_BAD 			= 5,//Type 5: Area containing bad memory
};
/*Next dword = Region "type"__attribute__((packed))
Next dword = ACPI 3.0 Extended Attributes bitfield (if 24 bytes are returned, instead of 20)
Bit 0 of the Extended Attributes indicates if the entire entry should be ignored (if the bit is clear). This is going to be a huge compatibility problem because most current OSs won't read this bit and won't ignore the entry.
Bit 1 of the Extended Attributes indicates if the entry is non-volatile (if the bit is set) or not. The standard states that "Memory reported as non-volatile may require characterization to determine its suitability for use as conventional RAM."
The remaining 30 bits of the Extended Attributes are currently undefined.*/

//-----------------------------TASK-STATE-SEGMENT---------------------------------

typedef struct {
	unsigned backlink;
	unsigned esp0;
	unsigned ss0;
	unsigned esp1;
	unsigned ss1;
	unsigned esp2;
	unsigned ss2;
	unsigned cr3;
	unsigned eip;
	unsigned eflags;
	unsigned eax;
	unsigned ecx;
	unsigned edx;
	unsigned ebx;
	unsigned esp;
	unsigned ebp;
	unsigned esi;
	unsigned edi;
	unsigned es;
	unsigned cs;
	unsigned ss;
	unsigned ds;
	unsigned fs;
	unsigned gs;
	unsigned ldt;
	unsigned bmoffset;
} x86_TaskStateSegment_t;

//---------------------------PAGING---------------------------------------
enum{
	PAGE_PRESENT 			= (1 <<  0),//page is available in mem
	PAGE_WRITABLE			= (1 <<  1),//page can be written
	PAGE_USER				= (1 <<  2),//page can be accessed by user
	PAGE_WRITE_THROUGH		= (1 <<  3),//Page-level write-through; indirectly determines the memory type used to access page referenced by this entry (see Section 4.9)
	PAGE_CACHE_DISABLED		= (1 <<  4),//Page-level cache disable; indirectly determines the memory type used to access page referenced by this entry (see Section 4.9)
	PAGE_ACCESSED			= (1 <<  5),//page has been accessed
	PAGE_DIRTY				= (1 <<  6),//page has been written, ignored for PTE
	PAGE_SIZE_4MB			= (1 <<  7),//page size is 4MB, valid only for PDE
	PAGE_GLOBAL				= (1 <<  8),//valid only for PTE

//	PAGE_SYSTEM0			= (1 <<  8),//valid only for PDE
//	PAGE_SYSTEM1			= (1 <<  9),//Ignored
//	PAGE_SYSTEM2			= (1 << 10),//Ignored
//	PAGE_SYSTEM3			= (1 << 11),//Ignored

//	PAGE_PAT				= (1 << 12),// If the PAT is supported, indirectly determines the memory type used to access the 4-MByte page referenced by this entry (see Section 4.9.2); otherwise, reserved (must be 0)
};

//---------------------CONTROLL-REGISTERS------------------------------------
enum{
	CR0_PAGING_ENABLED 				= (1 << 31),
	CR0_PAGING_CACHE_DISABLED		= (1 << 30),
	CR0_PAGING_Not_Write_through	= (1 << 29),
	//Alignment Mask (bit 18 of CR0)
	//Write Protect (bit 16 of CR0)
	//Numeric Error (bit 5 of CR0)
	//Extension Type (bit 4 of CR0)
	//Task Switched (bit 3 of CR0)
	CR0_EMULATION					= (1 <<  2),
	CR0_MONITOR_COPROCESSOR			= (1 <<  1),
	CR0_PROTECTION_ENABLE			= (1 <<  0),
	//Page-level Cache Disable (bit 4 of CR3)
	//Page-level Write-Through (bit 3 of CR3)
	//Virtual-8086 Mode Extensions (bit 0 of CR4)
	// Protected-Mode Virtual Interrupts (bit 1 of CR4)
	//Time Stamp Disable (bit 2 of CR4)
	//Debugging Extensions (bit 3 of CR4)
	//Page Size Extensions (bit 4 of CR4) 4MB PAGES
	//Physical Address Extension (bit 5 of CR4)
	// Machine-Check Enable (bit 6 of CR4)
	CR4_PAGE_GLOBAL_ENABLE			= (1 <<  7),
	//Performance-Monitoring Counter Enable (bit 8 of CR4)
	CR4_OSFXSR						= (1 <<  9),//Operating System Support for FXSAVE and FXRSTOR instructions
	CR4_OSXMMEXCPT 					= (1 << 10),//Operating System Support for Unmasked SIMD Floating-Point Exceptions
	//VMX-Enable Bit (bit 13 of CR4)
	//SMX-Enable Bit (bit 14 of CR4)
	//FSGSBASE-Enable Bit (bit 16 of CR4)
	//PCID-Enable Bit (bit 17 of CR4)
	//XSAVE and Processor Extended States-Enable Bit (bit 18 of CR4
	//SMEP-Enable Bit (bit 20 of CR4)
	//Task Priority Level (bit 3:0 of CR8)
};

//------------------------GLOBAL-DESCRIPTOR-TABLE-----------------------------------------

typedef struct{
	x86_u16	limit;               // The upper 16 bits of all selector limits.
	x86_u32	base;          // The address of the first gdt_entry_t struct.
}__attribute((packed)) GDTPtr_t, *GDTPtr_p;

typedef struct{
	x86_u16	limit_low;           // The lower 16 bits of the limit.
	x86_u16	base_low;            // The lower 16 bits of the base.
	x86_u8	base_middle;         // The next 8 bits of the base.
    x86_u16	limit_type;
    x86_u8	base_high;           // The last 8 bits of the base.
}__attribute((packed)) SegmentDescriptor_t, *SegmentDescriptor_p;

enum{
	SEGMENT_DESCRIPTOR_FLAG_4KB_UNITS		= (1 << 23),	//granularity field (multiply limit by 1b or 4kb)
	SEGMENT_DESCRIPTOR_FLAG_32BIT			= (1 << 22),	//default operation size or 16bit
	SEGMENT_DESCRIPTOR_FLAG_64BIT_SEGMENT	= (1 << 21),	//segm contains 64bit code
	SEGMENT_DESCRIPTOR_FLAG_SYSTEM			= (1 << 20),	//currently not used
	SEGMENT_DESCRIPTOR_FLAG_PRESENT			= (1 << 15),	//segmt is present in memory
	SEGMENT_DESCRIPTOR_FLAG_RING0			= (0 << 13),	//segm for r0
	SEGMENT_DESCRIPTOR_FLAG_RING3			= (3 << 13),	//segm for r3
	SEGMENT_DESCRIPTOR_FLAG_CODE_DATA		= (1 << 12),	//segm is for code and data otherwise for sys segm
//when SEGMENT_DESCRIPTOR_FLAG_CODE_DATA is set
	SEGMENT_DESCRIPTOR_TYPE_CODE			= (1 << 11),	//code segment
	SEGMENT_DESCRIPTOR_FLAG_CONFORMING		= (1 << 10),	//?
	SEGMENT_DESCRIPTOR_FLAG_READ			= (1 <<  9),	//code segm is execute/read
	SEGMENT_DESCRIPTOR_TYPE_DATA			= (0 << 11),	//data segment
	SEGMENT_DESCRIPTOR_FLAG_EXPAND_DOWN		= (1 << 10),	//data segm expands down
	SEGMENT_DESCRIPTOR_FLAG_WRITABLE		= (1 <<  9),	//data segm is read only
	SEGMENT_DESCRIPTOR_FLAG_ACCESSED		= (1 <<  8),	//code/data segm was read only
//when SEGMENT_DESCRIPTOR_FLAG_CODE_DATA is NOT set
	SEGMENT_DESCRIPTOR_TYPE_TSS_AVAILABLE	= (9  <<  8),	//entry points to available tss (32 or 64bit depends on which mode processor is)
	SEGMENT_DESCRIPTOR_TYPE_TSS_BUSY		= (11 <<  8),	//					busy
	SEGMENT_DESCRIPTOR_TYPE_CALL_GATE		= (12 <<  8),	//?
	SEGMENT_DESCRIPTOR_TYPE_INTERRUPT_GATE	= (14 <<  8),	//?
	SEGMENT_DESCRIPTOR_TYPE_TRAP_GATE		= (15 <<  8),	//?

	SEGMENT_DESCRIPTOR_CODE		= SEGMENT_DESCRIPTOR_TYPE_CODE		| SEGMENT_DESCRIPTOR_FLAG_READ |
								  SEGMENT_DESCRIPTOR_FLAG_4KB_UNITS	| SEGMENT_DESCRIPTOR_FLAG_32BIT |
								  SEGMENT_DESCRIPTOR_FLAG_PRESENT	| SEGMENT_DESCRIPTOR_FLAG_CODE_DATA,

	SEGMENT_DESCRIPTOR_DATA		= SEGMENT_DESCRIPTOR_TYPE_DATA		| SEGMENT_DESCRIPTOR_FLAG_WRITABLE |
								  SEGMENT_DESCRIPTOR_FLAG_4KB_UNITS	| SEGMENT_DESCRIPTOR_FLAG_32BIT |
								  SEGMENT_DESCRIPTOR_FLAG_PRESENT	| SEGMENT_DESCRIPTOR_FLAG_CODE_DATA,

	SEGMENT_DESCRIPTOR_TSS_R3	= SEGMENT_DESCRIPTOR_FLAG_PRESENT	| SEGMENT_DESCRIPTOR_FLAG_RING3 |
								  SEGMENT_DESCRIPTOR_TYPE_TSS_AVAILABLE,
};

static inline void SegmentDescriptor_init(SegmentDescriptor_p sd, x86_u32 base, x86_u32 limit, x86_u32 flags){
    sd->base_low    = (base & 0xFFFF);
    sd->base_middle = (base >> 16) & 0xFF;
    sd->base_high   = (base >> 24) & 0xFF;
    sd->limit_low   = (limit & 0xFFFF);
    sd->limit_type  = (((limit & 0x0F0000) | flags) >> 8) & 0xFFFF;
}

//---------------------------INTERRUPT-VECTOR-TABLE-----------------------------------------------

typedef struct{
	x86_u16	limit;
	x86_u32	base;
}__attribute((packed)) IDTPtr_t, *IDTPtr_p;

typedef struct {
	x86_u16	base_lo;             // The lower 16 bits of the address to jump to when this interrupt fires.
	x86_u16	sel;                 // segment selector.
	x86_u16	flags;               // flags. See documentation.
	x86_u16	base_hi;             // The upper 16 bits of the address to jump to.
}__attribute((packed)) idt_entry_t, *idt_entry_p;

static inline void InterruptDescriptor_init(idt_entry_p idte, x86_u32 base, x86_u16 sel, x86_u16 flags){
	idte->base_lo = base & 0xFFFF;
	idte->base_hi = (base >> 16) & 0xFFFF;
	idte->sel     = sel;
	idte->flags    = flags;
}

enum {
	INTERRUPT_DESCRIPTOR_PRESENT		= (1 << 15),
	INTERRUPT_DESCRIPTOR_RING3			= (3 << 13),
	INTERRUPT_DESCRIPTOR_RING0			= (0 << 13),
	INTERRUPT_DESCRIPTOR_32BIT			= (1 << 11),

	INTERRUPT_DESCRIPTOR_TASK_GATE		= (5 << 8),
	INTERRUPT_DESCRIPTOR_IRQ_GATE		= (6 << 8),
	INTERRUPT_DESCRIPTOR_TRAP_GATE		= (7 << 8),
};

//#define TRAP_GATE 0x8F00
//#define IRQ_GATE 0x8E00
//#define CALL_GATE 0x8C00
//#define USER_CALL_GATE 0xEE00

#endif /* IA_32_H_ */
