#include "FAT_Volume.h"
#include <string.h>
#include <time.h>
#include <iostream>
#include <fstream>
//using namespace std;
/*
uint32 DirectRead(HANDLE handle, void* buffer, uint32 count, uint32 *read){
	return
}*/

#include "../../source/include/Supernova/ErrorCodes.h"
#include "../../source/include/Supernova/DrvCommands.h"
#include "../../source/drivers/fat/fat.c"

const char FAT12_BootSector[] = "/home/Admin/Supernova/binary/SNLoader/bootsector_FAT.bin";

//this function assume that given pointer is LFN
int FAT_ParseLFNentry(wchar_t *LFN, void *LFN_entry)
{
	fat_LFN_Entry_t *entry = (fat_LFN_Entry_t*) LFN_entry;

	int which = entry->order & 0x3F;
	/*
	 - Bit 6 is set for the last LFN record in the sequence.
	 - Bit 7 is set if the LFN record is an erased long name entry or maybe if it is part of an erased long name?
	 */
	if (which > FAT_LFN_MAX_ORDER) return false;
	--which;
	wchar_t *name = LFN + which * 13;
	for (int i = 0; i < 5; ++i) *name++ = entry->first[i];
	for (int i = 0; i < 6; ++i) *name++ = entry->middle[i];
	for (int i = 0; i < 2; ++i) *name++ = entry->last[i];
	return true;
}

bool FAT_SetLFNEntry(wchar_t *LFN, void *LFN_entry, int which, bool Last, unsigned char checksum)
{
	fat_LFN_Entry_t *entry = (fat_LFN_Entry_t*) LFN_entry;
	if (which > FAT_LFN_MAX_ORDER) return false;
	--which;
	wchar_t *name = LFN + which * 13;

	for (int i = 0; i < 5; ++i) entry->first[i] = *name++;
	for (int i = 0; i < 6; ++i) entry->middle[i] = *name++;
	for (int i = 0; i < 2; ++i) entry->last[i] = *name++;

	entry->attribute = FAT_ATTRIBUTE_LONG_FILE_NAME;
	entry->order = which + 1;
	if(Last)entry->order |= 0x40;
	entry->charzero = 0;
	entry->checksum = checksum;
	entry->shortzero = 0;

	return true;
}

bool FAT_EntrySetName(const char *name, fat_File_Entry_t *entry)
{
	for (int j = 0; j < 8;){
		char c = *name;
		if(c && c != '.')name++;

		if (!c || c == '.') c = 0x20;
		else if(c == 0x20) continue;

		if(c != 0x20) c &= 0xDF;
		entry->Name[j] = c;
		++j;
	}
	if(*name && *name != '.'){
		entry->Name[6] = '~';
		entry->Name[7] = '1';
		while(*name != '.')name++;
	}
	if(*name)name++;
	for (int j = 0; j < 3; ++j){
		char c = *name;
		if (!c) c = 0x20;
		else name++;
		if(c != 0x20) c &= 0xDF;
		entry->Ext[j] = c;
	}

	return true;
}

unsigned char FAT_EntryGetCheckSum(fat_File_Entry_t *entry)
{
	int i;
	unsigned char sum = 0;
	char *nameptr = reinterpret_cast<char*>(entry);

	for (i = 11; i; i--)
		sum = ((sum & 1) << 7) + (sum >> 1) + *nameptr++;

	return sum;
}

bool FAT_Volume::ParsePath(FAT_FileHandle *Handle, const char* Path, bool DoCreate)
{
	return false;
}

void wchart2char(char* dest, const wchar_t *src)
{
	if (!dest || !src) return;
	while (*src && *src != 0xFFFF) {
		char t = static_cast<char>(*src);
		src++;
		*dest = t;
		dest++;
	}
	*dest = 0;
}

void char2wchart(wchar_t* dest, const char *src)
{
	if (!dest || !src) return;
	do {
		wchar_t t = static_cast<wchar_t>(*src);
		src++;
		*dest = t;
		dest++;
	} while (*src);
	*dest++ = 0;
	*dest = 0xFFFF;
}

