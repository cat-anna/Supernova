#ifndef _SFS_H
#define _SFS_H

#ifndef _BUILDING_SSM_
#include "Supernova.h"
#include "Supernova/di.h"

uint32 ReadDevice(uint32 DevID, uint32 Buf, uint32 LBA, uint32 Count);
uint32 WriteDevice(uint32 DevID, uint32 Buf, uint32 LBA, uint32 Count);
#endif

#define Packed_Struct __attribute__((packed))

enum{
	SFS_ERROR_UPDATE_FTABLE		= (1 <<  0),
};

static inline uint32 AttribIsSet(uint8 Att, uint8 Mask){
	return (Att & Mask) == Mask;
};

#define FILE_SEGMENTS 5
#define FILES_PER_SECTOR 8
#define FILE_NAME_LENGTH 18

typedef struct {//w klastrach
	uint32 Position: 32;
	uint32 Size: 24;
}Packed_Struct FilePartInfo_t, *FilePartInfo_p;

typedef struct {
	char Name[FILE_NAME_LENGTH];//nazwa pliku z rozszerzeniem oddzielonym '.'
	uint16 ParentDir;//nr katalogu w³aœciciela; 0 - root
	uint8 Attributes;//atrybuty pliku
	uint32 Date;//data i godzina utworzenia pliku
	uint32 Size;//rozmiar pliku w bajtach
	FilePartInfo_t Segments[FILE_SEGMENTS];
}Packed_Struct FileData_t, *FileData_p;

typedef struct{
	FileData_t File[FILES_PER_SECTOR];
}Packed_Struct FileSector_t, *FileSector_p;

typedef struct {
	uint8 JumpCode[3];
	uint8 fsName[5];
	uint16 BytesPerSector;
	uint8 SectorsPerCluster;
	uint16 SectorsPerTrack;
	uint16 NumberOfHeads;		
	uint32 PartitionID;
	uint32 TotalSectors;
	uint32 FileStructPos;
	uint32 FileStructSize;
	uint8 DriveNumber;
	uint16 Version;	
	uint8 MediaType;
	uint8 Code[475];
	uint16 Signature;
}Packed_Struct PBS_t, *PBS_p;

typedef struct {
	uint8 Header[5];
	uint16 Version;
	char Name[20];
	uint8 ClusterSize; //iloœæ sektorów w klastrze
	uint32 ClusterCount;//iloœæ klastrów w partycji
	uint32 PartitionSize;//wielkosc w bajtach
	uint16 HiddenClusters;//iloœæ ukrytych klastrów ³¹cznie z pbs <?>
	uint32 FileStructPos;//nr pierwszego sektora
	uint16 FileStructSize;//rozmiar w sektorach
	uint8 Puste[466];//pozosta³a wolna przestrzeñ w pbs
	uint16 Signature;//obowi¹zkowa sygnaturka
}Packed_Struct PartitionInfoSector_t, *PartitionInfoSector_p;

typedef struct{
	uint32 start, size;
} BlockData_t, *BlockData_p;

typedef struct{
	BlockData_p Blocks_begin;
	BlockData_p Blocks_end;
	uint32 TotalFreeClusters;
} FreeBlocksManager_t, *FreeBlocksMeneger_p;

typedef struct{
	PartitionInfoSector_t InfoSector;
	uint32 PartitionID;
	uint32 FilesCount;
	FileSector_p SectorTable;
	FileData_p FilesTable;
	
	FreeBlocksManager_t FreeBlocks;

	uint32 ErrorFlags;
	uint32 DevID;
}SFS_Volume_t, *SFS_Volume_p;

typedef struct {
	char *pat;	
	uint32 FileNumber;
	uint16 ParentDir;
	uint16 temp;
} FindHandle_t, *FindHandle_p;

typedef struct{
	uint32 size;
	uint32 pos;
	FileData_p fptr;
	SFS_Volume_p volume;
	uint32 flags;//tryb dostêpu, w jaki sposów utworzony
	void *buffer;
} FileHandle_t, *FileHandle_p;

uint32 SFS_OpenVolume(uint32 DevID, SFS_Volume_p mfs);
bool SFS_VolumeIsReady(SFS_Volume_p vol);

uint32 SFS_FindFirst(SFS_Volume_p vol, char *Pat, SearchRec_p sr, FindHandle_p FindHandle);
uint32 SFS_FindNext(SFS_Volume_p vol, SearchRec_p SR, FindHandle_p FH);

FileData_p FindFile(SFS_Volume_p vol, char *FName, uint16 ParentNumber, uint16 *FNumber, uint32 findflags);
char* ParsePath(SFS_Volume_p vol, char* path, uint16 *ParentNumber);//zwraca pattern z path'a czyli po ostatnim /
FileData_p FindFreeFileRecord(SFS_Volume_p vol, uint16 *FNumber);

uint32 SFS_CreateFile(FileHandle_p fh, char* file, uint32 rights);
uint32 SFS_ReadFile(FileHandle_p fh, uint32 buf, uint32 buf_size, uint32* Read);

uint32 SFS_GetFileSize(SFS_Volume_p vol,  FileHandle_p FH);
uint32 SFS_GetFileDate(SFS_Volume_p vol, FileHandle_p FH);

uint32 SFS_WriteFile(SFS_Volume_p vol, FileHandle_p H, uint32 src_pid, uint32 buf,
			uint32 buf_size, uint32* Written);
			
uint32 SFS_DeleteFile(SFS_Volume_p vol, char* file);

#endif
