#ifndef LOADER_H_
#define LOADER_H_

#define _IA32_
#include "headers/x86.h"

typedef struct{		//main declaration of this structure is in loader16.asm
	char Name[20];
	unsigned Attributes;
	unsigned Date;
	unsigned Size;
	void*	FilePtr;
}LoaderFile_t, *LoaderFile_p;

typedef struct {
	unsigned LoadedFilesCount;
	LoaderFile_t LoadedFiles[10];
	unsigned MemoryMemEntiresCount;
	x86_MemoryMapEntry_t MemoryMap[10];
	unsigned BootPartitionID;
}DataFromLoader_t, *DataFromLoader_p;

//const DataFromLoader_p DataFromLoader = (DataFromLoader_p)0x1000;
#define DataFromLoader ((DataFromLoader_p)0x1000)

#endif /* LOADER_H_ */
