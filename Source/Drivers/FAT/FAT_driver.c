#include <Supernova.h>
#include "Supernova/di.h"
#include "FAT.h"
#include <string.h>

enum {
	FAT_VOLUMES_COUNT		= 3,
};

uint32 DriverID;
volatile unsigned Drv_state;

typedef struct FATVolume_s {
	uint32 Flags;
	FAT_t fat;
	VolumeInfo_t VolInfo;
	HANDLE DriveHandle;
}FATVolume_t, *FATVolume_p;

FATVolume_t Volumes[FAT_VOLUMES_COUNT];

uint32 FAT_CheckRoot(FAT_p FAT){
	if(FAT->Root) return SUCCES;
	if(!FAT->DriveHandle)return ERRORCODE_FATAL;
	uint32 sz = sizeof(FAT_FileEntry_t) * FAT->RootEntryCount;
	FAT->Root = (FAT_FileEntry_t*)kmalloc(sz);
	if(DirectSeek(FAT->DriveHandle, FAT->RootDirectory * FAT->ClusterSize, SEEK_BEGINING) != SUCCES) return FAT_READ_ROOT_ERROR;
	uint32 read;
	DirectRead(FAT->DriveHandle, FAT->Root, sz, &read);
	if(read != sz) return FAT_READ_ROOT_ERROR;
	return SUCCES;
}

uint32 FAT_CheckFAT(FAT_p FAT){
	if(FAT->FAT) return SUCCES;
	if(!FAT->DriveHandle)return ERRORCODE_FATAL;
	uint32 sz = FAT->FatSize * FAT->BytesPerSector;
	FAT->FAT = kmalloc(sz);
	if(DirectSeek(FAT->DriveHandle, 1 * FAT->BytesPerSector, SEEK_BEGINING) != SUCCES) return FAT_READ_ROOT_ERROR;
	uint32 read;
	DirectRead(FAT->DriveHandle, FAT->FAT, sz, &read);
	if(read != sz) return FAT_READ_FAT_ERROR;
	return SUCCES;
}

uint32 FAT_Vol_WriteFile(VolumeInfo_p vol, uint32 Handle, uint32 fDest, void *SrcBuf, uint32 Count, uint32 *Written){
	kprintf("fat: %s\n", __FUNCTION__);
	vol = 0;
	Handle = 0;
	fDest = 0;
	SrcBuf = 0;
	Count = 0;
	Written = 0;
	return 1;
}

uint32 FAT_Vol_SeekFile(VolumeInfo_p vol, uint32 Handle, uint32 Pos, uint32 Mode){
	kprintf("fat: %s\n", __FUNCTION__);
	vol = 0;
	Handle = Pos = Mode = 0;
	return 1;
}

uint32 FAT_Vol_OpenFile(VolumeInfo_p vol, const char* constpath, uint32 *Handle, uint32 Mode){
	FATVolume_p FATVol = (FATVolume_p)vol->VolumePointer;
	FAT_p FAT = &FATVol->fat;
	if(!Handle || !FAT || FAT_CheckRoot(FAT) != SUCCES) return 1;

	char Copy[250];
	strcpy(Copy, constpath);
	FAT_FilePointer_t FPtr;
	memset(&FPtr, 0, sizeof(FAT_FilePointer_t));
	//char *path = Copy;
	//char *filename = strrchr(Copy, '/');

	uint32 found = 0;

	if (FindFile(FAT, Copy, &FPtr) == SUCCES) {
		if(Mode & FILE_CREATE){
//			if(FPtr.FatFile.Name[0] != 0 && FPtr.FatFile.Name[0] != 0xE5){
//				FAT_Load();
//				FAT_FreeClusterChain(FPtr.FatFile.LowCluster);
//				FAT_Save();
//			}
//			FPtr.FatFile.LowCluster = 0;
//			FPtr.FatFile.FileSize = 0;
//			FPtr.FatFile.LastAccessedDate = 0;
//
//			FPtr.FatFile.CreationTime = FPtr.FatFile.CreationDate = 0;

//			struct tm * timeinfo;
//			time_t curr = time(NULL);
//			timeinfo = localtime(&curr);
//			FPtr.FatFile.CreationTime = static_cast<unsigned short>(FAT_MakeTime(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec));
//			FPtr.FatFile.CreationDate = static_cast<unsigned short>(FAT_MakeDate(timeinfo->tm_year, timeinfo->tm_mon + 1, timeinfo->tm_mday));

//			FPtr.FatFile.LastModificationDate = FPtr.FatFile.CreationDate;
//			FPtr.FatFile.LastModificationTime = FPtr.FatFile.CreationTime;
		}
//		kprintf("by open\n");
		found = true;
	}
	if (!found && (Mode & FILE_CREATE)) {
/*		if (!filename) {
			filename = Copy;
			path = 0;
		} else {
			*filename = 0;
			filename++;
		}
		FAT_FilePointer_t FParent = {};
*/
//		if (!FindFile(path, &FParent))SetErrorAndReturn(8,0);
//		if (!AllocFile(filename, &FParent, &FPtr))SetErrorAndReturn(9,0);
//		found = true;
	}
	if (!found) return 1;

	FAT_FileHandle_t *fh = (FAT_FileHandle_t*)kmalloc(sizeof(FAT_FileHandle_t));
	memcpy(&fh->Ptr, &FPtr, sizeof(FAT_FilePointer_t));
	fh->Position = 0;
	fh->Flags = Mode;

	*Handle = (uint32)fh;
	return SUCCES;
}

