#ifndef _LIBMGR_H
#define _LIBMGR_H

#include <Supernova/SharedLibs.h>

enum{
	SHLIB_FLAG_VALID		= (1 << 0),		//Entry contain valid data

	SHLIB_FLAG_BUILTIN		= (1 << 16),
};

typedef struct SharedLibrary_s{
	uint32 BaseAddr;
	uint32 PageTable;//physical address
	uint32 Flags;
	uint32 LibraryID;
	ExecImage_t Exec;
	char Name[32];
}SharedLibrary_t, *SharedLibrary_p;

uint32 InstallLibrary(void* LibData, const char* FileName);

SharedLibrary_p GetSharedLibrary(const char* name);
uint32 GetSharedFunction(const char *LibName, const char* FunName, uint32 *FunAddr);
uint32 GetFunctionAddres(SharedLibrary_p LibPtr, const char* fname, uint32 *FunAddr);

void DumpLibrary(SharedLibrary_p Lib);


handle_p ProcessGetModuleHandle(process_p proc, const char *module);
uint32 ProcessGetSharedFunction(process_p proc, const char *Name, uint32 *FunAddr);

handle_p ProcessLoadLibrary(process_p proc, const char *sname);

#endif
