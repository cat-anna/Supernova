#include "SFS_drv.h"

//VolumeReg_t VolRegData;
#define MAX_VOLUMES_COUNT 5

SFS_Volume_p Volumes[MAX_VOLUMES_COUNT];
HANDLE h_Msg;

SFS_Volume_p AllocNewVolume(){
	int i = 0;
	while(i < MAX_VOLUMES_COUNT && Volumes[i] != 0)i++;
	SFS_Volume_p sfs = (SFS_Volume_p)malloc(sizeof(SFS_Volume_t));
	Volumes[i] = sfs;
	ZeroMemory(sfs, sizeof(SFS_Volume_t));
	return sfs;
}

SFS_Volume_p PickUpVolume(uint32 VolID){
	int i = 0;
	for(;i < MAX_VOLUMES_COUNT; i++)
		if(Volumes[i]->PartitionID == VolID)
			return Volumes[i];
	return 0;
}

/*
uint32 DRV_GetTotalSpace(uint32 VolID){
	MFS_Volume_p vol = PickUpVolume(VolID);
	if(!vol || !MFS_VolumeIsReady(vol))return 0;
	return (vol->InfoSector.ClusterCount - vol->InfoSector.HiddenClusters) * vol->InfoSector.ClusterSize * 512;		
}

uint32 DRV_GetFreeSpace(uint32 VolID){
	MFS_Volume_p vol = PickUpVolume(VolID);
	if(!vol || !MFS_VolumeIsReady(vol))return 0;
	return vol->FreeBlocks.TotalFreeClusters * vol->InfoSector.ClusterSize * 512;
}

static void DRV_ProcessJob(MFS_queue_item_p job){
	switch (job->job){
		case MFS_JOB_FINDFIRST:{
//			printf("FINDFIRST\n");
			char* pat = malloc(job->info);
			SearchRec_t l_sr;
			FindHandle_p fh = (FindHandle_p)malloc(sizeof(FindHandle_t));
			ZeroMemory(fh, sizeof(FindHandle_t));
			ReadProcessMemory(job->src_pid, job->source, (uint32)pat, job->info);
			if(MFS_FindFirst(&Volumes[job->VolID], pat, &l_sr, fh) == 0){
				job->result = (uint32)fh;
				WriteProcessMemory(job->src_pid, (uint32)&l_sr, job->dest, sizeof(SearchRec_t));
			} else {
				free(fh->pat);
				free(fh);
				job->result = 0;
			}		
			job->status = MFS_JOBSTATUS_DONE;
		}return;
		case MFS_JOB_FINDNEXT:{
			SearchRec_t l_sr;
			if(MFS_FindNext(&Volumes[job->VolID], &l_sr, (FindHandle_p)job->source) == 0){
				job->result = 0;
				WriteProcessMemory(job->src_pid, (uint32)&l_sr, job->dest, sizeof(SearchRec_t));
			} else {
				job->result = 1;
			}		
			job->status = MFS_JOBSTATUS_DONE;
		}return;
		case MFS_JOB_FINDCLOSE:{
			if(job->source != 0){
				FindHandle_p fh = (FindHandle_p)job->source;
				free(fh->pat);
				free(fh);
			}
			ZeroMemory(job, sizeof(MFS_queue_item_t));
		}return;
		case MFS_JOB_CREATEFILE:{
			char* file = malloc(job->info);
			
			ReadProcessMemory(job->src_pid, job->source, (uint32)file, job->info);
			FileHandle_p fh = (FileHandle_p)malloc(sizeof(FileHandle_t));
			ZeroMemory(fh, sizeof(FileHandle_t));
			if(MFS_CreateFile(&Volumes[job->VolID], file, job->dest, fh) == 0){
				job->result = (uint32)fh;
			} else {
				free(fh);
				job->result = 0;
			}		
			free(file);
			job->status = MFS_JOBSTATUS_DONE;	
		}return;
		case MFS_JOB_READFILE:{
			FileHandle_p fh = (FileHandle_p)job->source;
			uint32 read, ret;
			if((ret = MFS_ReadFile(&Volumes[job->VolID], fh, job->src_pid, job->dest, job->info, &read)) == 0){
				job->result = read;
			}//else printf("MFS ret: %d\n", ret);
			job->status = MFS_JOBSTATUS_DONE;		
		}return;
		case MFS_JOB_GETFILESIZE:{
			FileHandle_p fh = (FileHandle_p)job->source;
			job->result = MFS_GetFileSize(&Volumes[job->VolID], fh);
			job->status = MFS_JOBSTATUS_DONE;			
		}return;
		case MFS_JOB_CLOSEHANDLE:{
			if(job->source != 0){
				FileHandle_p fh = (FileHandle_p)job->source;
				free(fh->buffer);
				free(fh);
			}
			ZeroMemory(job, sizeof(MFS_queue_item_t));		
		}return;
		case MFS_JOB_WRITEFILE:{
			FileHandle_p fh = (FileHandle_p)job->source;
			uint32 written;//, ret;
			if((ret = MFS_WriteFile(&Volumes[job->VolID], fh, job->src_pid, job->dest, job->info, &written)) == 0){
				job->result = written;
			}//else printf("MFS_WriteFile: %d\n", ret);
			job->status = MFS_JOBSTATUS_DONE;		
		}return;
	}
}
*/

uint32 ReadDevice(uint32 h_dev, uint32 Buf, uint32 LBA, uint32 Count){
	device_request_t req;
	req.request = REQUEST_HANDLE_READ;
	DevStackRead(&req, LBA, Count, Buf);
	uint32 r;
	if(DeviceRequest((HANDLE)h_dev, &req, &r) == 0) return r;
	return 0;
}