struct FAT_FindHandle
{
	unsigned IsInRoot;
	unsigned Cluster;
	unsigned ClusterCount;
	unsigned EntryNo;
	char* pattern;
	fat_File_Entry_t Table[16];
};

int FAT_Volume::FindFirst(GlobalSearchRecord &sr, const char* Path)
{
	FAT_FindHandle *fh = new FAT_FindHandle;
	sr.FindHandle = (unsigned) fh;
	fh->IsInRoot = 1;
	fh->EntryNo = 16;
	fh->Cluster = 1 + fat_bs->table_size_16 * fat_bs->table_count;
	fh->ClusterCount = fat_bs->root_entry_count >> 4;

	fh->pattern = new char[250];
	strcpy(fh->pattern, Path);

	char* p = fh->pattern;
	if (*p == '/') p++;

	//	cout << "cc " << fh->ClusterCount << " " << fh->Cluster << endl;
	fh->pattern = p;

	FLastError = 0;
	return 0;
}

int FAT_Volume::FindNext(GlobalSearchRecord &sr)
{
	FAT_FindHandle *fh = reinterpret_cast<FAT_FindHandle*>(sr.FindHandle);

	FLastError = 0;
	char *p = fh->pattern;

	wchar_t LFNBuffer[256];
	bool hasLFN = false;
	memset(LFNBuffer, 0, 256 * sizeof(short));
	do {
		if (fh->EntryNo >= 15) {
			FImage->ReadBlock(fh->Cluster, 1, fh->Table);
			++fh->Cluster;
			--fh->ClusterCount;
			fh->EntryNo = 0;
		}
		for (int i = fh->EntryNo; i < 16; ++i) {
			//	cout << i<< '|';
			fat_File_Entry_t *Entry = &fh->Table[i];
			if (Entry->Name[0] == 0 || Entry->Name[0] == 0xE5) //empty or deleted entry
				continue;

			if (Entry->Attributes == 0x08) continue;

			if (Entry->Attributes == 0x0F) {
				if (Entry->Name[0] & 0x80) continue; //deleted lfn
				FAT_ParseLFNentry(LFNBuffer, Entry);
				hasLFN = true;
				continue;
			}

			fh->EntryNo = i + 1;
			sr.Attributes = Entry->Attributes;
			sr.Size = Entry->FileSize;

			char Name[15] = { 0 };
			FAT_EntryGetName(Name, Entry);

			if (hasLFN) {
				wchart2char(sr.Name, LFNBuffer);
			} else {
				strcpy(sr.Name, Name);
			}

			return 0;
		}
		fh->EntryNo = 16;
	} while (fh->ClusterCount > 0);
	FLastError = 1;
	return 1;
}

int FAT_Volume::FindClose(GlobalSearchRecord &sr)
{
	FLastError = 1;
	return 1;
}
/*
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
 */