uint32 FAT_Vol_CloseFile(VolumeInfo_p vol, uint32 Handle){
	FAT_FileHandle_t *fh = (FAT_FileHandle_t*)Handle;
//	FATVolume_p FATVol = (FATVolume_p)vol->VolumePointer;
//	FAT_p FAT = &FATVol->fat;
	kfree(fh);
	return SUCCES;
}

uint32 FAT_Vol_ReadFile(VolumeInfo_p vol, uint32 Handle, uint32 fSrc, void *destBuf, uint32 Count, uint32 *Read){
	FAT_FileHandle_t *fh = (FAT_FileHandle_t*)Handle;
	FATVolume_p FATVol = (FATVolume_p)vol->VolumePointer;
	FAT_p FAT = &FATVol->fat;
	if(!fh || !destBuf || !Read) return ERRORCODE_WRONG_INPUT;
	if(!(fh->Flags & ACCES_READ)) return EC_READ_NOT_ALLOWED;

	if(fSrc != fh->Position){
		return ERRORCODE_FATAL;
	}

	unsigned filebegin = fh->Position;
	//unsigned fileend = filebegin + Count;
	unsigned FirstCluster = filebegin / FAT->ClusterSize;
//	unsigned LastCluster = fileend / FAT->ClusterSize;
	unsigned ClusterOffset = filebegin % FAT->ClusterSize;
	unsigned FollowCluster = fh->Ptr.FatFile.LowCluster;
	unsigned DataAreaOffset = FAT->DataStart - 2;

	if(FollowCluster == 0){
		*Read = 0;
		return EC_READ_OUT_OF_BOUND;
	}

	FAT_CheckFAT(FAT);

	uint32 i;
	for(i = 0; i < FirstCluster; ++i){
		kprintf("for FirstCluster %d\n", FirstCluster);
		FollowCluster = FAT12_Get(FAT->FAT, FollowCluster);
	}

	char *Buffer = kmalloc(FAT->ClusterSize);
	char *Dest = (char*)destBuf;

	if(ClusterOffset){
		DirectSeek(FAT->DriveHandle, FAT->ClusterSize * (DataAreaOffset + FollowCluster), SEEK_BEGINING);
		uint32 read;
		DirectRead(FAT->DriveHandle, Buffer, FAT->ClusterSize, &read);
		if(read != FAT->ClusterSize){
			kfree(Buffer);
			return ERRORCODE_FATAL;
		}
		read = FAT->ClusterSize - ClusterOffset;
		kprintf("CLUSTER offset:%d r:%d\n", ClusterOffset, read);
		memcpy(Dest, Buffer, read);
		Dest += read;
		FollowCluster = FAT12_Get(FAT->FAT, FollowCluster);
	}

	unsigned TotalSize = Count;
	while(Count){
		uint32 RF = FAT->ClusterSize * (DataAreaOffset + FollowCluster);
		DirectSeek(FAT->DriveHandle, RF, SEEK_BEGINING);
		uint32 read;
		DirectRead(FAT->DriveHandle, Buffer, FAT->ClusterSize, &read);
		if(read != FAT->ClusterSize){
			kfree(Buffer);
			return 1;
		}

		unsigned Size2Read;
		Size2Read = (Count < FAT->ClusterSize)?(Count):(FAT->ClusterSize);
		memcpy(Dest, Buffer, Size2Read);

//		kprintf("FAT read c:%d f:%d r:%d\n", Count, FollowCluster, Size2Read);

		Count -= Size2Read;
		Dest += Size2Read;

		if(Count > 0){
			FollowCluster = FAT12_Get(FAT->FAT, FollowCluster);
//			kprintf("FollowCluster %x\n", FollowCluster);
			if(FollowCluster == 0x0FF8)
				break;
		}
	}

	*Read = TotalSize - Count;
	fh->Position += *Read;

	kfree(Buffer);
	return SUCCES;
}

