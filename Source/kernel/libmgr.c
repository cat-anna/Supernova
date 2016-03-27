#include <headers/ELF.h>
#include "kernel.h"

typedef struct BuiltinLibEntry_s {
	const char* Name;
	void *Ptr;
}BuiltinLibEntry_t, *BuiltinLibEntry_p;

BuiltinLibEntry_t DILibTable[] = {
	{"kprintf", &kprintf},

	{"RegisterDriver", &RegisterDriver},
	{"RegisterDevice", &RegisterDevice},
	{"RegisterVolume", &RegisterVolume},
	{"RegisterIRQ", &RegisterIRQ},

	{"kmalloc", &kmalloc},
	{"kfree", &kfree},
	{"Lowkmalloc", &Lowkmalloc},
	{"Lowkfree", &Lowkfree},

	{"ReleaseLock", &ReleaseLock},
	{"AcquireLock", &AcquireLock},

	{"DirectOpen", &DirectOpen},
//	{"DirectClose", &DirectClose},
	{"DirectRead", &DirectRead},
//	{"DirectWrite", &DirectWrite},
	{"DirectSeek", &DirectSeek},
	{"DirectCommand", &DirectCommand},

	{"GetDeviceName", &GetDeviceName},

	{"InternalMessage", &InternalMessage},
	{"InternalGetMessage", &InternalGetMessage},
	{"", 0},
};

#define SharedLibsTable ((SharedLibrary_p)SHAREDLIBS_TABLE_LOCATION)

static int GetSharedLibEntry(SharedLibrary_p *libptr)
{
	if(!libptr)return -1;
	SharedLibrary_p lp = SharedLibsTable;
	uint32 slbID = 0;
	while(lp->Flags & SHLIB_FLAG_VALID){
		++lp;
		++slbID;
		if(slbID >= SHAREDLIBS_TABLE_COUNT)
			return ERRORCODE_FATAL;
	}
	*libptr = lp;
	memset(lp, 0, sizeof(SharedLibrary_t));
	return slbID;
}

uint32 SharedLibs_init()
{
#ifdef  _DEBUG_
	if(SHAREDLIBS_TABLE_SIZE < SHAREDLIBS_TABLE_COUNT * sizeof(SharedLibrary_t)){
		kprintf("%4tRequested SharedLibsTable Count does not fit in allocated space%t\n");
	}
#endif
	memset(SharedLibsTable, 0, SHAREDLIBS_TABLE_COUNT * sizeof(SharedLibrary_t));

	SharedLibrary_p LibPtr = SharedLibsTable;
	uint32 slbID = GetSharedLibEntry(&LibPtr);

	LibPtr->Flags = SHLIB_FLAG_VALID | SHLIB_FLAG_BUILTIN;
	LibPtr->LibraryID = slbID;
	//LibPtr->BaseAddr = ;
	strncpy(LibPtr->Name, "di.lib", 32);

	LibPtr->Exec.ExecType = EXEC_TYPE_BUILTIN;
	LibPtr->Exec.DynSymTable = DILibTable;

	return SUCCES;
}

uint32 InstallLibrary(void* LibData, const char* FileName){
	if(!LibData)return ERRORCODE_WRONG_INPUT;
	Elf32_Ehdr *header = (Elf32_Ehdr*)LibData;
//	kprintf("|%x|%s|\n", header, header);

	if(!IS_ELF(header))return EC_WRONG_EXEC_TYPE;
	if(header->e_type != ET_DYN)return EC_EXEC_NOT_DYNAMIC;

	SharedLibrary_p LibPtr = SharedLibsTable;
	uint32 slbID = GetSharedLibEntry(&LibPtr);

	LibPtr->Flags = SHLIB_FLAG_VALID;
	LibPtr->LibraryID = slbID;
	LibPtr->BaseAddr = SHAREDLIBS_MEMORY_BEGIN + 0x400000 * slbID;

	strncpy(LibPtr->Name, FileName, 32);
/*	char *NameDot = strrchr(LibPtr->Name, '.');
	if(NameDot)
		*NameDot = 0;*/

	LibPtr->PageTable = AllocateKernelPageTable(LibPtr->BaseAddr);

	uint32 err = 0;
	err = Elf32_LoadImage(LibData, LibPtr->BaseAddr, MEM_FLAGS_Pr_Wr_Usr_Gl, &LibPtr->Exec);
	if(err){
		memset(LibPtr, 0, sizeof(SharedLibrary_t));
		return err;
	}

//	kprintf("\n\tentry:%x\n\tFList:%x\n\tmain:%x\n", LibPtr->Entry, LibPtr->Entry->FList, LibPtr->Entry->LibMain);
	return SUCCES;
}