bool FAT_Volume::FindFile(const char* Path, FAT_FilePointer *Ptr)
{
	if (!Path || !*Path){
		Ptr->FatFile.Attributes = FAT_ATTRIBUTE_DIRECTORY;
		Ptr->FatFile.LowCluster = 1 + fat_bs->table_size_16 * fat_bs->table_count;
		return true;
	}
	char path[250];
	strcpy(path, Path);
	for(char *u = path; *u; ++u)
		if(*u >= 'a' && *u <= 'z') *u &= 0xDF;
	char* p = path;

	if (*p == '/') p++;

	wchar_t LFNBuffer[256];
	memset(LFNBuffer, 0xFF, 256*2);
	bool hasLFN = false;

	unsigned Cluster = 1 + fat_bs->table_size_16 * fat_bs->table_count;
	unsigned ClusterCount = fat_bs->root_entry_count >> 4;

	fat_File_Entry_t Table[16];

	char *tok = strtok(p, "/");
	while (tok) {
		char *next = strtok(NULL, "/");
		do {
			FImage->ReadBlock(Cluster, 1, Table);
			++Cluster;
			--ClusterCount;
			for (int i = 0; i < 16; ++i) {
				fat_File_Entry_t *Entry = &Table[i];
				if (Entry->Name[0] == 0 || Entry->Name[0] == 0xE5) //empty or deleted entry
					continue;
				if (Entry->Attributes == FAT_ATTRIBUTE_LONG_FILE_NAME) {
					if (Entry->Name[0] & 0x80) continue; //deleted lfn
					FAT_ParseLFNentry(LFNBuffer, Entry);
					hasLFN = true;
					continue;
				}
				if (Entry->Attributes & FAT_ATTRIBUTE_VOLUME_ID) continue; //volume entry

				char Name[250];
				memset(Name, 0, 250);
				if (hasLFN) {
					wchart2char(Name, LFNBuffer);
					memset(LFNBuffer, 0xFF, 256*2);
				} else {
					FAT_EntryGetName(Name, Entry);
				}
				hasLFN = false;
	//			printf("%s|%s\n", Name, tok);
				if (strcasecmp(Name, tok)) continue;
			//	cout << "Found!\n";
				if (next) { //get into directory
					Cluster = 0;
					break;
				} else { //file is already found
					memcpy(&Ptr->FatFile, Entry, sizeof(fat_File_Entry_t));
					Ptr->Cluster = Cluster-1;
					Ptr->EntryNo = i;
//					printf("File Found: %d %d\n", Cluster, i);
					FLastError = 0;
					return true;
				}
			}
		} while (ClusterCount > 0);

		tok = next;
	};

	return false;
}

bool FAT_Volume::AllocFile(const char* FileName, FAT_FilePointer *dir, FAT_FilePointer *Ptr)
{
	int namelen = strlen(FileName);
	int EntryCount = 1;
	int LFNEntryCount = 0;
	if(namelen > 12){
		LFNEntryCount = namelen / 13;
		if(namelen % 13)
				namelen++;
		EntryCount += LFNEntryCount;
	}

	if(EntryCount > 19)SetErrorAndReturn(50,false);

	printf("AllocFile ec:%d lfn:%d\n", EntryCount, LFNEntryCount);

	fat_File_Entry_t Table[32];

	if(!(dir->FatFile.Attributes & FAT_ATTRIBUTE_DIRECTORY))
		SetErrorAndReturn(50, false);
	unsigned Cluster = dir->FatFile.LowCluster;

	unsigned Begin = 0;
	unsigned BeginCluster = 0;

	do{
		FImage->ReadBlock(Cluster, 1, Table);

		for(int i = 0; i < 16; ++i){
			if (Table[i].Name[0] != 0) //not empty
				continue;

			Begin = i;
			BeginCluster = Cluster;
			int end = i + EntryCount;
			int remain = EntryCount;
			while(i < end && i < 32){
				if(i == 15){ //last entry was empty, need to load next sector
					Cluster++;//GetNextCluster;
					FImage->ReadBlock(Cluster, 1, Table + 16);
				}
				++i;
				--remain;
				if(Table[i].Name[0] == 0) continue;

				//if we are here that means there weren't enough space
				if(i > 15){
					memcpy(Table, Table + 16, 512);
					i -= 16;
				}
				break;
			}
			if(remain == 0){
				Cluster = 0;
				break;
			} else {
				BeginCluster = 0;
			}
		}
		if(Cluster)Cluster++;
	}while(Cluster > 0);

	if(BeginCluster == 0)SetErrorAndReturn(30,false);

//set file entry
	fat_File_Entry_t *Entry = &Table[Begin + EntryCount - 1];

	memset(Entry, 0, sizeof(fat_File_Entry_t));

	FAT_EntrySetName(FileName, Entry);
	unsigned char chk = FAT_EntryGetCheckSum(Entry);

	if(LFNEntryCount){
		wchar_t LFNBuf[256];
		memset(LFNBuf, 0xFF, 256*2);
		char2wchart(LFNBuf, FileName);
		for(int i = 1; i <= LFNEntryCount; ++i)
			FAT_SetLFNEntry(LFNBuf, &Table[Begin + LFNEntryCount - i], i, i == LFNEntryCount, chk);
	}

	/*unsigned firstcluster = FAT_AllocChain(1);
	cout << firstcluster<<endl;
	Entry->LowCluster = static_cast<short>(firstcluster);
	FAT_Save();*/

	struct tm * timeinfo;
	time_t curr = time(NULL);
	timeinfo = localtime(&curr);

	Entry->CreationTime = static_cast<unsigned short>(FAT_MakeTime(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec));
	Entry->CreationDate = static_cast<unsigned short>(FAT_MakeDate(timeinfo->tm_year, timeinfo->tm_mon + 1, timeinfo->tm_mday));

	Entry->LastModificationDate = Entry->CreationDate;
	Entry->LastModificationTime = Entry->CreationTime;

//	printf("AllocFile2: %d %d\n", BeginCluster, Begin);
	FImage->WriteBlock(BeginCluster, 1, Table);

	if(Begin > 15)
		FImage->WriteBlock(BeginCluster + 1, 1, Table + 16);
//set return pointer

	Ptr->Cluster = BeginCluster;
	Ptr->EntryNo = Begin + EntryCount - 1;
	memcpy(&Ptr->FatFile, Entry, sizeof(fat_File_Entry_t));

	return true;
}

