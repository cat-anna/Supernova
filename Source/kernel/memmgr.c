//LCF=0x156875A0;name=memmgr.c;part=kernel;
#define _COMPILING_MEMORY_MANAGER_
#include "kernel.h"
#include <headers/x86.h>
#include <headers/Loader.h>

void GDT_Init();

#define KERNEL_PD 		((ulong*)KernelPageDir)

uint32 UsableMemoryCount;
uint32 MemoryManagerOptions;

//#define MEM_DEBUG

//----------------PHYSICAL-MEMORY-ALLOCATOR--------------------------

uint32 *PageStack;
uint32 PageStackStart;
uint32 PageStackEnd;

uint32 StackGetPage(){
	while(*PageStack == 0)--PageStack;
	uint32 Page = *PageStack;
	*PageStack = 0;
	--PageStack;
	return Page;
}

void FreePage(uint32 PhysAddr){
	PhysAddr &= 0xFFFFF000;
	if(PhysAddr > 0)*++PageStack = PhysAddr;
}

uint32 PageBitmapStart;
uint32 PageBitmapEnd;
uint32 PageBitmapFirstAddress;

uint32 BitmapGetBlock(uint32 BlockSize)
{
	if(MemoryManagerOptions & MM_OPTION_BITMAP_DISABLED)return 0;
	if(BlockSize == 0)return 0;
	uint32 *Bitmap = (uint32*)PageBitmapStart;
	uint32 addr = 0;
//	Breakpoint();
	while((uint32)Bitmap < PageBitmapEnd){
		if(!*Bitmap){
			Bitmap++;
			addr += 32 * 4 * 0x1000;
			continue;
		}
		uint32 bit = BitSearchLSB(*Bitmap);
		if(BlockSize < 32 - bit){
			addr += bit * 4 * 0x1000;
		//	register unsigned tmp;
			//tmp = *Bitmap;
			while(BlockSize--){
				BitClear(Bitmap, bit);
				bit++;
			}
		//	*Bitmap = tmp;
			return addr + PageBitmapFirstAddress;
		} else {
		//case when BlockSize is bigger than free pages in this loop
			return 0;
		//ignored
		}
		return 0;
	}
	return 0;
}

//-------------KERNEL-MMGR----------------------------

enum{
	MainKernelPageTable 	= 0xB000,

	KernelPageDir			 = 0xC000A000,
};

static inline void memswap(void *dest, void *src, unsigned len){
    char *s = (char*)src;
    char *d = (char*)dest;
    for(; len != 0; len--){
    	char t = *s;
    	*s++ = *d;
    	*d++ = t;
    }
}

