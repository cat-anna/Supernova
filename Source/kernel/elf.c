#include "kernel.h"
#include <headers/ELF.h>

static uint32 Elf32_GetLoadOffset(Elf32_Ehdr *header, uint32 DestAddr);
static uint32 Elf32_DynamicBasicProcess(ExecImage_p Image, Elf32_Dyn *dyn);
static uint32 ELF32_GetImportedFunctionAddress(process_p proc, const char *Name, uint32 *FunAddr);
static uint32 Elf32_ProcessRelocPLT(process_p proc, Elf32_Dyn **DynPtr);
static uint32 Elf32_ProcessRelocation(ExecImage_p Image, uint32 RelOff, uint32 Relsz, uint32 RelEntSz);

/*	switch(header->e_type){
		break;
	}
	case ET_DYN:{//Shared object file

		break;
	}
//#define ET_NONE		0		 No file type
//#define ET_REL		1		 Relocatable file
//#define ET_CORE		4		 Core file
//#define ET_NUM		5		 Number of defined types
	}*/

Elf32_Shdr *ELF32_FindSection(Elf32_Ehdr *header, uint32 SectionType);
Elf32_Shdr *ELF32_GetSectionByAddress(Elf32_Ehdr *header, uint32 VAddr);
void Elf32_ApplyOneRelocation(Elf32_Rel *Rel, uint32 Offset, uint32 NewAddr);
uint32 ELF32_GetSymbol(ExecImage_p Image, const char *SymName, Elf32_Sym **Symbol);


uint32 Elf32_LoadImage(void *ELF_header, uint32 Addr, uint32 DestMemFlags, ExecImage_p Image){
	if(!ELF_header || !DestMemFlags || !Image)return ERRORCODE_WRONG_INPUT;
	Elf32_Ehdr *header = (Elf32_Ehdr*)ELF_header;
	if(!IS_ELF(header))return EC_WRONG_EXEC_TYPE;

	memset(Image, 0, sizeof(ExecImage_t));
	Image->LoadOffset = Elf32_GetLoadOffset(header, Addr);
	Image->ExecType = EXEC_TYPE_ELF32;
	Image->BaseAddress = (void*)Addr;

	uint32 i;
	Elf32_Phdr *proghead = (Elf32_Phdr*)((uint32)header + header->e_phoff);
	for(i = 0; i < header->e_phnum; ++i){
		uint32 PrAddr = proghead[i].p_vaddr + Image->LoadOffset;
		switch(proghead[i].p_type){
		case PT_LOAD:{// Loadable program segment
			uint32 VAddr = PrAddr;
			uint32 EndAddr = VAddr + proghead[i].p_memsz;
			EndAddr = (EndAddr & ~0xFFF) + 0x1000;
			VAddr = (VAddr & ~0xFFF);
			//AllocAndWriteMemory();
			uint32 alloc = AllocateMemory(VAddr, EndAddr, DestMemFlags);
			if(alloc != VAddr){
				FreeMemory(VAddr, EndAddr);
				return alloc;
			}
			memcpy((void*)PrAddr, (void*)((uint32)header + proghead[i].p_offset), proghead[i].p_filesz);
			break;
		}
		case PT_DYNAMIC://Dynamic linking information
			Image->DynamicData = (Elf32_Dyn*)PrAddr;
			break;
		case PT_INTERP:// Program interpreter
			//Image->Interpreter = (void*)Addr;
			break;
		case PT_PHDR:// Entry for program header table itself
			//Image->ProgramHeaders = (void*)Addr;
			break;
		}
	}

	Image->Entry = Image->LoadOffset + header->e_entry;

	if(Image->DynamicData)
		return Elf32_DynamicBasicProcess(Image, (Elf32_Dyn*)Image->DynamicData);

	return SUCCES;
//#define PT_NULL		0	/* Program header table entry unused */
//#define PT_NOTE		4	/* Auxiliary information */
//#define PT_SHLIB	5		/* Reserved */
}