unsigned FAT_Volume::CreateFile(const char* Name, unsigned CreateMode)
{
	FLastError = 1;
	char Copy[250];
	strcpy(Copy, Name);

	char *path = Copy;
	char *filename = strrchr(Copy, '/');

	FAT_FilePointer FPtr = { 0 };
	bool found = 0;

	if (FindFile(path, &FPtr)) {
		if(CreateMode & FILE_CREATE){
			if(FPtr.FatFile.Name[0] != 0 && FPtr.FatFile.Name[0] != 0xE5){
				FAT_Load();
				FAT_FreeClusterChain(FPtr.FatFile.LowCluster);
				FAT_Save();
			}
			FPtr.FatFile.LowCluster = 0;
			FPtr.FatFile.FileSize = 0;
			FPtr.FatFile.LastAccessedDate = 0;
			struct tm * timeinfo;
			time_t curr = time(NULL);
			timeinfo = localtime(&curr);
			FPtr.FatFile.CreationTime = static_cast<unsigned short>(FAT_MakeTime(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec));
			FPtr.FatFile.CreationDate = static_cast<unsigned short>(FAT_MakeDate(timeinfo->tm_year, timeinfo->tm_mon + 1, timeinfo->tm_mday));

			FPtr.FatFile.LastModificationDate = FPtr.FatFile.CreationDate;
			FPtr.FatFile.LastModificationTime = FPtr.FatFile.CreationTime;
		}
//		std::cout <<"by open\n";
		found = true;
	}
	if (!found && (CreateMode & FILE_CREATE)) {
		if (!filename) {
			filename = Copy;
			path = 0;
		} else {
			*filename = 0;
			filename++;
		}
		FAT_FilePointer FParent = { 0 };

		if (!FindFile(path, &FParent))SetErrorAndReturn(8,0);
		if (!AllocFile(filename, &FParent, &FPtr))SetErrorAndReturn(9,0);
		found = true;
	}
	if (!found) SetErrorAndReturn(10,0);

	FAT_FileHandle *fh = new FAT_FileHandle;
	memcpy(&fh->Ptr, &FPtr, sizeof(FAT_FilePointer));
	fh->Position = 0;
	fh->Flags = CreateMode;
	FLastError = 0;
	return reinterpret_cast<unsigned>(fh);
}