uint32 MemoryManager_init(void){
	MemoryManagerOptions = 0;
	ulong *kernelpagedir = (ulong*)0xC000A000;
	ulong *lowpagetable = (ulong*)0xC000B000;

	uint32 i = 0;
	for (i = 0; i < 1024; i++)
	{
		kernelpagedir[i] = 0;			//Clear the page directory entries
		if (i < 256)	//Map only first 1Mb to Kernel_CR3
			lowpagetable[i] = (i << 12) | PAGE_FLAGS_Pr_Wr_Gl;
	}

// Fills the addresses 0...1MB and 3072MB...3073MB of the page directory with the same page table
	kernelpagedir[   0] = (ulong)(MainKernelPageTable | PAGE_FLAGS_Pr_Wr);
	kernelpagedir[ 768] = (ulong)(MainKernelPageTable | PAGE_FLAGS_Pr_Wr);
	kernelpagedir[1023] = (ulong)(Kernel_CR3 | PAGE_FLAGS_Pr_Wr);
//Enable paging and global pages
	ldcr3(Kernel_CR3);							//Set Page Directory
	ldcr4(get_cr4() | CR4_PAGE_GLOBAL_ENABLE);	//Enable Global Pages
	ldcr0(get_cr0() | CR0_PAGING_ENABLED);		//Enable Paging
//Init segmentation
	GDT_Init();
//Do Physical Memory Manager init
//Sort Memory Map
	uint32 MapCount = DataFromLoader->MemoryMemEntiresCount;
	x86_MemoryMapEntry_t *MM =  DataFromLoader->MemoryMap;
	for(i = 0; i < MapCount - 1; i++){
		uint32 smallest = i;
		uint32 j = i + 1;
		for(; j < MapCount; j++)
			if(MM[smallest].base > MM[j].base)
				smallest = j;
		if(smallest != i)
			memswap(MM + smallest, MM + i, sizeof(x86_MemoryMapEntry_t));
	}

	i = 0;
//find area starting at 1mb
	while(MM[i].base < 0x100000 && i < MapCount){i++;};
//check whether found area has at least 0xE0000 bytes of size
	if(MM[i].length <= 0xE0000){
		Console_SetTextColor(4);
		puts("%4tNot enough memory\nSupernova requires at least 2MB of memory%t\n");
		SystemHalt();
	}

	uint32 Memlen = MM[i].length;
	UsableMemoryCount = Memlen;

	uint32 Mem = MM[i].base;
	uint32 PBitmap = 0;				//count of pages in bitmap
	uint32 PStack  = Memlen >> 12;	//count of pages in stack
#ifdef _PRINT_INIT_INFO_
	kprintf("\tUsable memory found: %d@%dkb\n", PStack, Memlen >> 10);
#undef _PRINT_INIT_INFO_
#endif
//each bit in page bitmap means 16kb(4pages)of unused memory

	if(PStack > 0x700){				//if page stack has more than 0x700(count of 4kb pages in 7Mb)
		PBitmap = PStack - 0x700;	//move remain pages to bitmap
		PStack = 0x700;				//stack can not contain more than 0x700 pages
		if(PBitmap & 0x3){			//make sure amount pages in bitmap is aligned to 4 (because each bit is 16kb)
			PStack += PBitmap & 0x3;
			PBitmap &= ~0x3;
		}
	}

	PageStackStart	= 0xC0100000;
	PageStackEnd	= PageStackStart + 0x1000 + ((PStack << 2) & 0xFFFFF000);
	PageStack		= (uint32*)PageStackStart;

	PageBitmapStart = PageStackEnd;
	if(PBitmap)
		PageBitmapEnd = PageBitmapStart + ((PBitmap >> 16) & 0xFFFFF000) + 0x1000;
	else
		PageBitmapEnd = PageBitmapStart;

//	kprintf("\nPageStackStart:%x\nPageStackEnd:%x\n", PageStackStart, PageStackEnd);
//	kprintf("\nPageBitmapStart:%x\nPageBitmapEnd:%x\n", PageBitmapStart, PageBitmapEnd);
//	kprintf("\nstack:%x\nbitmap:%x\nsum:%x\n", PStack, PBitmap, Memlen >> 12);

	i = 0;
	uint32 p = (PageBitmapEnd - PageStackStart) >> 12;
	for(; i < p; i++){
		lowpagetable[256 + i] = Mem | PAGE_FLAGS_Pr_Wr;
		FlushOneTlb((void*)(0xC0000000 | Mem));
		Mem += 0x1000;
	}
	PStack -= p & 3;
	PBitmap -= p & (~3);

//	kprintf("\nstack:%x\nbitmap:%x\n", PStack, PBitmap);

	i = 0;
	//PageStack += ((PageStackEnd - PageStackStart) >> 10) - PStack;
	uint32 temp = 0x800000 - 0x1000;
#ifdef _PRINT_INIT_INFO_
		kprintf("\tPage stack: %d@%dkb\n", PStack, PStack << 2);
#endif
//	Breakpoint();
	while(PStack > 0){
		--PStack;
		*PageStack = temp;
		++PageStack;
		temp -= 0x1000;
		Mem += 0x1000;
	}
	--PageStack;
//	kprintf("\nMem:%x\tPageStack:%x\n", Mem, PageStack);

	PageBitmapFirstAddress = Mem;
	uint32 *Bitmap = (uint32*)PageBitmapStart;
//	Breakpoint();
	if(!PBitmap){
		MemoryManagerOptions |= MM_OPTION_BITMAP_DISABLED;
#ifdef _PRINT_INIT_INFO_
		kprintf("\tPage bitmap disabled\n");
#endif
	} else {
#ifdef _PRINT_INIT_INFO_
		kprintf("\tPage bitmap: %d@%dkb\n", PBitmap, PBitmap << 2);
#endif
		PBitmap >>= 2; //amout of 16 kb blocks
		while(PBitmap){
			temp = 0;
			if(PBitmap > 32){
				temp = ~temp;
				PBitmap -= 32;
			}else
				while(PBitmap){
					PBitmap--;
					BitSet(&temp, PBitmap);
				}
			*Bitmap = temp;
	//		kprintf("\ttemp: %x\tPBitmap:%x\n", temp, Bitmap);
			Bitmap++;
		}
	}
	return SUCCES;
}

