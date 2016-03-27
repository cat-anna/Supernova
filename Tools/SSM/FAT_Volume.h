#ifndef FAT_VOLUME_H
#define FAT_VOLUME_H

#include "VolumeInterface.h"

#define _BUILDING_SSM_
#include "../../source/include/sysdef.h"
#include "../../source/drivers/fat/fat.h"

typedef FAT_BS_s fat_BS_t;

enum{
	FAT_FILENAME_LIMIT		= 250,
	FAT_LFN_MAX_ORDER		= FAT_FILENAME_LIMIT / 13,
};

struct FAT_FilePointer
{
	fat_File_Entry_t FatFile;
	unsigned Cluster, EntryNo;
};

struct FAT_FileHandle
{
	FAT_FilePointer Ptr;
	wchar_t LFN[FAT_FILENAME_LIMIT];
	unsigned Position;
	unsigned Flags;

};

class FAT_Volume: public VolumeInterface
{
protected:
	unsigned char *FAT;
	char FBootSector[512];
	fat_BS_t *fat_bs;
	unsigned FatType;

	unsigned ClusterSize;

	bool ParsePath(FAT_FileHandle *Handle, const char* Path, bool DoCreate);
	bool FindFile(const char* Path, FAT_FilePointer *Ptr);
	bool AllocFile(const char* FileName, FAT_FilePointer *dir, FAT_FilePointer *Ptr);

	bool FAT_Load();
	bool FAT_Save();
	unsigned FAT_AllocChain(unsigned ClusterCount);
	unsigned FAT_AllocCluster(unsigned Previous);
	unsigned FAT_FollowCluster(unsigned Cluster);
	void FAT_FreeClusterChain(unsigned First);

	bool FAT_SaveFileEntry(FAT_FilePointer *Ptr);
public:
	FAT_Volume(cImage *image);
    ~FAT_Volume();

//    void DeleteFile(char *FName);
    void InsertFile(char* vname, char* sysname);

    unsigned CreateFile(const char* Name, unsigned CreateMode);
    int WriteFile(unsigned handle, void* Source, unsigned Count, unsigned* Written);
	void CloseFile(unsigned handle);

    void Format(const char* VolName, const sVolumeGeometry *Geometry);

/*	unsigned GetFileSize(unsigned handle);
	unsigned GetFileDate(unsigned handle);*/

	int FindFirst(GlobalSearchRecord &sr, const char* Path);
	int FindNext(GlobalSearchRecord &sr);
	int FindClose(GlobalSearchRecord &sr);
};

#endif
