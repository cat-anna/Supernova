#ifndef SFS_VOLUME_H
#define SFS_VOLUME_H

#include <stdio.h>
#include "VolumeInterface.h"

class SFS_Volume : public VolumeInterface
{
public:
    SFS_Volume(char* fname);
    ~SFS_Volume();

    void DeleteFile(char *FName);
    void InsertFile(char* vname, char* sysname);

    unsigned CreateFile(const char* Name, unsigned CreateMode);
    int WriteFile(unsigned handle, void* Source, unsigned Count, unsigned* Written);
    void CloseFile(unsigned handle);

    void Format(char* BS, char* name, unsigned preset);

	unsigned GetFileSize(unsigned handle);
	unsigned GetFileDate(unsigned handle);

	int FindFirst(GlobalSearchRecord &sr, const char* path);
	int FindNext(GlobalSearchRecord &sr);
	int FindClose(GlobalSearchRecord &sr);
protected:
    void* SFS_p;
    FILE *ImgPtr;
private:
};

unsigned EncodeDate_SFS(unsigned Year, unsigned Month, unsigned Day, unsigned Hour, unsigned Minute);
unsigned GetSysFileTime_SFS(char* f_p);
unsigned GetSystemTime_SFS();

#endif // SFS_VOLUME_H
