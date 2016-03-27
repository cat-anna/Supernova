#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <vector>
#include <string.h>
using namespace std;

#include "Supernova_Volume.h"

unsigned GetSystemTime_SFS() {
	return 0;
	/*	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
    WIN::SYSTEMTIME s;
    WIN::GetSystemTime(&s);
    return EncodeDate_SFS(s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute);*/
};

unsigned EncodeDate_SFS(unsigned Year, unsigned Month, unsigned Day, unsigned Hour, unsigned Minute) {
    return Minute + (Hour << 6)	+ (Day << 11) + (Month << 16) + (Year << 20);
}

unsigned GetSysFileTime_SFS(char* f_p) {
	struct stat buf;
	struct tm * timeinfo;
	if (stat(f_p, &buf))return 0;
	timeinfo = localtime (&buf.st_mtime);
	return EncodeDate_SFS(timeinfo->tm_year, timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_hour, timeinfo->tm_min);

/*, timeinfo->tm_mon + 1, );

	if(options & 0x400)
		fprintf(f_h, "#define COMPILE_TIME_HOUR\t%4d\n"
					 "#define COMPILE_TIME_MINUTE\t%4d\n"
					 "#define COMPILE_TIME_SECOND\t%4d\n"
					 "#define COMPILE_TIME_STRING\t__TIME__\n"
					 "\n",
					 , , timeinfo->tm_sec


		printf("\nLast modified date and time = %s\n", timeStr);
	}
   /* WIN::HANDLE h = WIN::CreateFileA(f_p, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
//	if(h == WIN::INVALID_HANDLE_VALUE) return 0;
    WIN::FILETIME ft;
    WIN::GetFileTime(h, 0, 0, &ft);
    WIN::CloseHandle(h);
    WIN::SYSTEMTIME s;
    WIN::FileTimeToSystemTime(&ft, &s);
    return EncodeDate_SFS(s.wYear, s.wMonth, s.wDay, s.wHour, s.wMinute);*/
	return 0;
};

namespace SFS {
#define _BUILDING_SSM_

//#include "../../source/include/sysdef.h"
//#include "../../source/include/pawlos_msg.h"
//#include "../../source/include/pawlos_files.h"
//#include "../../source/include/pawlos_misc.h"
//#include "../source/include/supernova/di.h"
//#include "../../source/libc/string.c"
#define ZeroMemory(P, S) memset(P,0,S)

uint32 ReadDevice(uint32 DevID, uint32 Buf, uint32 LBA, uint32 Count) {
    FILE *f = reinterpret_cast<FILE*>(DevID);
    fseek(f, LBA * 512, SEEK_SET);
    int r = fread((void*)Buf, 512, Count, f);
//    printf("ReadDevice %d %d %d\n", r, LBA, Count);
    return r;
}

uint32 WriteDevice(uint32 DevID, uint32 Buf, uint32 LBA, uint32 Count) {
    FILE *f = reinterpret_cast<FILE*>(DevID);
    fseek(f, LBA * 512, SEEK_SET);
    int r = fwrite((void*)Buf, 512, Count, f);
 //  printf("WriteDevice %d %d %d\n", r, LBA, Count);
    return r;
}

#define ReadProcessMemory(P, S, D, C) memcpy((void*)(D), (void*)(S), C)
#define WriteProcessMemory(P, S, D, C) memcpy((void*)(S), (void*)(D), C)

#include "../../source/drivers/Supernova File System/SFS.c"

#undef WriteProcessMemory
#undef ReadProcessMemory
}

#define SFS_vol ((SFS::SFS_Volume_p)SFS_p)

unsigned SFS_Volume::GetFileDate(unsigned handle) {
    FLastError = 0;
    return SFS::SFS_GetFileDate(SFS_vol, (SFS::FileHandle_p)handle);
}

unsigned SFS_Volume::GetFileSize(unsigned handle) {
    FLastError = 0;
    return 0;
}

int SFS_Volume::WriteFile(unsigned handle, void* Source, unsigned Count, unsigned* Written) {
    FLastError = SFS::SFS_WriteFile(SFS_vol, (SFS::FileHandle_p)handle, 0, (unsigned)Source, Count, Written);
    return FLastError;
}

void SFS_Volume::InsertFile(char* vname, char* sysname) {
	FILE *in = fopen(sysname, "rb");
    if(in == NULL){
    	FLastError = 0;
    	//printf("SFS: Missing file'%s'", sysname);
    	return;
    }
    fseek(in, 0, SEEK_END);
    unsigned fs = ftell(in);
    void* buf = malloc(fs);
    fseek(in, 0, SEEK_SET);
    memset(buf, 0xFF, fs);
    int r = fread(buf, 1, fs, in);
    if(r != fs){
    	fclose(in);
    	FLastError = 101;
    	return;
    }
    unsigned vh, w=0, err;
    if((vh = CreateFile(vname, SFS::FILE_ACCES_FULL | SFS::FILE_CREATE_NEW)) != 0) {
        err = this->WriteFile(vh, buf, fs, &w);
    }
    CloseFile(vh);

    fclose(in);
    FLastError = !(w == fs);
 //   printf("ERROR:%d %d %d %d\n", err, w, fs, vh);
}