int FAT_Volume::WriteFile(unsigned handle, void* Source, unsigned Count, unsigned* Written)
{
	if(!handle || !Source || !Count || !Written)SetErrorAndReturn(10,10);

	FAT_FileHandle *fh = reinterpret_cast<FAT_FileHandle*>(handle);

	unsigned filebegin = fh->Position;
	unsigned fileend = filebegin + Count;

	unsigned FirstCluster = filebegin / ClusterSize;
	unsigned LastCluster = fileend / ClusterSize;

	unsigned ClusterCount = LastCluster - FirstCluster + 1;

	unsigned ClusterOffset = filebegin % ClusterSize;

	unsigned FollowCluster = fh->Ptr.FatFile.LowCluster;

	unsigned DataAreaOffset = 1 + fat_bs->table_size_16 * fat_bs->table_count +
				 (fat_bs->root_entry_count >> 4) - 2;

	FAT_Load();

	if(FollowCluster == 0){
		FollowCluster = FAT_AllocCluster(0x0);
		fh->Ptr.FatFile.LowCluster = static_cast<short>(FollowCluster);
//		printf("FAT_AllocCluster %d\n", FirstCluster);
	}

	for(int i = 0; i < FirstCluster; ++i){
		printf("for FirstCluster %d\n", FirstCluster);
		FollowCluster = FAT_FollowCluster(FollowCluster);
	}

	char *Buffer = new char[ClusterSize];
	if(ClusterOffset){
		FImage->ReadBlock(DataAreaOffset + FollowCluster, 1, Buffer);
	}

	unsigned TotalSize = Count;
	while(Count){
		unsigned Size2Write;
		Size2Write = (Count < ClusterSize)?(Count):(ClusterSize - ClusterOffset);
		memset(Buffer + ClusterOffset + Size2Write, 0, ClusterSize - Size2Write - ClusterOffset);
		memcpy(Buffer + ClusterOffset, Source, Size2Write);

		FImage->WriteBlock(DataAreaOffset + FollowCluster, 1, Buffer);
	//	printf("WRITE: c:%d 2w:%d of:%d %d+%d=%d\n", Count, Size2Write, ClusterOffset, FollowCluster, DataAreaOffset, FollowCluster+DataAreaOffset);

		ClusterOffset = 0;
		Count -= Size2Write;
		Source = ((char*)Source) + Size2Write;

		if(Count > 0){
			unsigned tmp = FAT_FollowCluster(FollowCluster);
//			printf("FollowCluster %x %x\n", FollowCluster, tmp);
			if(tmp == 0x0FF8){
				FollowCluster = FAT_AllocCluster(FollowCluster);
			} else
				FollowCluster = tmp;
		}
	}

	FAT_Save();

	fh->Ptr.FatFile.FileSize = fileend;
	FAT_SaveFileEntry(&fh->Ptr);
	fh->Position += TotalSize;

	delete Buffer;

	SetErrorAndReturn(0, TotalSize);
}

void FAT_Volume::CloseFile(unsigned handle)
{
	FLastError = 0;
	if(!handle)	return;
	FAT_FileHandle *fh = reinterpret_cast<FAT_FileHandle*>(handle);
	delete fh;
}

void FAT_Volume::InsertFile(char* vname, char* sysname)
{
	FILE *in = fopen(sysname, "rb");
    if(in == NULL){
    	FLastError = 1;
//    	cout << "insert error\n";
    	return;
    }
    fseek(in, 0, SEEK_END);
    unsigned fs = ftell(in);
    char* buf = new char[fs+1];
    fseek(in, 0, SEEK_SET);
    memset(buf, 0xFF, fs);
    int r = fread(buf, 1, fs, in);
    if(r != fs){
    	fclose(in);
    	FLastError = 101;
    	return;
    }
    unsigned vh, w=0, err;
    if((vh = CreateFile(vname, FILE_CREATE)) != 0) {
        err = this->WriteFile(vh, buf, fs, &w);
    }
    CloseFile(vh);

    fclose(in);
    delete buf;
    FLastError = !(w == fs);
}