uint32 Elf32_ProcessDynamicSection(process_p proc){
	if(!proc || !proc->Image.DynamicData)return ERRORCODE_WRONG_INPUT;

	ExecImage_p Image = &proc->Image;
	Elf32_Dyn *Dyn = (Elf32_Dyn*)proc->Image.DynamicData;

	for(;Dyn->d_tag != DT_NULL;++Dyn)
		switch(Dyn->d_tag){
		case DT_HASH://address of symbol hash table
//TODO: DT_HASH discover what it is
			break;

		case DT_PLTGOT:{//PLT has relocs
			uint32 r = Elf32_ProcessRelocPLT(proc, &Dyn);
			if(r != SUCCES) return r;
			break;
		}

		case DT_NEEDED:{//Name of needed library
			if(!proc)break;
			char *sname = Image->DynStrTable + Dyn->d_un.d_val;
			ProcessLoadLibrary(proc, sname);
			break;
		}
		}

//#define DT_RELACOUNT	0x6ffffff9
//#define DT_HASH		4		/* Address of symbol hash table */
//#define DT_STRTAB	5		/* Address of string table */
//#define DT_SYMTAB	6		/* Address of symbol table */
//#define DT_RELA		7		/* Address of Rela relocs */
//#define DT_RELASZ	8		/* Total size of Rela relocs */
//#define DT_RELAENT	9		/* Size of one Rela reloc */
//#define DT_STRSZ	10		/* Size of string table */
//#define DT_SYMENT	11		/* Size of one symbol table entry */
//#define DT_INIT		12		/* Address of init function */
//#define DT_FINI		13		/* Address of termination function */
//#define DT_SONAME	14		/* Name of shared object */
//#define DT_RPATH	15		/* Library search path (deprecated) */
//#define DT_SYMBOLIC	16		/* Start symbol search here */

//#define DT_DEBUG	21		/* For debugging; unspecified */
//#define DT_TEXTREL	22		/* Reloc might modify .text */
//set ExecImage values
//	Image->ImageSize = MemEnd - Addr;

	return SUCCES;
}

//-----------------------USEFULL-FUNCTIONS---------------------------------------------

void ElfDumpSymbolTable(const void *SymTab, const char *StrTab, uint32 SymTabCount){
	uint32 j;
	Elf32_Sym *ELFSymTab = (Elf32_Sym*)(SymTab);

	puts("  [no]value   |size    |bind|type|other|shndx   |name\n");
	for (j = 0; j < SymTabCount; ++j){
		kprintf("  [%2d]%8x|%8x|  %2d|  %2d|   %2x|%8x|%s\n",
			j, ELFSymTab[j].st_value, ELFSymTab[j].st_size,
			ELF32_ST_BIND(ELFSymTab[j].st_info), ELF32_ST_TYPE(ELFSymTab[j].st_info),
			ELFSymTab[j].st_other, ELFSymTab[j].st_shndx,
			StrTab + ELFSymTab[j].st_name);
	}
}

void Elf32_ApplyOneRelocation(Elf32_Rel *Rel, uint32 Offset, uint32 NewAddr){
	void *Rel_ptr = (void*)(Rel->r_offset + Offset);
	uint32 R_type = ELF32_R_TYPE(Rel->r_info);

//	kprintf("import %x: %x %x %x\n", R_type, Offset, Rel->r_offset, NewAddr);
	switch (R_type){
	case R_386_NONE://No reloc
		return;
	case R_386_RELATIVE:// Adjust by program base
//	case R_386_32://Direct 32 bit
		*((uint32*)Rel_ptr) += Offset;
		break;
//	case R_386_PC32://PC relative 32 bit
//		*((int*)Rel_ptr) += NewAddr - Offset;
//		break;
	case R_386_JMP_SLOT://Create PLT entry
		*((uint32*)Rel_ptr) = NewAddr;
		break;
	default:
		kprintf("%4tUnknown relocation type: %d%t\n", R_type);
		break;
	}
//#define R_386_GOT32	3		/* 32 bit GOT entry */
//#define R_386_PLT32	4		/* 32 bit PLT address */
//#define R_386_COPY	5		/* Copy symbol at runtime */
//#define R_386_GLOB_DAT	6		/* Create GOT entry */
//#define R_386_GOTOFF	9		/* 32 bit offset to GOT */
//#define R_386_GOTPC	10		/* 32 bit PC relative offset to GOT */
///* Keep this the last entry.  */
//#define R_386_NUM	11
}