//----------------ALLOCATEING-MEMORY-------------------------------

static uint32 PDAllocMem(ulong *PD, uint32 VAddr, uint32 PTCount, uint32 Flags){
//	kprintf("PDAllocMem PD:%8x VA:%8x c:%d\n", PD, VAddr, PTCount);
	uint32 FirstPD = VAddr >> 22;
	uint32 FirstPT = (VAddr >> 12) & 0x3FF;
	//uint32 done = 0;
	while(PTCount > 0){
//		Breakpoint();
		uint32 addr = 0, PagesCount = 0;

		if(PTCount >= 4){
			addr = BitmapGetBlock(1);
			PagesCount = 4;
		}
		if(!addr || PTCount == 1){
			addr = StackGetPage();
			PagesCount = 1;
		}
		if(!addr) return ERRORCODE_NO_MORE_MEMORY;

		while(PagesCount > 0){
			ulong *PT = (ulong*)(((ulong)PD & 0xFFC00000) + (0x1000 * FirstPD));
			if(PD[FirstPD] == 0){
				PD[FirstPD] = addr | Flags;
				--PagesCount;
				addr += 0x1000;
				if(PagesCount == 0)break;
				//FlushOneTlb(PT);
			}
			if(PT[FirstPT] == 0){
				PT[FirstPT] = addr | Flags;
				--PTCount;
				--PagesCount;
				addr += 0x1000;
			//	FlushOneTlb((uint32*)ReqAddrBegin);//flush new address
			}
			++FirstPT;
			if(FirstPT >= 0x400){
				FirstPT = 0;
				++FirstPD;
			}
		}
	}
	return SUCCES;
}

uint32 AllocateMemory(uint32 BaseAddr, uint32 EndAddr, uint32 Flags){
	uint32 PTCount = (EndAddr - BaseAddr) >> 12;

	uint32 r = PDAllocMem(CURRENT_PD, BaseAddr, PTCount, Flags);
	if(r != SUCCES)
		return r;
	else
		return BaseAddr;
}

uint32 FreeMemory(uint32 BaseAddr, uint32 EndAddr)
{
//	BaseAddr = EndAddr = 0;
	return (BaseAddr + EndAddr) * 0;
}

uint32 KernelHeapAlloc(uint32 ReqAddrBegin, uint32 ReqAddrEnd, uint32 dofree)
{
	if(ReqAddrBegin < KERNEL_HEAP_START || ReqAddrEnd > KERNEL_HEAP_END)
		return ERRORCODE_NO_MORE_MEMORY;
	if(dofree)
		return ERRORCODE_FATAL;
	if((ReqAddrBegin & 0x0FFF) || (ReqAddrEnd & 0x0FFF))
		return ERRORCODE_WRONG_INPUT;

	if(AllocateMemory(ReqAddrBegin, ReqAddrEnd, PAGE_FLAGS_Pr_Usr_Gl) != ReqAddrBegin)
		return ERRORCODE_FATAL;
	return 0;
}

