#include "FAT.h"

#ifndef _BUILDING_SSM_
#include "Supernova/di.h"
#include <string.h>
#endif

#ifndef _BUILDING_SSM_

uint32 FindFile(FAT_p FAT, const char* Path, FAT_FilePointer_p Ptr){
	if (!Path || !*Path){
		Ptr->FatFile.Attributes = FAT_ATTRIBUTE_DIRECTORY;
		Ptr->FatFile.LowCluster = FAT->RootDirectory;
		return 1;
	}

	char path[250];

	strcpy(path, Path);
	char *u;
	for(u = path; *u; ++u)
		if(*u >= 'a' && *u <= 'z') *u &= 0xDF;
	char* p = path;

	if (*p == '/') p++;

	wchar_t LFNBuffer[256];
	memset(LFNBuffer, 0xFF, 256 * sizeof(wchar_t));
	bool hasLFN = false;

	fat_File_Entry_t *Root = FAT->Root;
	uint32 EntryCount = FAT->RootEntryCount;
	uint32 EntryNo = 0;

	char *tok = strtok(p, "/");
	while (tok) {
		char *next = strtok(NULL, "/");
		for(; EntryNo < EntryCount; ++EntryNo) {
			fat_File_Entry_t *Entry = Root + EntryNo;
			if (Entry->Name[0] == 0 || Entry->Name[0] == 0xE5) //empty or deleted entry
					continue;
			if (Entry->Attributes == FAT_ATTRIBUTE_LONG_FILE_NAME) {
				if (Entry->Name[0] & 0x80) continue; //deleted lfn
			//	FAT_ParseLFNentry(LFNBuffer, Entry);
			//	hasLFN = true;
				continue;
			}
			if (Entry->Attributes & FAT_ATTRIBUTE_VOLUME_ID) continue; //volume entry

			char Name[250];
			memset(Name, 0, 250);
			if (hasLFN) {
			//	wchart2char(Name, LFNBuffer);
			//	memset(LFNBuffer, 0xFF, 256*2);
			} else {
				FAT_EntryGetName(Name, Entry);
			}
			hasLFN = false;
//			kprintf("%s|%s\n", Name, tok);
			if (strcasecmp(Name, tok)) continue;
//			kprintf("Found!\n");
			if (next) { //get into directory
				tok = 0;
				return 1;//not supported!
				//TODO: Support directories
				break;
			} else { //file is already found
				memcpy(&Ptr->FatFile, Entry, sizeof(fat_File_Entry_t));
				uint32 EntPerCl = FAT->ClusterSize / sizeof(fat_File_Entry_t);
				Ptr->Cluster = EntryNo / EntPerCl;
				Ptr->EntryNo = EntryNo % EntPerCl;
//				kprintf("FILE Found ptr: %d %d\n", Ptr->Cluster, Ptr->EntryNo);
				return SUCCES;
			}
		};
		tok = next;
	};

	return 1;
}

#endif

unsigned short FAT12_Get(unsigned char *FAT, unsigned short no){
	unsigned fat_offset = no + (no / 2);// multiply by 1.5
	unsigned short fat_val = *(unsigned short*)(FAT + fat_offset);
	if(no & 0x0001)
		fat_val = fat_val >> 4;
	else
		fat_val = fat_val & 0x0FFF;
	return fat_val;
}

void FAT12_Set(unsigned char *FAT, unsigned short no, unsigned short value){
	unsigned fat_offset = no + (no / 2);// multiply by 1.5
	unsigned short fat_val = *(unsigned short*)(FAT + fat_offset);
	value &= 0xFFF;
	if(no & 0x0001){
		fat_val &= 0x000F;
		fat_val |= (value << 4);
	} else {
		fat_val &= 0xF000;
		fat_val |= value;
	}
	*(unsigned short*)(FAT + fat_offset) = fat_val;
}

uint32 FAT_EntryGetName(char *name, fat_File_Entry_t *entry){
	int j;
	for (j = 0; j < 8; ++j)
		if (entry->Name[j] != 0x20) *name++ = entry->Name[j];
		else break;

	*name++ = '.';

	for (j = 0; j < 3; ++j)
		if (entry->Ext[j] != 0x20) *name++ = entry->Ext[j];
		else break;

	*name = 0;
	return SUCCES;
}

uint32 InitFAT(FAT_BS_t *fat_bs, FAT_t *fat){
	if(!fat || !fat_bs)return ERRORCODE_WRONG_INPUT;
//calculate needed values
	memset(fat, 0, sizeof(FAT_t));
	fat->BytesPerSector = fat_bs->bytes_per_sector;
	fat->ClusterSize = fat_bs->sectors_per_cluster * fat_bs->bytes_per_sector;
	fat->RootDirectory = 1 + fat_bs->table_count * fat_bs->table_size_16;
	fat->DataStart = fat->RootDirectory + (fat_bs->root_entry_count >> 4);
	fat->RootEntryCount = fat_bs->root_entry_count;
	fat->FatCount = fat_bs->table_count;
	fat->FatSize = fat_bs->table_size_16;
//chceck fat type
	if (fat_bs->total_sectors_16 < 4085) fat->FatType = 12;
	else {
		if (fat_bs->total_sectors_16 < 65525) fat->FatType = 16;
		else fat->FatType = 32;
	}
//some fat type depend initialization
	if(fat->FatType < 17){
	//get volumeID
		fat->VolumeID = fat_bs->EBPB.fat16.volume_id;
	//copy volume name
		char *i = fat_bs->EBPB.fat16.volume_label;
		char *j = fat->Name;
		while(*i && *i != ' ') *j++ = *i++;
	} else
		return EC_FILESYSTEM_UNKNOWN_FILESYSTEM;


	fat->Flags = 1;//right now its a dummy value

	return SUCCES;
}

uint32 DetectFAT(void* BootSector) {
	if(!BootSector) return ERRORCODE_WRONG_INPUT;

//Make sure there it is a bootsector
	uint32 BootSign = ((short*) BootSector)[255] & 0xFFFF;
	if (BootSign != 0xAA55) return EC_FILESYSTEM_UNKNOWN_FILESYSTEM;

//check boot jump
	uint32 bootjump = *(uint32*)BootSector & 0x00FF00FF;
	if(bootjump != 0x9000EB) return EC_FILESYSTEM_UNKNOWN_FILESYSTEM;

	uint32 FatType;
	FAT_BS_t *fat_bs = (FAT_BS_t*)BootSector;
	if (fat_bs->total_sectors_16 < 4085) FatType = 12;
	else {
		if (fat_bs->total_sectors_16 < 65525) FatType = 16;
		else FatType = 32;
	}

	if (FatType <= 16) {
		if ((fat_bs->EBPB.fat16.boot_signature & 0x28) != 0x28) {
			return EC_FILESYSTEM_UNKNOWN_FILESYSTEM;
		}
	} else {
		return EC_FILESYSTEM_UNKNOWN_FILESYSTEM;
	}

	return SUCCES;
}