bool FAT_Volume::FAT_SaveFileEntry(FAT_FilePointer *Ptr)
{
	char *buffer = new char[ClusterSize];
	FImage->ReadBlock(Ptr->Cluster, 1, buffer);

	fat_File_Entry *Table = reinterpret_cast<fat_File_Entry*>(buffer);

	memcpy(Table + Ptr->EntryNo, &Ptr->FatFile, sizeof(fat_File_Entry));

//	printf("FAT_SaveFileEntry %d %d\n", Ptr->Cluster,Ptr->EntryNo );

	FImage->WriteBlock(Ptr->Cluster, 1, buffer);

	delete buffer;
}

FAT_Volume::FAT_Volume(cImage *image)
		: VolumeInterface(image)
{
	FAT = 0;
	if (!FImage->IsReady()
			|| GetSysFileTime(FAT12_BootSector) > GetSysFileTime(FImage->GetFileName())) {
		FLastError = 1;
		return;
	}

	FImage->ReadBlock(0, 1, FBootSector);
	fat_bs = reinterpret_cast<fat_BS_t*>(FBootSector);

	if(DetectFAT(FBootSector) != SUCCES){
		FLastError = 1;
		return;
	}

	ClusterSize = fat_bs->sectors_per_cluster * fat_bs->bytes_per_sector;

	FLastError = 0;
}


void FAT_Volume::FAT_FreeClusterChain(unsigned First){
	if(!FAT && !FAT_Load())return;
	if(!First || First >= 0x0FF8)return;

	do{
		unsigned tmp = FAT12_Get(FAT, First);
		FAT12_Set(FAT, First, 0);
		First = tmp;
	}while(First > 2 && First < 0x0FF8);
}

unsigned FAT_Volume::FAT_AllocChain(unsigned ClusterCount)
{
	if(!FAT && !FAT_Load())return false;

	unsigned short FistCluster = 0;
	unsigned short PrvCluster = 0;
	unsigned short i = 3;
	while(ClusterCount > 0){
		unsigned short cluster = FAT12_Get(FAT, i);

		if(cluster > 0){
			++i;
			continue;
		}
		printf("FAT_AllocChain %d\n", i);
		if(FistCluster == 0){
			FistCluster = i;
		} else {
			FAT12_Set(FAT, i, PrvCluster);
		}

		PrvCluster = i;
		++i;
		--ClusterCount;
	}
	FAT12_Set(FAT, PrvCluster, 0x0FF8);

	return FistCluster;
}

unsigned FAT_Volume::FAT_AllocCluster(unsigned Previous)
{
	if(!FAT && !FAT_Load())return false;

	unsigned i = 2;
	while(1){
		unsigned g = FAT12_Get(FAT, i);
	//	printf("FAT_AllocCluster: %d->%d\n", i, g);
		if(g == 0)break;
		++i;
	}
//	printf("FAT_AllocCluster %d\n", i);

	if(Previous)
		FAT12_Set(FAT, Previous, i);
	FAT12_Set(FAT, i, 0x0FF8);
	return i;
}

unsigned FAT_Volume::FAT_FollowCluster(unsigned Cluster)
{
	if(!FAT && !FAT_Load())return false;
	return FAT12_Get(FAT, Cluster);
}

bool FAT_Volume::FAT_Load()
{
	if(FAT)return true;

	FAT = new unsigned char[fat_bs->table_size_16 * 512];
	FImage->ReadBlock(1, fat_bs->table_size_16, FAT);
	return true;
}

bool FAT_Volume::FAT_Save()
{
	if(!FAT)return false;
	unsigned sector = 1;
	for (int i = 0; i < fat_bs->table_count; ++i) {
		FImage->WriteBlock(sector, fat_bs->table_size_16, FAT);
		sector += fat_bs->table_size_16;
	}
//	delete FAT;
//	FAT = 0;
	return true;
}