uint32 AllocateKernelPageTable(uint32 address){
	uint32 PT = address >> 22;//get PT number
	ulong* PD = KERNEL_PD;
	if(PD[PT] != 0) return 0;//we can create new PT only if address not exists
	uint32 page = StackGetPage();//get new PT physical address
	PD[PT] = page | PAGE_FLAGS_Pr_Usr_Gl;//set new PT
	++KernelAreaVersion;
	return page;//return physical address of new PT
}

uint32 AllocAndWriteMemory(uint32 PDPhys, uint32 VAddr, void* data, uint32 datasize, uint32 Flags){
	ulong cr3 = get_cr3();
	uint32 begin = VAddr & 0xFFFFF000;
	uint32 end = VAddr + datasize;
	if(end & 0xFFF) end = (end & 0xFFFFF000) + 0x1000;

	if(PDPhys == cr3){
		AllocateMemory(begin, end, Flags);
		memcpy((void*)VAddr, data, datasize);
		return SUCCES;
	}

	uint32 Result;
	static SPINLOCK this_lock;
	AcquireLock(&this_lock);
	ulong *PD = CURRENT_PD;
	ulong *DestPD = (ulong*)0xFFBFF000;
	ulong tmp = PD[1022];
	PD[1022] = PDPhys | MEM_FLAGS_Pr_Wr;
	FlushOneTlb(DestPD);

	uint32 PTC = (end - begin) >> 12;
	Result = PDAllocMem(DestPD, begin, PTC, Flags);

	if(Result == SUCCES){
		uint32 FirstPD = begin >> 22;
		uint32 Src = (uint32)data;
		uint32 tmp2 = PD[1021];
		while(datasize > 0){
			PD[1021] = DestPD[FirstPD];
			//flush cache of whole PT
			uint32 mask = VAddr & 0xFFFFF;
			uint32 tocopy = 0x400000 - mask;
			if(tocopy > datasize) tocopy = datasize;
			uint32 dstAddr = 0xFF400000 + mask;
//	kprintf("AAWM: va:%8x  dst:%8x  Src:%8x  toc:%x  ds:%x\n", VAddr, dstAddr, Src, tocopy, datasize);
			memcpy((void*)dstAddr, (void*)Src, tocopy);
			datasize -= tocopy;
			VAddr += tocopy;
			Src += tocopy;
			++FirstPD;
		}
		PD[1021] = tmp2;
	}

	PD[1022] = tmp;
	ldcr3(get_cr3());//flush context
	ReleaseLock(&this_lock);
	return Result;
}

uint32 PDSetPTEntry(uint32 PD, uint32 PT, uint32 value){
	uint32 cr3 = get_cr3();
	ulong *CPD = CURRENT_PD;
	if(cr3 == PD){
		uint32 tmp = CPD[PT];
		CPD[PT] = value;
		return tmp;
	} else {
		uint32 cpd_tmp;
		cpd_tmp = CPD[1022];

		CPD[1022] = PD | MEM_FLAGS_Pr_Wr;
		ulong *DstPD = (ulong*)0xFFBFF000;
		FlushOneTlb(DstPD);

		uint32 tmp = DstPD[PT];
		DstPD[PT] = value;

		CPD[1022] = cpd_tmp;
		FlushOneTlb(DstPD);

		return tmp;
	}
}

//-------------------------------------------------------------------

