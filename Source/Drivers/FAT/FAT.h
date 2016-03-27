#ifndef _FAT_H_
#define _FAT_H_

#ifndef _BUILDING_SSM_
#include <Supernova.h>
//#include <Supernova/di.h>
#endif

enum{
	FAT_ATTRIBUTE_READ_ONLY			= 0x01,
	FAT_ATTRIBUTE_HIDDEN			= 0x02,
	FAT_ATTRIBUTE_SYSTEM			= 0x04,
	FAT_ATTRIBUTE_VOLUME_ID			= 0x08,
	FAT_ATTRIBUTE_DIRECTORY			= 0x10,
	FAT_ATTRIBUTE_ARCHIVE			= 0x20,

	FAT_ATTRIBUTE_LONG_FILE_NAME	= 0x0F,
};

typedef struct fat_File_Entry {
	char Name[8];
	char Ext[3];
	unsigned char Attributes;
	char NTReserved;
	unsigned char CreationTime_mili;
	unsigned short CreationTime;
	unsigned short CreationDate;
	unsigned short LastAccessedDate;
	unsigned short HighCluster;
	unsigned short LastModificationDate;
	unsigned short LastModificationTime;
	unsigned short LowCluster;
	unsigned int FileSize;
}__attribute__((packed))  fat_File_Entry_t, FAT_FileEntry_t;

typedef struct fat_LFN_Entry
{
	char order;
	short first[5];
	char attribute;
	char charzero;
	unsigned char checksum;
	short middle[6];
	short shortzero;
	short last[2];
}__attribute__((packed))  fat_LFN_Entry_t;

static inline unsigned short FAT_MakeTime(unsigned short Hour, unsigned short Minute, unsigned short Second)
{
	Second	= (Second >>  1) & 0x001F;
	Minute	= (Minute <<  5) & 0x03E0;
	Hour	= (Hour   << 11) & 0xFC00;
	return Hour | Minute | Second;
}

static inline unsigned short FAT_MakeDate(unsigned short Year, unsigned short Month, unsigned short Day)
{
	Year -= 2000;
	Day	= Day & 0x000F;
	Month = (Month << 5) & 0x00E0;
	Year  = (Year  << 9) & 0xFF00;
	return Year | Month | Day;
}

typedef struct fat_extBS_32
{
	//extended fat32 stuff
	unsigned int		table_size_32;
	unsigned short		extended_flags;
	unsigned short		fat_version;
	unsigned int		root_cluster;
	unsigned short		fat_info;
	unsigned short		backup_BS_sector;
	unsigned char 		reserved_0[12];
	unsigned char		drive_number;
	unsigned char 		reserved_1;
	unsigned char		boot_signature;
	unsigned int 		volume_id;
	unsigned char		volume_label[11];
	unsigned char		fat_type_label[8];

}__attribute__((packed)) fat_extBS_32_t;

typedef struct fat_extBS_16
{
	//extended fat12 and fat16 stuff
	unsigned char	bios_drive_num;
	unsigned char	reserved1;
	unsigned char	boot_signature;
	unsigned int	volume_id;
	char			volume_label[11];
	char			fat_type_label[8];

}__attribute__((packed)) fat_extBS_16_t;

typedef struct FAT_BS_s{
	unsigned char 		bootjmp[3];
	unsigned char 		oem_name[8];
	unsigned short 	    bytes_per_sector;
	unsigned char		sectors_per_cluster;
	unsigned short		reserved_sector_count;
	unsigned char		table_count;
	unsigned short		root_entry_count;
	unsigned short		total_sectors_16;
	unsigned char		media_type;
	unsigned short		table_size_16;
	unsigned short		sectors_per_track;
	unsigned short		head_side_count;
	unsigned int 		hidden_sector_count;
	unsigned int 		total_sectors_32;

	union fat_EBPB_u{
		fat_extBS_32_t fat32;
		fat_extBS_16_t fat16;
	}EBPB;
}__attribute__((packed)) FAT_BS_t, *FAT_BS_p;



typedef struct FAT_s {
	uint32 Flags;
	uint32 VolumeID;
	uint32 ClusterSize;
	uint32 DataStart;
	uint32 FatType;
	uint32 BytesPerSector;
	char Name[32];

	HANDLE DriveHandle;

//Root Directory
	FAT_FileEntry_t *Root;
	uint32 RootEntryCount;
	uint32 RootDirectory;	//in sectors

//fat table
	void *FAT;
	uint32 FatCount;
	uint32 FatSize;

} FAT_t, *FAT_p;

typedef struct FAT_FilePointer_s {
	FAT_FileEntry_t FatFile;
	uint32 Cluster, EntryNo;
} FAT_FilePointer_t, *FAT_FilePointer_p;

typedef struct FAT_FileHandle_s {
	FAT_FilePointer_t Ptr;
	wchar_t *LFN;
	unsigned Position;
	unsigned Flags;
}FAT_FileHandle_t;


uint32 DetectFAT(void* BootSector);

uint32 InitFAT(FAT_BS_t *fat_bs, FAT_t *fat);

uint32 FindFile(FAT_p FAT, const char* Path, FAT_FilePointer_p Ptr);

uint32 FAT_EntryGetName(char *name, fat_File_Entry_t *entry);

unsigned short FAT12_Get(unsigned char *FAT, unsigned short no);
void FAT12_Set(unsigned char *FAT, unsigned short no, unsigned short value);

#endif
