//LCF=0x156875A1;name=memmgr.h;part=kernel;
#ifndef MEMMGR_H
#define MEMMGR_H

#ifndef _COMPILING_MEMORY_MANAGER_
extern const uint32 UsableMemoryCount;
extern const uint32 MemoryManagerOptions;
#endif

#define CURRENT_PD		((ulong*)0xFFFFF000)
#define CURRENT_CR3_MAP	((ulong*)0xFFC00000)

enum{
	Kernel_CR3 = 0xA000,

	MM_OPTION_BITMAP_DISABLED	= (1 << 10),

	SEGMENT_SELECTORS_RING0		= (0x10 | (0x08 << 16)),
	SEGMENT_SELECTORS_RING3		= (0x23 | (0x1B << 16)),

	MEM_FLAG_Wr				= PAGE_WRITABLE,
	MEM_FLAG_Pr				= PAGE_PRESENT,
	MEM_FLAG_Gl				= PAGE_GLOBAL,
	MEM_FLAG_Usr			= PAGE_USER,

	PAGE_FLAGS_Pr_Wr	 	= MEM_FLAG_Pr | MEM_FLAG_Wr,
	PAGE_FLAGS_Pr_Wr_Gl	 	= MEM_FLAG_Pr | MEM_FLAG_Wr | PAGE_GLOBAL,
	PAGE_FLAGS_Pr_Wr_Usr	= MEM_FLAG_Pr | MEM_FLAG_Wr 			  | PAGE_USER,
	PAGE_FLAGS_Pr_Wr_Usr_Gl = MEM_FLAG_Pr | MEM_FLAG_Wr | PAGE_GLOBAL | PAGE_USER,
	PAGE_FLAGS_Pr_Usr_Gl	= MEM_FLAG_Pr 				| PAGE_GLOBAL | PAGE_USER,


	MEM_FLAGS_Pr_Wr			= MEM_FLAG_Pr | MEM_FLAG_Wr,
	MEM_FLAGS_Pr_Gl			= MEM_FLAG_Pr 				| MEM_FLAG_Gl,
	MEM_FLAGS_Pr_Wr_Gl		= MEM_FLAG_Pr | MEM_FLAG_Wr | MEM_FLAG_Gl,

	MEM_FLAGS_Pr_Usr		= MEM_FLAG_Pr								| MEM_FLAG_Usr,
	MEM_FLAGS_Pr_Wr_Usr		= MEM_FLAG_Pr | MEM_FLAG_Wr					| MEM_FLAG_Usr,


	MEM_FLAGS_Pr_Wr_Usr_Gl	= PAGE_FLAGS_Pr_Wr_Usr_Gl,
};
#define Kernel_Phys Kernel_CR3

#ifdef _COMPILING_KERNEL_HEAP_
uint32 KernelHeapAlloc(uint32 ReqAddrBegin, uint32 ReqAddrEnd, uint32 dofree);
	//Only Kernel Heap is allowed to use this function
	//both addresses have to be page aligned. 'dofree' have to be 0, this value is for future use
#endif

//uint32 BitmapGetBlock(uint32 BlockSize);//allocating BlockSize * 4kb of memory, returns first address

uint32 AllocateMemory(uint32 BaseAddr, uint32 EndAddr, uint32 Flags);//Master memory allocator
uint32 FreeMemory(uint32 BaseAddr, uint32 EndAddr);//Master memory deallocator

uint32 AllocAndWriteMemory(uint32 PDPhys, uint32 VAddr, void* data, uint32 datasize, uint32 Flags);
	//Writes datasize bytes from data to PDPhys at VAddr. if VAddr does not exists, will be allocated.
uint32 AllocateKernelPageTable(uint32 Address);
	//Creates PT in Kernel PD which contain specific address, if not exists
	//Does not update current cr3

//set value in PD at PT, returns old one
uint32 PDSetPTEntry(uint32 PD, uint32 PT, uint32 value);

//Creates secured copy of kernel PD. Returns physical address.
uint32 CreateSecuredPageDirectory();

//Updates kernel area in passed proc (addresses 0xA0000000 -> 0xFFFFFFFF), desired process cr3 has to be set
void UpdateProcessKernelArea(process_p proc);

//maps SrcAddr, from current cr3, to DestAddr in DestPD
HANDLE MapOnePageTable(uint32 SrcAddr, uint32 DestPD, uint32 DestAddr);
//removes memory mapping
void UnMapPageTable(HANDLE H);

//all lines below this comment are deprecated
//DO NOT USE them

enum{	// defined also in memmgr.a
	SHARED_FRAME_ADDRESS_START		= 0xA0000000,
	SHARED_FRAME_FIRST_PDE			= (SHARED_FRAME_ADDRESS_START >> 22),

	SHARED_FRAME_ADDRESS_END		= 0xB0000000,
	SHARED_FRAME_LAST_PDE			= (SHARED_FRAME_ADDRESS_END >> 22),
};

typedef struct shared_frame_info_s{		//also defined in memmgr.asm
	uint32 src_pid;
	uint32 dst_pid;
	uint32 src_addr;
	uint32 dst_addr;
	uint32 dst_pde;
	uint32 fr_size;			//in bytes
} shared_frame_info_t, *shared_frame_info_p;

//void FreePage(uint32 PhysAddr);

//uint32 CreateSharedFrame(shared_frame_info_p sh_info);
//void FreeSharedFrame(shared_frame_info_p sh_info);

//uint32 CopyProcessMemory(uint32 src_pid, uint32 src_addr, uint32 Count, uint32 dst_pid, uint32 dst_addr);

//uint32 GetPhysicalAddress(uint32 address);//get physical address form current Pd

	
#endif