uint32 FAT_Vol_GetFileSize(VolumeInfo_p vol, uint32 Handle){
	FAT_FileHandle_t *h = (FAT_FileHandle_t*)Handle;
	vol = 0;
	return h->Ptr.FatFile.FileSize;
}

uint32 FAT_Command(uint32 cmd, uint32 hparam, uint32 lparam){
	switch(cmd){
	case COMMAND_FILESYSTEM_DETECT:{
		uint32 r = DetectFAT((void*)hparam);
		if(r != SUCCES)return r;
		FATVolume_t *vol = Volumes;
		uint32 i;
		for(i = 0; i < FAT_VOLUMES_COUNT; ++i, ++vol)
			if(vol->Flags == 0)	break;

		r = InitFAT((FAT_BS_t*)hparam, &vol->fat);
		if(r != SUCCES)return r;

		vol->VolInfo.DeviceID = lparam;
		vol->VolInfo.VolumeID = vol->fat.VolumeID;
		vol->VolInfo.DriverID = DriverID;
		vol->VolInfo.State = 0;
		vol->VolInfo.VolumePointer = vol;

		vol->VolInfo.Functions.OpenFile = FAT_Vol_OpenFile;
		vol->VolInfo.Functions.CloseFile = FAT_Vol_CloseFile;
		vol->VolInfo.Functions.ReadFile = FAT_Vol_ReadFile;
		vol->VolInfo.Functions.WriteFile = FAT_Vol_WriteFile;
		vol->VolInfo.Functions.SeekFile = FAT_Vol_SeekFile;
		vol->VolInfo.Functions.GetFileSize = FAT_Vol_GetFileSize;

		r = RegisterVolume(DriverID, &vol->VolInfo, vol->fat.Name);
		if(r != SUCCES)return r;
		InternalMessage(DriverID, FAT_MSG_INIT_VOLUME, (uint32)vol, 0);
		vol->Flags = 1;//dummy value
		break;
	}
	}
	return SUCCES;
}

uint32 DrvEntry(uint32 DrvID){
	DriverID = DrvID;

	memset(Volumes, 0, FAT_VOLUMES_COUNT * sizeof(FATVolume_t));
	DriverFunctions_t DrvFun;
	memset(&DrvFun, 0, sizeof(DriverFunctions_t));
	//DrvFun.Initialize = &FAT_Initialize;
	DrvFun.Command = &FAT_Command;
	RegisterDriver(DrvID, &DrvFun, "FAT", DRIVER_INFO_FILE_SYSTEM);

	Drv_state = 1;
	while(Drv_state > 0){
		Sleep(100);

		message_t msg;
		if(InternalGetMessage(&msg) != SUCCES)continue;

		switch (msg.message){
		case FAT_MSG_INIT_VOLUME:{
			char buf[50] = "/dev/";
			char *devName = strrchr(buf, '/');
			++devName;

			FATVolume_t *v = (FATVolume_t*)msg.hparam;
			HANDLE handle;
			if(GetDeviceName(v->VolInfo.DeviceID, devName) != SUCCES){
				//ERROR, unregister volume!!
				kprintf("ERROR get name!\n");
				break;
			}

			handle = DirectOpen(DriverID, buf, ACCES_FULL);
			if(!handle){
				//ERROR, unregister volume!!
				kprintf("ERROR volume open!\n");
				break;
			}
			v->fat.DriveHandle = handle;
			v->DriveHandle = handle;
			break;
		}
		}
	}
	return SUCCES;
}
