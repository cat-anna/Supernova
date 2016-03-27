#ifndef _EXEC_H_
#define _EXEC_H_

enum{
	EXEC_TYPE_MASK				= 0xFF,
	EXEC_TYPE_ELF32				= 1,
	EXEC_TYPE_BUILTIN			= 10,

	EXEC_FLAG_DYNAMIC_SECTION	= (1 << 15),
	EXEC_FLAG_DYNAMIC_SYMBOLS	= (1 << 16),
};

typedef struct ExecImage_s{
	uint32 ExecType;		//type of executable image

	uint32 ImageSize;
	void* BaseAddress;		//and image header
	sint32 LoadOffset;
	uint32 Entry;

	void *DynamicData;		//pointer to dynamic section

	void *DynSymTable;
	char *DynStrTable;
	uint32 DynSymCount;

	uint32 cr3;

}ExecImage_t, *ExecImage_p;

//elf.c
uint32 Elf32_LoadImage(void *ELF_header, uint32 Addr, uint32 DestMemFlags, ExecImage_p Image);
uint32 Elf32_ProcessDynamicSection(process_p proc);

void ElfDumpSymbolTable(const void *SymTab, const char *StrTab, uint32 SymTabCount);



#ifdef _HEADER_ELF_H_

uint32 ELF32_GetSymbol(ExecImage_p Image, const char *SymName, Elf32_Sym **Symbol);

#endif

#endif