Elf32_Shdr *ELF32_FindSection(Elf32_Ehdr *header, uint32 SectionType){
	uint32 i;
	Elf32_Shdr *section = (Elf32_Shdr*)((uint32)header + header->e_shoff);
	for(i = 0; i < header->e_shnum; ++i, ++section)
		if(section->sh_type == SectionType)
			return section;
	return 0;
}

Elf32_Shdr *ELF32_GetSectionByAddress(Elf32_Ehdr *header, uint32 VAddr){
	uint32 i;
	Elf32_Shdr *section = (Elf32_Shdr*)((uint32)header + header->e_shoff);
	for(i = 0; i < header->e_shnum; ++i, ++section)
		if(section->sh_addr == VAddr)
			return section;
	return 0;
}

uint32 ELF32_GetSymbol(ExecImage_p Image, const char *SymName, Elf32_Sym **Symbol){
	Elf32_Sym *Sym = (Elf32_Sym*)(Image->DynSymTable);
	uint32 j;
	for (j = 0; j < Image->DynSymCount; ++j, ++Sym){
		if(strcmp(SymName, Image->DynStrTable + Sym->st_name))continue;
		*Symbol = Sym;
		return SUCCES;
	}
	return EC_SHARED_SYMBOL_NOT_FOUND;
}

//----------------------STATIC-WORKERS--------------------------------------------------

static uint32 Elf32_ProcessRelocation(ExecImage_p Image, uint32 RelOff, uint32 Relsz, uint32 RelEntSz){
	Elf32_Rel *Rel = (Elf32_Rel*)(Image->LoadOffset + RelOff);
	uint32 Rel_entryc = Relsz / RelEntSz;
	uint32 j = 0;
	for(; j < Rel_entryc; ++j, ++Rel){
//		kprintf("REL: %x %x %x\n", Image->LoadOffset, Image->BaseAddress, Rel->r_offset);
		Elf32_ApplyOneRelocation(Rel, Image->LoadOffset, (uint32)Image->BaseAddress);
	}

	return SUCCES;
}

static uint32 Elf32_ProcessRelocPLT(process_p proc, Elf32_Dyn **DynPtr){

	Elf32_Rel *Rel = 0;
	uint32 Count = 0;
	Elf32_Dyn *Dyn = *DynPtr;
	ExecImage_p Image = &proc->Image;

	while(Dyn){
		switch(Dyn->d_tag){
		case DT_PLTREL:// Type of reloc in PLT
			break;

		case DT_PLTRELSZ:// Size in bytes of PLT relocs
			Count = Dyn->d_un.d_val / sizeof(Elf32_Rel);
			break;

		case DT_PLTGOT:// Processor defined value
			break;

		case DT_JMPREL:// Address of PLT relocs
			Rel = (Elf32_Rel*)(Image->LoadOffset + Dyn->d_un.d_ptr);
			break;

		default:
			*DynPtr = Dyn - 1;
			Dyn = 0;
			continue;
		}
		++Dyn;
	}
//check whether all needed values were found
	if(!Rel || !Count)
		return ERRORCODE_FATAL;

	uint32 i;
	uint32 FunAddr;
	Elf32_Sym *SymTable = (Elf32_Sym*)(Image->DynSymTable);
	for(i = 0; i < Count; ++i, ++Rel){
		Elf32_Sym *Sym = SymTable + (ELF32_R_SYM(Rel->r_info));
		char *Name = Image->DynStrTable + Sym->st_name;

		uint32 res = ELF32_GetImportedFunctionAddress(proc, Name, &FunAddr);

//		kprintf("import: %x+%x->%x(%s)\n", Image->LoadOffset, Rel->r_offset, FunAddr, Name);

		if(res == SUCCES){
			Elf32_ApplyOneRelocation(Rel, Image->LoadOffset, FunAddr);
		} else {
			kprintf("%4timport: %s not found (%x)(%x)%t\n", Name, res, FunAddr);
			return EC_SHARED_FUNCION_NOTFOUND;
		}
	}
	return SUCCES;
}