void SFS_Volume::Format(char* BS, char* name, unsigned preset) {

    FILE *bs = fopen(BS, "rb");
    fseek(bs, 0, SEEK_END);
    unsigned bs_s = ftell(bs);
    fseek(bs, 0, SEEK_SET);
    if(bs == NULL || bs_s != 512) {
        fclose(bs);
        FLastError = 50;
        return;
    }
  //  printf("Doing format...\n");
    SFS::PBS_p PBS = new SFS::PBS_t;
    SFS::PartitionInfoSector_p PIS = new SFS::PartitionInfoSector_t;
    memset(PIS, 0, sizeof(SFS::PartitionInfoSector_t));

    fread(PBS, 1, 512, bs);

    PBS->Signature = PIS->Signature = 0xAA55;
    strcpy(PIS->Name, name);

    if(preset == 0) {
        PBS->BytesPerSector = 512;
        PBS->SectorsPerCluster = PIS->ClusterSize = 1;
        PBS->SectorsPerTrack = 18;
        PBS->NumberOfHeads = 2;
        PBS->PartitionID = time(0);
        PBS->TotalSectors = 2880;
        PBS->FileStructPos = PIS->FileStructPos = 2;
        PBS->FileStructSize = PIS->FileStructSize = 3;
        PBS->DriveNumber = 0;
        PBS->Version = 0;
        PBS->MediaType = 0x0F;
        PIS->ClusterCount = 2880;
        PIS->PartitionSize = 2880 * 512;
        PIS->HiddenClusters = 100;
    }

    SFS::WriteDevice((unsigned)ImgPtr, (unsigned)PBS, 0, 1);
    SFS::WriteDevice((unsigned)ImgPtr, (unsigned)PIS, 1, 1);
    unsigned sc = 2;
    ZeroMemory(PBS, 512);
    while (sc < PIS->ClusterCount) {
        SFS::WriteDevice((unsigned)ImgPtr, (unsigned)PBS, sc, 1);
        sc++;
    }

    delete PBS;
    delete PIS;
    fclose(bs);
    ZeroMemory(SFS_p, sizeof(SFS::SFS_Volume_t));
    FLastError = SFS::SFS_OpenVolume((unsigned)ImgPtr, SFS_vol);
}

unsigned SFS_Volume::CreateFile(const char* Name, unsigned CreateMode) {
    SFS::FileHandle_p fh = (SFS::FileHandle_p)malloc(sizeof(SFS::FileHandle_t));
    ZeroMemory(fh, sizeof(SFS::FileHandle_t));
	fh->volume = SFS_vol;
    FLastError = SFS::SFS_CreateFile(fh, (char*)Name, CreateMode);
    if(FLastError != 0) {
//        printf("CreateFile Error! (%d)\n", FLastError);
        free(fh);
        return 0;
    }
    return (unsigned)fh;
}

void SFS_Volume::CloseFile(unsigned handle) {
    free((SFS::FileHandle_p)handle);
}

void SFS_Volume::DeleteFile(char *FName) {
    SFS::SFS_DeleteFile(SFS_vol, FName);
}

SFS_Volume::SFS_Volume(char* fname) : VolumeInterface(0){
	ImgPtr = fopen(fname, "r+b");
	if(ImgPtr == NULL)
		ImgPtr = freopen(fname, "r+b", fopen(fname, "wb"));
    if(ImgPtr == NULL) {
        FLastError = 10;
    }
//	printf("%s|%d|%d\n", fname,(unsigned)ImgPtr, FLastError);
	if(FLastError)return;

    SFS_p = malloc(sizeof(SFS::SFS_Volume_t));
    ZeroMemory(SFS_p, sizeof(SFS::SFS_Volume_t));
    FLastError = SFS::SFS_OpenVolume((unsigned)ImgPtr, SFS_vol);
  // 	printf("%s|%d|%d\n", fname,(unsigned)ImgPtr, FLastError);
}

SFS_Volume::~SFS_Volume() {
    fclose(ImgPtr);
    free(SFS_p);
}


int SFS_Volume::FindFirst(GlobalSearchRecord &sr, const char* path){
    SFS::FindHandle_p fh = new SFS::FindHandle_t;
    SFS::SearchRec_t sfs_sr = {0};

    ZeroMemory(fh, sizeof(SFS::FindHandle_t));

    FLastError = SFS::SFS_FindFirst(SFS_vol, const_cast<char*>(path), &sfs_sr, fh);
    if(FLastError != 0) {
        free(fh);
        return FLastError;
    }
    sr.FindHandle = (unsigned)fh;

    sr.Attributes = sfs_sr.flags;
    sr.CreationDateTime = sfs_sr.date;
    sr.Size = sfs_sr.size;
    strcpy(sr.Name, sfs_sr.name);

    return 0;
}

int SFS_Volume::FindNext(GlobalSearchRecord &sr){
    SFS::SearchRec_t sfs_sr = {0};
    SFS::FindHandle_p fh = (SFS::FindHandle_p)sr.FindHandle;

    FLastError = SFS::SFS_FindNext(SFS_vol, &sfs_sr, fh);
    if(FLastError != 0) {
        free(fh);
        return FLastError;
    }
    sr.Attributes = sfs_sr.flags;
    sr.CreationDateTime = sfs_sr.date;
    sr.Size = sfs_sr.size;
    memcpy(sr.Name, sfs_sr.name, 18);

    return 0;
}

int SFS_Volume::FindClose(GlobalSearchRecord &sr){
    SFS::FindHandle_p fh = (SFS::FindHandle_p)sr.FindHandle;
    sr.FindHandle = 0;
   // delete fh;  //dont know why, but this line generates fatal error
    FLastError = 0;
    return 0;
}