uint32 WriteDevice(uint32 DevID, uint32 Buf, uint32 LBA, uint32 Count){
	DevID = Buf = LBA = Count = 0;
	return 0;
}

uint32 DriverFunction(device_request_p req){
//	printf("SFS DriverFunction %d\n", req->request);
	switch (req->request){
	case REQUEST_OPEN_HANDLE:{
		char* path = (char*)malloc(req->Stack.OpenHandle.path_size + 1);
		uint32 sh;
		HANDLE h_fr = CreateSharedFrame(req->Sender, (uint32)req->Stack.OpenHandle.path, req->Stack.OpenHandle.path_size + 1, (uint32)&sh);
		strcpy(path, (void*)sh);
//		printf("SFS path: %s\ts:%d\n", path, req->Stack.OpenHandle.path_size);
		CloseHandle(h_fr);

		SFS_Volume_p sfs = PickUpVolume(req->DevID);
		if(!sfs)return INVALID_HANDLE_VALUE;

		FileHandle_p fh = (FileHandle_p)malloc(sizeof(FileHandle_t));
		ZeroMemory(fh, sizeof(FileHandle_t));
		fh->size = sizeof(FileHandle_t);
		fh->volume = sfs;
		uint32 res = SFS_CreateFile(fh, path, req->Stack.OpenHandle.flags);
		if(res){
			free(fh);
			return INVALID_HANDLE_VALUE;
		}
		return RegisterProcessHandle(req->Sender, (uint32)fh, req->Stack.OpenHandle.flags & FILE_ACCES_FULL);
	}
	case REQUEST_HANDLE_GET_SIZE:{
		FileHandle_p fh_size = (FileHandle_p)req->DevID;
		if(fh_size->size != sizeof(FileHandle_t)) return 0;
		return fh_size->fptr->Size;
	}
	case REQUEST_HANDLE_READ:{
		FileHandle_p fh = (FileHandle_p)req->DevID;
		if(fh->size != sizeof(FileHandle_t)) return 0;

		uint32 count = req->Stack.Read.count;
		uint32 sh_addr;
		HANDLE h_sh = CreateSharedFrame(req->Sender, req->Stack.Read.dst, count, (uint32)&sh_addr);
		uint32 read;
		Breakpoint();
		uint32 ret = SFS_ReadFile(fh, sh_addr, count, &read);

		CloseHandle(h_sh);
		return (ret)?(0):(count);
	}
	default:
		printf("SFS unknown\n");
		break;
	}

	return 0;//RegisterProcessHandle(req->Sender, 555);
}

uint32 MessageHandler(uint32 message, uint32 hparam, uint32 lparam){
//	printf("\nSFS MessageHandler %x %x %x\n", message, hparam, lparam);
	switch(message){
	case MSG_NEW_DEVICE:
			if(lparam == DEV_TYPE_DISK_REMOVABLE){
				HANDLE drv = OpenDevice(hparam);
				SFS_Volume_p sfs = AllocNewVolume();
				uint32 r = SFS_OpenVolume(drv, sfs);
				//printf("SFS %d\n", r);
				if(r != 0){
					CloseHandle(drv);
					//FreeVolume(sfs);
					break;
				}
				sfs->PartitionID = RegisterVolume(sfs->PartitionID, sfs->InfoSector.Name, 0);

			}
		break;
	}
	return 0;
}

int main(){
	RegisterDriver(&DriverFunction, "SFS");
	h_Msg = AllocateMessageHandler(&MessageHandler);
	ZeroMemory(Volumes, MAX_VOLUMES_COUNT * sizeof(SFS_Volume_p));

	while(1){
		msg_header_p msg;
		if(GetMessage(&msg)){
		//	if(msg->message == MSG_EXIT)return ((message_p)msg)->hparam;
		//	printf("dispatch\n");
			DispatchMessage(msg);
		}
		else
			Sleep(0);
	}
	/*DriverID = DID;
	LastFreeVolume = 0;
	ZeroMemory(Volumes, sizeof(MFS_Volume_t) * MAX_VOLUMES_COUNT);
	
	ZeroMemory(&VolRegData, sizeof(VolumeReg_t));
	VolRegData.FindFirst = &DRV_FindFirst;
	VolRegData.FindNext = &DRV_FindNext;
	VolRegData.FindClose = &DRV_FindClose;
	VolRegData.CreateFile = &DRV_CreateFile;
	VolRegData.ReadFile = &DRV_ReadFile;
	VolRegData.GetFileSize = &DRV_GetFileSize;
	VolRegData.CloseHandle = &DRV_CloseHandle;
	VolRegData.GetTotalSpace = &DRV_GetTotalSpace;
	VolRegData.GetFreeSpace = &DRV_GetFreeSpace;
	VolRegData.WriteFile = &DRV_WriteFile;

	message_t msg;
	while(1){
		Sleep(0);
		if(GetMessage(&msg) == 0){
			switch (msg.type){
				case MSG_NEW_DEVICE:
					if(msg.lparam == DEV_TYPE_DISK_REMOVABLE || msg.lparam == DEV_TYPE_DISK_SOLID)
						RegisterNewVolume(msg.hparam);
					break;
			}
		}
		uint32 i = 0;
		MFS_queue_item_p q = MFS_jobQeueue;
		for(;i < MFS_JOB_QUEUE_SIZE; i++)
			if(q->status == MFS_JOBSTATUS_READY){
				DRV_ProcessJob(q);
				break;
			}else q++;
	}*/
	return 0;
}