static uint32 Elf32_DynamicBasicProcess(ExecImage_p Image, Elf32_Dyn *dyn){
	uint32 Sym, SymEntSz;
	uint32 Rel, RelOff, RelSz, RelEnt;
	Sym = SymEntSz = 0;
	Rel = RelOff = RelSz = RelEnt = 0;
	for(; dyn->d_tag != DT_NULL; ++dyn)
		switch(dyn->d_tag){
//Symbol and String table
		case DT_STRTAB:
			Image->DynStrTable = (char*)(Image->LoadOffset + dyn->d_un.d_ptr);
			++Sym;
			break;
		case DT_SYMTAB:
			Image->DynSymTable = (void*)(Image->LoadOffset + dyn->d_un.d_ptr);
			++Sym;
			break;
		case DT_STRSZ:// Size of string table
			break;
		case DT_SYMENT:// Size of one symbol table entry
			SymEntSz = dyn->d_un.d_val;
			++Sym;
			break;
//relocations
		case DT_REL:// Address of Rel relocs
			RelOff = dyn->d_un.d_ptr;
			++Rel;
			break;
		case DT_RELSZ:// Total size of Rel relocs
			RelSz = dyn->d_un.d_val;
			++Rel;
			break;
		case DT_RELENT:// Size of one Rel reloc
			RelEnt = dyn->d_un.d_val;
			++Rel;
			break;
		}

//calculate symbol table entry count, of curse if all needed values are valid
	if(Sym >= 3)
		Image->DynSymCount = ((uint32)Image->DynStrTable - (uint32)Image->DynSymTable) / SymEntSz;

//Apply Relocations, if all needed values are valid
	if(Rel >= 3)
		Elf32_ProcessRelocation(Image, RelOff, RelSz, RelEnt);

	return SUCCES;
}

static uint32 ELF32_GetImportedFunctionAddress(process_p proc, const char *Name, uint32 *FunAddr){
//look for function in current image
	if(proc->Image.DynSymTable){
		Elf32_Sym *Sym;
		if(ELF32_GetSymbol(&proc->Image, Name, &Sym) != SUCCES) return EC_SHARED_NOTFOUND;

		if(Sym->st_shndx > 0 && ELF32_ST_TYPE(Sym->st_info) == STT_FUNC){
			*FunAddr = (uint32)proc->Image.BaseAddress + Sym->st_value;
			return SUCCES;
		}
	}

	return ProcessGetSharedFunction(proc, Name, FunAddr);
}

static uint32 Elf32_GetLoadOffset(Elf32_Ehdr *header, uint32 DestAddr){
	Elf32_Phdr *proghead = (Elf32_Phdr*)((uint32)header + header->e_phoff);
	uint32 lowVaddr = proghead[0].p_vaddr;
	uint32 i;
	for(i = 1; i < header->e_phnum; ++i)
		if(lowVaddr > proghead[i].p_vaddr)
			lowVaddr = proghead[i].p_vaddr;
	return DestAddr - lowVaddr;
}