HANDLE MapOnePageTable(uint32 SrcAddr, uint32 DestPD, uint32 DestAddr){
	handle_p h = GetFreeHandleEntry(CurrentThread->OwnerProcess);
	if(!h) return INVALID_HANDLE_VALUE;

	uint32 SrcPT = SrcAddr >> 22;
	uint32 DstPT = DestAddr >> 22;

	ulong *SrcPD = CURRENT_PD;
	ulong *DstPD = (ulong*)0xFFBFF000;
	ulong tmp = SrcPD[1022];
	SrcPD[1022] = DestPD | MEM_FLAGS_Pr_Wr;
	FlushOneTlb(DstPD);
	uint32 OldVal = DstPD[DstPT];
	DstPD[DstPT] = SrcPD[SrcPT];
	SrcPD[1022] = tmp;
	ldcr3(get_cr3());//flush whole context

	h->param = DestPD;
	h->position = DestAddr;
	h->data.value = OldVal;
	h->type = HANDLE_TYPE_MEMPRY_MAP;

	return h;
}

void UnMapPageTable(HANDLE H){
	handle_p h = CheckHandle(H, HANDLE_TYPE_MEMPRY_MAP, 0);
	if(!h)return;

	uint32 DstPT = h->position >> 22;

	ulong *SrcPD = CURRENT_PD;
	ulong *DstPD = (ulong*)0xFFBFF000;
	ulong tmp = SrcPD[1022];
	SrcPD[1022] = h->param | MEM_FLAGS_Pr_Wr;
	FlushOneTlb(DstPD);
	DstPD[DstPT] = h->data.value;
	SrcPD[1022] = tmp;

	ldcr3(get_cr3());//flush whole context

	h->flags = 0;
}

//-------------------------------------------------------------------

void UpdateProcessKernelArea(process_p proc){
	if(proc->cr3_version == KernelAreaVersion)return;	//kernel area is up to date
	if(proc->flags & PROCESS_FLAG_KERNEL){
		proc->cr3_version = KernelAreaVersion;
		return;		//do not update master kernel area
	}

	ulong *KPD = KERNEL_PD;
	ulong *CurrPD = CURRENT_PD;

	uint32 i = KERNEL_AREA_BEGIN >> 22;
	for(; i < KERNEL_AREA_END >> 22; ++i){
		ulong val = KPD[i];
		if(val) val = (val & 0xFFFFF000) | MEM_FLAGS_Pr_Wr_Gl;
		CurrPD[i] = val;
	}

	proc->cr3_version = KernelAreaVersion;
}

uint32 CreateSecuredPageDirectory(){
	uint32 NewDir = StackGetPage();//alloc address of new PD
 	//mount new PD at dummy address for modifications
	CURRENT_PD[1] = NewDir | MEM_FLAGS_Pr_Wr;
	ulong *NPD = (ulong*)((uint32)CURRENT_CR3_MAP + 0x1000);//get virtual address of new PD
	FlushOneTlb(NPD);	
	
	memset(NPD, 0, 0x1000);
	//associate secured kernel page and new page directory
	NPD[768] = MainKernelPageTable | MEM_FLAGS_Pr_Gl;

	uint32 i = KERNEL_AREA_BEGIN >> 22;
	ulong *KPD = KERNEL_PD;
	for(; i < KERNEL_AREA_END >> 22; ++i){
		ulong val = KPD[i];
		if(val) val = (val & 0xFFFFF000) | MEM_FLAGS_Pr_Wr_Gl;
		NPD[i] = val;
	}


	NPD[1023] = NewDir | MEM_FLAG_Pr;

	CURRENT_PD[1] = 0;//unmount PD
	FlushOneTlb(NPD);
	return NewDir;
}