SharedLibrary_p GetSharedLibrary(const char* Name){
	if(!Name)return 0;
	uint32 i;
	SharedLibrary_p LibPtr = SharedLibsTable;
	for(i = 0; i < SHAREDLIBS_TABLE_COUNT; ++i, ++LibPtr){
		if(strcmp(Name, LibPtr->Name) == 0)return LibPtr;
	}
	return 0;
}

uint32 GetFunctionAddres(SharedLibrary_p LibPtr, const char* fname, uint32 *FunAddr){
	if(!fname || !LibPtr || !FunAddr)return ERRORCODE_WRONG_INPUT;

	switch(LibPtr->Exec.ExecType & EXEC_TYPE_MASK){
	case EXEC_TYPE_ELF32:{
		Elf32_Sym *Sym;
		if(ELF32_GetSymbol(&LibPtr->Exec, fname, &Sym) != SUCCES) return EC_SHARED_FUNCION_NOTFOUND;
		if(Sym->st_shndx > 0 && ELF32_ST_TYPE(Sym->st_info) == STT_FUNC){
			*FunAddr = (uint32)LibPtr->Exec.BaseAddress + Sym->st_value;
			return SUCCES;
		}
		break;
	}
	case EXEC_TYPE_BUILTIN:{
		BuiltinLibEntry_p e = (BuiltinLibEntry_p)LibPtr->Exec.DynSymTable;

		for(;e->Ptr;++e){
			if(strcmp(fname, e->Name))continue;
			*FunAddr = (uint32)e->Ptr;
			return SUCCES;
		}
		break;
	}
	}

	return EC_SHARED_FUNCION_NOTFOUND;
}

void DumpLibrary(SharedLibrary_p Lib){
	if(!Lib)return;

	ElfDumpSymbolTable(Lib->Exec.DynSymTable, Lib->Exec.DynStrTable, Lib->Exec.DynSymCount);
}

handle_p ProcessGetModuleHandle(process_p proc, const char *module){
	const char *name = strrchr(module, '/');
	if(!name) name = module;

	handle_p h = proc->HandleTable;
	uint32 i = HANDLES_PER_PROCESS;
	for(; i; --i, ++h)
		if(h->type == HANDLE_TYPE_SHARED_LIBARY){
			if(strcasecmp(name, h->data.lib->Name)) continue;
			return h;
		}

	return INVALID_HANDLE_VALUE;
}

uint32 ProcessGetSharedFunction(process_p proc, const char *Name, uint32 *FunAddr){
//TODO: add necessary checks

	handle_p h = proc->HandleTable;
	uint32 i = HANDLES_PER_PROCESS;
	for(; i; --i, ++h)
		if(h->type == HANDLE_TYPE_SHARED_LIBARY){
			SharedLibrary_p lib = h->data.lib;

			uint32 ret = GetFunctionAddres(lib, Name, FunAddr);
			if(ret == SUCCES)
				return SUCCES;
		}

	return EC_SHARED_FUNCION_NOTFOUND;
}

handle_p ProcessLoadLibrary(process_p proc, const char *sname){
	const char *name = strrchr(sname, '/');
	if(!name) name = sname;
	else ++name;
//kprintf("%s: proc:%s  lib:%s\n", __FUNCTION__, proc->name, name);

	SharedLibrary_p lib = GetSharedLibrary(name);

	if(!lib)
		//{loadLibFormDisk(); InstallLibrary();}
		return INVALID_HANDLE_VALUE;

	handle_p h = GetFreeHandleEntry(proc);
	if(!h) return INVALID_HANDLE_VALUE;

	if(!(lib->Flags & SHLIB_FLAG_BUILTIN)){
		PDSetPTEntry(proc->cr3, lib->BaseAddr >> 22, lib->PageTable | MEM_FLAGS_Pr_Usr);
		FlushOneTlb((void*)lib->BaseAddr);
	}

	h->type = HANDLE_TYPE_SHARED_LIBARY;
	h->data.lib = lib;

	return h;
}
