#ifndef VOLUMEINTERFACE_H
#define VOLUMEINTERFACE_H

#include <iostream>
#include <stdio.h>
using namespace std;

struct sVolumeGeometry
{
	unsigned Sectors, Heads, SectorsPerTrack, BytesPerSector;
};

enum
{
	Geometry_Fdd144M,

	Geometry_High,
};

const sVolumeGeometry PredefinedGeometry[Geometry_High] = {
	{2880, 2, 18, 512},
};

#define SetErrorAndReturn(A,B) {FLastError = A; return B;}


enum{
    FILE_ACCES_FULL,

    FILE_OPEN		= 0x10,
    FILE_CREATE		= 0x20,
    FILE_ALLWAYS	= 0x01,
    FILE_EXISTING	= 0x02,

    FILE_OPEN_ALLWAYS	= FILE_OPEN | FILE_CREATE,
    FILE_OPEN_EXISTING	= FILE_OPEN,

};

struct GlobalSearchRecord{
	char Name[500];
	unsigned char Attributes;
	unsigned CreationDateTime;
	unsigned Size;
	unsigned FindHandle;
};

#define VIRTUAL     {FLastError = 0xFF0000;return 0;}
#define VIRTUAL_V   {FLastError = 0xFF0000;}

class VolumeInterface;

class cImage
{
	FILE *fptr;
	int FImageSize, FBlockSize, FBlockCount;
	bool Ready;
	char const*FFileName;
public:
	cImage();
	~cImage();

	int GetImageSize();
	bool IsReady();
	char const *GetFileName();

	bool OpenFile(const char* FileName, int BlockSize);
	bool CreateImage(const char* FileName, int BlocksCount, int BlockSize);

	int ReadBlock(int BlockNo, int BlockCount, void *Buffer);
	int WriteBlock(int BlockNo, int BlockCount, void *Buffer);

	VolumeInterface *Volume;
};

class VolumeInterface
{
protected:
	cImage *FImage;
    unsigned FLastError;
public:
    VolumeInterface(cImage *image);
    virtual ~VolumeInterface();
    unsigned LastError(bool remain = false){
    	if(remain)return FLastError;
    	unsigned tmp = FLastError;
    	FLastError = 0;
    	return tmp;
    };

    virtual void List(const char *path);
    virtual void Format(const char* VolName, const sVolumeGeometry *Geometry)VIRTUAL_V;

    virtual void Format(char* BS, char* name, unsigned preset)VIRTUAL_V;

	virtual unsigned CreateFile(const char* Name, unsigned CreateMode)VIRTUAL;
	virtual int WriteFile(unsigned handle, void* Source, unsigned Count, unsigned* Written)VIRTUAL;
    virtual void CloseFile(unsigned handle)VIRTUAL_V;

	virtual unsigned GetFileSize(unsigned handle)VIRTUAL;
	virtual unsigned GetFileDate(unsigned handle)VIRTUAL;

	virtual void DeleteFile(char *FName)VIRTUAL_V;
	virtual void InsertFile(char* vname, char* sysname)VIRTUAL_V;

	virtual int FindFirst(GlobalSearchRecord &sr, const char* path)VIRTUAL;
	virtual int FindNext(GlobalSearchRecord &sr)VIRTUAL;
	virtual int FindClose(GlobalSearchRecord &sr)VIRTUAL;

	virtual char* Name()VIRTUAL;
	virtual unsigned FreeSpace()VIRTUAL;
	virtual unsigned DiskSize()VIRTUAL;
	virtual int CreateDir(char *DName)VIRTUAL;
	virtual int Go(char *DName)VIRTUAL;
	virtual int FindFirst(char *Pattern, GlobalSearchRecord *sr)VIRTUAL;
	virtual int FindNext(GlobalSearchRecord *sr)VIRTUAL;
	virtual int FindClose(GlobalSearchRecord *sr)VIRTUAL;
	virtual int ReadFile(unsigned handle, void* Dest, unsigned Count, unsigned* Read)VIRTUAL;
	virtual int SeekFile(unsigned handle, unsigned Count, unsigned char SeekMethod)VIRTUAL;
};

time_t GetSysFileTime(const char * f_p);

#endif // VOLUMEINTERFACE_H