uint32 GetPhysicalAddress(uint32 address){
    ulong PDi = (ulong)address >> 22;
    ulong PTi = (ulong)address >> 12 & 0x03FF;
 
    ulong *PD = (ulong*)0xFFFFF000;
	if(PD[PDi] == 0)return 0;
    // Here you need to check whether the PD entry is present.
 
    ulong *PT = ((ulong*)0xFFC00000) + (0x400 * PDi);
    if(PT[PTi] == 0)return 0;
    // Here you need to check whether the PT entry is present.
 
    return ((PT[PTi] & 0xFFFFF000) + ((ulong)address & 0xFFF));
}
/*
uint32 CopyProcessMemory(uint32 src_pid, uint32 src_addr, uint32 Count,
						 uint32 dst_pid, uint32 dst_addr){
	kprintf("CopyProcessMemory %d %x %d %d %x\n", src_pid, src_addr, Count, dst_pid, dst_addr);
	return 0;
	if(!src_addr || !dst_addr || !Count)return 0;
	bochs_breakpoint();
//	ldcr3(Kernel_Phys);
	
	uint32 src_cr3 = proctab[src_pid].cr3;
	uint32 dst_cr3 = proctab[dst_pid].cr3;
	uint32 read = Count;
	//uint32 kbuf_size = MinValue(100 * 1024, read);
	void *kbuf = (void*)0xC0040000;//malloc(kbuf_size);
	
	//while(read > 0){
	//	uint32 toread = MinValue(kbuf_size, read);
		CopyFromKernelSpace(src_addr, src_cr3, (uint32)kbuf, read);
		CopyFromKernelSpace((uint32)kbuf, dst_cr3, dst_addr, read);
	//	src_addr += toread;
//		dst_addr += toread;
//		read -= toread;
//	}
	
//	free(kbuf);
	
//	ldcr3(proctab[current_task].cr3);
	return Count;
}*/
/*
uint32 CreateSharedFrame(shared_frame_info_p sh_info){
	if(!sh_info)return 1;
	if(!sh_info->src_addr || !sh_info->fr_size)return 2;

	uint32 PT_count = ((((sh_info->src_addr & 0xFFF) + sh_info->fr_size) & 0xFFFFF000) >> 12) + 1;
	if(PT_count > 1024 || PT_count == 0)return 3;

//	kprintf("CreateSharedFrame %d %d %x\n", sh_info->src_pid, sh_info->fr_size, sh_info->src_addr);

	ulong* d_PD = (ulong*)0xFFFFF000;
	uint32 d_PDE = SHARED_FRAME_FIRST_PDE;
	uint32 d_PTE = 0;
	while(d_PD[d_PDE] != 0 && d_PDE < SHARED_FRAME_LAST_PDE)d_PDE++;

	ulong* s_PD = (ulong*)0xFFBFF000;
	uint32 s_PDE = (sh_info->src_addr >> 22);
	uint32 s_PTE = (sh_info->src_addr >> 12) & 0x3FF;
	d_PD[1022] = ProcessList[sh_info->src_pid].cr3 | PAGE_FLAGS_FOR_USER;
	FlushOneTlb(s_PD);
	ulong* s_PT = (ulong*)(0xFF800000 | (s_PDE << 12));
//	kprintf("s_PDE %d\t \n", s_PDE);

	FlushOneTlb(s_PT);

	ulong vaddr = d_PDE << 22;

	sh_info->dst_pde = d_PDE;
	sh_info->dst_addr = vaddr | (sh_info->src_addr & 0xFFF);
//Breakpoint();
	while(PT_count--){
		ulong *d_PT = (ulong*)(0xFFC00000 | (d_PDE << 12));
		if(d_PD[d_PDE] == 0){
			d_PD[d_PDE] = AllocatePage() | PAGE_FLAGS_FOR_USER;
			FlushOneTlb(d_PT);
		}
//		Breakpoint();
		if(s_PT)d_PT[d_PTE] = (s_PT[s_PTE] & 0xFFFFF000) | PAGE_FLAGS_FOR_USER;
		FlushOneTlb((ulong*)vaddr);
		vaddr += 0x1000;

		d_PTE++;
		if(d_PTE == 0x400){//1024
			d_PTE = 0;
			d_PDE++;
		}
		s_PTE++;
		if(s_PTE == 0x400){//1024
			s_PTE = 0;
			s_PDE++;
			s_PT = (ulong*)(0xFF800000 | (s_PDE << 12));
			FlushOneTlb(s_PT);
		}
	}
//	Breakpoint();
	d_PD[1022] = 0;
	FlushOneTlb((ulong*)0xFF8FF000);
	FlushOneTlb(s_PT);
	return 0;

	return 1;
}*/
/*
void FreeSharedFrame(shared_frame_info_p sh_info){
	if(!sh_info)return;
	ulong* PD = CURRENT_PD;
	ulong *d_PT = (ulong*)(0xFFC00000 | (sh_info->dst_pde << 12));
	FreePage(PD[sh_info->dst_pde]);
	PD[sh_info->dst_pde] = 0;
	FlushOneTlb(d_PT);
}*/