FAT_Volume::~FAT_Volume()
{
	FAT_Save();
	delete FAT;
	FAT = 0;
}

void FAT_Volume::Format(const char* VolName, const sVolumeGeometry *Geometry)
{
	std::ifstream bootsect(FAT12_BootSector);
	bootsect.seekg(0, ios_base::end);
	unsigned bs_s = bootsect.tellg();
	bootsect.seekg(0, ios_base::beg);

	if (!bootsect.is_open() || bs_s != 512) {
		FLastError = 50;
		std::cout <<"Not Opened\n";
		return;
	}

	bootsect.read(FBootSector, 512);
	bootsect.close();
	fat_bs = reinterpret_cast<fat_BS_t*>(FBootSector);

	strncpy(reinterpret_cast<char*>(fat_bs->oem_name), "SSM     ", 8);

	fat_bs->bytes_per_sector = Geometry->BytesPerSector;
	fat_bs->sectors_per_cluster = 1;
	fat_bs->reserved_sector_count = 1;
	fat_bs->table_count = 2;
	fat_bs->root_entry_count = 224;
	fat_bs->total_sectors_16 = Geometry->Sectors;
	fat_bs->media_type = 0xF0;
	fat_bs->table_size_16 = 9;
	fat_bs->sectors_per_track = Geometry->SectorsPerTrack;
	fat_bs->head_side_count = Geometry->Heads;
	fat_bs->hidden_sector_count = 0;
	fat_bs->total_sectors_32 = 0;

	char* FAT = new char[fat_bs->table_size_16 * 512];
	memset(FAT, 0, fat_bs->table_size_16 * 512);

	if (Geometry->Sectors < 4085) { //FAT12
		fat_bs->EBPB.fat16.volume_id = time(NULL);
		memset(fat_bs->EBPB.fat16.volume_label, 0x20, 11);
		int name_len = strlen(VolName);
		memcpy(fat_bs->EBPB.fat16.volume_label, VolName,
				((name_len < 11) ? (name_len) : (11)));
		strncpy(reinterpret_cast<char*>(fat_bs->EBPB.fat16.fat_type_label), "FAT12   ",
				8);
		FAT[0] = 0xF0;
		FAT[1] = 0xFF;
		FAT[2] = 0xFF;
	}

	FImage->CreateImage(0, Geometry->Sectors, Geometry->BytesPerSector);
	unsigned sector = 0;
	FImage->WriteBlock(sector++, 1, FBootSector);
	for (int i = 0; i < fat_bs->table_count; ++i) {
		  	FImage->WriteBlock(sector, fat_bs->table_size_16, FAT);
		sector += fat_bs->table_size_16;
	}

	memset(FAT, 0, Geometry->BytesPerSector);
	struct tm * timeinfo;
	time_t curr = time(NULL);
	timeinfo = localtime(&curr);

	//bool FAT_EntrySetName(const char *name, fat_File_Entry_t *entry)
	fat_File_Entry_t *N = (fat_File_Entry_t*)FAT;
	FAT_EntrySetName(VolName, N);
	N->Attributes = FAT_ATTRIBUTE_VOLUME_ID;

	N->CreationTime = static_cast<unsigned short>(FAT_MakeTime(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec));
	N->CreationDate = static_cast<unsigned short>(FAT_MakeDate(timeinfo->tm_year, timeinfo->tm_mon + 1, timeinfo->tm_mday));

	N->LastModificationDate = N->CreationDate;
	N->LastModificationTime = N->CreationTime;

	FImage->WriteBlock(sector++, 1, FAT);

	memset(FAT, 0, Geometry->BytesPerSector);
	while (sector < Geometry->Sectors) {
		   	FImage->WriteBlock(sector, 1, FAT);
		   	sector++;
	}



	delete FAT;
	ClusterSize = fat_bs->sectors_per_cluster * fat_bs->bytes_per_sector;
	FLastError = 0;
}