//----------------------------GLOBAL-DESCRIPTOR-TABLES---------------------------------
x86_TaskStateSegment_t	tss;

#define GDT_entries	((SegmentDescriptor_p)SEGMENTS_DESCRIPTORS_TABLE)
#define GDT_ptr		((GDTPtr_p)SEGMENTS_DESCRIPTORS_POINTER)

static void write_tss(sint32 num, uint16 ss0, uint32 esp0){
    // Firstly, let's compute the base and limit of our entry into the GDT.
    uint32 base = (uint32) &tss;
    uint32 limit = base + sizeof(tss);

    // Now, add our TSS descriptor's address to the GDT.
 //   gdt_set_gate(num, base, limit, 0xE9, 0x00);
    SegmentDescriptor_init(GDT_entries + num, base, limit, SEGMENT_DESCRIPTOR_TSS_R3);

    // Ensure the descriptor is initially zero.
    memset(&tss, 0, sizeof(tss));

    tss.ss0  = ss0;  // Set the kernel stack segment.
    tss.esp0 = esp0; // Set the kernel stack pointer.

    // Here we set the cs, ss, ds, es, fs and gs entries in the TSS. These specify what
    // segments should be loaded when the processor switches to kernel mode. Therefore
    // they are just our normal kernel code/data segments - 0x08 and 0x10 respectively,
    // but with the last two bits set, making 0x0b and 0x13. The setting of these bits
    // sets the RPL (requested privilege level) to 3, meaning that this TSS can be used
    // to switch to kernel mode from ring 3.
    tss.cs = 0x0b;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x13;
}

void GDT_Init(){
    GDT_ptr->limit = sizeof(SegmentDescriptor_t) * 6 - 1;
    GDT_ptr->base  = (x86_u32)GDT_entries;

    SegmentDescriptor_init(GDT_entries + 0, 0, 0, 0);
    SegmentDescriptor_init(GDT_entries + 1, 0, 0xFFFFFFFF, SEGMENT_DESCRIPTOR_CODE);
    SegmentDescriptor_init(GDT_entries + 2, 0, 0xFFFFFFFF, SEGMENT_DESCRIPTOR_DATA);
    SegmentDescriptor_init(GDT_entries + 3, 0, 0xFFFFFFFF, SEGMENT_DESCRIPTOR_CODE | SEGMENT_DESCRIPTOR_FLAG_RING3);
    SegmentDescriptor_init(GDT_entries + 4, 0, 0xFFFFFFFF, SEGMENT_DESCRIPTOR_DATA | SEGMENT_DESCRIPTOR_FLAG_RING3);

	write_tss(5, 0x10, 0x0);

	asm volatile("lgdt (%0)\n\t"
				 "mov $0x10, %%ax\n\t"
				 "mov %%ax, %%ds\n\t"
				 "mov %%ax, %%es\n\t"
				 "mov %%ax, %%fs\n\t"
				 "mov %%ax, %%gs\n\t"
				 "mov %%ax, %%ss\n\t"
				 ".byte 0xEA\n\t"			//opcode for far jump
				 ".long gdt_flush\n\t"		//jump address
				 ".word 0x0008\n\t"			//code selector
			  	 "gdt_flush:\n\t"
				 ::"a"((uint32)GDT_ptr));
	asm volatile("ltr %w0"::"r"((5*8) | 3));//select tss
}

