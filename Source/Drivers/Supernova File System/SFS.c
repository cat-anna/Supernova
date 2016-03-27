#include "SFS.h"

//SSM - Supernova System Manager

static inline bool GetFreeBlock(FreeBlocksMeneger_p fbm, BlockData_p block){
	BlockData_p freeb = fbm->Blocks_begin;
	if(freeb->start == 0 || freeb->size == 0)return false;	
	memcpy(block, freeb, sizeof(BlockData_t));	
	return true;
}

bool InsertUsedBlock(FreeBlocksMeneger_p fbm, uint32 ub_start, uint32 ub_size){
	if(!ub_start || !ub_size)return true;
	BlockData_p block = fbm->Blocks_begin;
	for(; block < fbm->Blocks_end; block++)
   		if(block->start == ub_start){
			if(block->size == ub_size){
				BlockData_p block_next = block;	
				block_next++;
				for(; block_next < fbm->Blocks_end; block_next++, block++){
					memcpy(block, block_next, sizeof(BlockData_t));
				}
				break;	
			}else {		
				block->start += ub_size;
				block->size -= ub_size;		
				break;
			}
		} else
		if(block->start < ub_start && block->start + block->size == ub_start + ub_size){
			block->size -= ub_size;			
			break;
		}else 
			return false;

	fbm->TotalFreeClusters -= ub_size;
	return true;
}

bool AddFreeSpace(FreeBlocksMeneger_p fbm, uint32 fb_start, uint32 fb_size){
	if(!fb_start || !fb_size)return true;
	BlockData_p block = fbm->Blocks_begin;
	
	for(; block < fbm->Blocks_end; block++)
		if(block->start == fb_start + fb_size){
			block->start -= fb_size;
			block->size += fb_size;
			break;
		}else
		if(block->start + block->size == fb_start){
			block->size += fb_size;
			break;
		} else 
		if(block->start < fb_start){
			BlockData_t b;
			b.size = fb_size;
			b.start = fb_start;


			BlockData_p block_end = fbm->Blocks_end;	
			block_end--;
			if(block_end->start != 0){
				return false;
			}
			for(; block < --block_end;){
				BlockData_p b = block_end;
				b++;
				memcpy(block_end, b, sizeof(BlockData_t));
			}

			break;				
		}	
		
	fbm->TotalFreeClusters += fb_size;
	return true;
}

static inline bool ReadFilesTable(SFS_Volume_p vol){
	if(vol->FilesTable)return true;
	vol->FilesCount = vol->InfoSector.FileStructSize * FILES_PER_SECTOR;

	uint32 size = sizeof(FileSector_t) * vol->InfoSector.FileStructSize;
	vol->SectorTable = (FileSector_p)malloc(size);
	
	if(ReadDevice(vol->DevID, (uint32)vol->SectorTable,
			vol->InfoSector.FileStructPos, vol->InfoSector.FileStructSize) != vol->InfoSector.FileStructSize){
		free(vol->SectorTable);
		return false;
	}
	
	vol->FilesTable = (FileData_p)vol->SectorTable;
	return true;
}

static inline bool ScanForFreeBlocks(SFS_Volume_p vol){
	if(vol->FreeBlocks.Blocks_begin && vol->FreeBlocks.Blocks_end)return true;
	
	uint32 bsize = sizeof(BlockData_t) * 10;
	vol->FreeBlocks.Blocks_begin = (BlockData_p)malloc(bsize);
	vol->FreeBlocks.Blocks_end = (BlockData_p)((uint32)vol->FreeBlocks.Blocks_begin + bsize);
	ZeroMemory(vol->FreeBlocks.Blocks_begin, bsize);
	BlockData_p fcd = vol->FreeBlocks.Blocks_begin;		
	fcd->start = vol->InfoSector.HiddenClusters;
	fcd->size = vol->InfoSector.ClusterCount - fcd->start;
	vol->FreeBlocks.TotalFreeClusters = fcd->size;
	
	uint32 i = 0;
	FileData_p f = vol->FilesTable;
	for(; i < vol->FilesCount; i++, f++)
		if(f->Date > 0){
			uint32 j = 0;
			for(; j < FILE_SEGMENTS; j++)
				if(f->Segments[j].Position)
					InsertUsedBlock(&vol->FreeBlocks, f->Segments[j].Position, f->Segments[j].Size);
		}
	return true;
}

void FreeFile(SFS_Volume_p vol, FileData_p F){
	if(!F)return;
	
	uint32 i = 0;
	for(; i < FILE_SEGMENTS; i++){
		AddFreeSpace(&vol->FreeBlocks, F->Segments[i].Position, F->Segments[i].Size);
	}
	ZeroMemory(F, sizeof(FileData_t));
}

bool SFS_VolumeIsReady(SFS_Volume_p vol){
	return  ReadFilesTable(vol) 		&&
			ScanForFreeBlocks(vol);
}

bool SFS_UpdateFile(SFS_Volume_p vol, FileData_p f){
	uint32 s_offset = (uint32)f - (uint32)vol->FilesTable;
	s_offset /= sizeof(FileData_t);
	s_offset /= FILES_PER_SECTOR;
	
	if(WriteDevice(vol->DevID, ((uint32)vol->SectorTable + s_offset * sizeof(FileSector_t)),
			vol->InfoSector.FileStructPos + s_offset, 1) != 1){
		vol->ErrorFlags |= SFS_ERROR_UPDATE_FTABLE;
		return false;
	}
	return true;	
}

uint32 SFS_FindNext(SFS_Volume_p vol, SearchRec_p SR, FindHandle_p FH){
	uint32 i = FH->FileNumber;
	FileData_p f = &vol->FilesTable[i];
	for(; i < vol->FilesCount; i++){		
		if(f->Date > 0 && f->ParentDir == FH->ParentDir &&	
			!strcmppat(f->Name, FH->pat) &&					
			!AttribIsSet(f->Attributes, FILE_FLAG_DELETED))
		{
			FH->FileNumber = i + 1;
			memcpy(SR->name, f->Name, FILE_NAME_LENGTH);
			SR->flags = f->Attributes;
			SR->date = f->Date;
			SR->size = f->Size;			
			return 0;
		} 
		f++;
	}
	return 3;
}

uint32 SFS_FindFirst(SFS_Volume_p vol, char *Pat, SearchRec_p sr, FindHandle_p FH){
	if(!SFS_VolumeIsReady(vol))return 1;
	
	uint16 ParentDir;		
	char *p = ParsePath(vol, Pat, &ParentDir);	
	
	FH->ParentDir = ParentDir;
	if(p && *p){
		FH->pat = (char*)malloc(strlen(p) + 1);
		strcpy(FH->pat, p);	
	}else{
		FH->pat = (char*)malloc(2);
		strcpy(FH->pat, "*");		
	}
	FH->FileNumber = 0;
	if(ParentDir && !strcmppat("..", FH->pat)){
		strcpy(sr->name, "..");
		sr->flags = vol->FilesTable[ParentDir].Attributes;
		sr->date = vol->FilesTable[ParentDir].Date;
		sr->size = vol->FilesTable[ParentDir].Size;
		return 0;
	} else 
		return SFS_FindNext(vol, sr, FH);
}

FileData_p FindFile(SFS_Volume_p vol, char *FName, uint16 ParentNumber, uint16 *FNumber, uint32 findflags){
	if(!FName || !*FName)return 0;
	if(!SFS_VolumeIsReady(vol))return 0;
	FileData_p ft = vol->FilesTable;
	uint32 i = 0;
	for(; i < vol->FilesCount; i++)
		if(ft->Date > 0 &&
			!strcmp(ft->Name, FName) &&//strcmp pattern!!!!!!!!!!!!!!1
			ft->ParentDir == ParentNumber &&
			!AttribIsSet(ft->Attributes, FILE_FLAG_DELETED) &&
			AttribIsSet(ft->Attributes, findflags))
		{
			if(FNumber)*FNumber = i + 1;	
			return ft;
		} 
			else ft++;
	return 0;	
}

uint32 SFS_OpenVolume(uint32 DevID, SFS_Volume_p mfs){
	uint8 *instbuf = (uint8*)malloc(1024);
	
	if(ReadDevice(DevID, (uint32)instbuf, 0, 2) == 2 &&
			!strcmp("SFS", (const char*)((PBS_p)instbuf)->fsName)){
		mfs->DevID = DevID;
		mfs->PartitionID = ((PBS_p)instbuf)->PartitionID;
		memcpy(&mfs->InfoSector, (void*)((uint32)instbuf + sizeof(PBS_t)), 512);
	} else{
	//	printf("|%s|\n", ((PBS_p)instbuf)->fsName);
		free(instbuf);
		return 1;
	}	
	free(instbuf);
	return 0;
}

uint32 SFS_CreateFile(FileHandle_p fh, char* file, uint32 rights){
	uint16 PNumber;
	char *fname = ParsePath(fh->volume, file, &PNumber);
	if(!fname)return 1;
	fh->flags = rights;

//	printf("parsed: %s\n", fname);

	FileData_p F = FindFile(fh->volume, fname, PNumber, 0, 0);

//	printf("found %x\n", rights);
	if(rights & FILE_OPEN){
		if(!F && (rights & FILE_ALLWAYS)){
			rights &= FILE_CREATE;
		}
	}
	if(rights & FILE_CREATE){
		if(!F)F = FindFreeFileRecord(fh->volume, 0);
		else if(rights & FILE_ALLWAYS)FreeFile(fh->volume, F);
	}

	if(!F)return 2;
		
	if(rights & FILE_CREATE){
		strcpy(F->Name, fname);
		F->ParentDir = PNumber;
		F->Date = EncodeDate(2011, 1, 15, 12, 24);
		F->Size = 0;
		if(!SFS_UpdateFile(fh->volume, F))return 3;
	} 	
	fh->fptr = F;
	return 0;
}

char* ParsePath(SFS_Volume_p vol, char* path, uint16 *ParentNumber){
	if(!path)return 0;
	if(!SFS_VolumeIsReady(vol))return 0;
	
	char* p = (char*)malloc(strlen(path) + 1);
	char* copy = p;
	strcpy(p, path);	
	
	uint16 ParentDir = 0;
	if(*p == '/')p++;
	
	char* pos;
	while((pos = strchr(p, '/'))){
		uint16 SNumber = 0;
		*pos = 0;
		pos++;
		if(!strcmp("..", p)){
			ParentDir = vol->FilesTable[ParentDir].ParentDir;
		} else {
			FileData_p F = FindFile(vol, p, ParentDir, &SNumber, FILE_FLAG_DIR);
			if(F != NULL)ParentDir = SNumber;
			else break;
		}
		p = pos;
	}		
	
	if(ParentNumber)*ParentNumber = ParentDir;
	char* fnamepos = (char*)(path + (p - copy));
	free(copy);
	return fnamepos;
}

uint32 SFS_ReadFile(FileHandle_p fh, uint32 buf, uint32 buf_size, uint32* Read){
	if(!SFS_VolumeIsReady(fh->volume))return 1;
	if(!fh || !buf || !buf_size || !Read)return 2;
	if(!(fh->flags & FILE_ACCES_READ))return 3;
	if(fh->fptr->Attributes & FILE_FLAG_DIR)return 4;

	*Read = 0;	

	uint32 sizetoread = MinValue(buf_size, fh->fptr->Size - fh->pos);
	if(sizetoread == 0)return 0;
	
	uint32 cluster_size = fh->volume->InfoSector.ClusterSize * 512;
	uint32 hpos_clusteroffset = fh->pos % cluster_size;
	
	if(fh->buffer){
		uint32 from_buf = MinValue(cluster_size - hpos_clusteroffset, sizetoread);	
		memcpy((void*)(buf + hpos_clusteroffset), fh->buffer, from_buf);
		sizetoread -= from_buf;
		buf += from_buf;
		fh->pos += from_buf;
		*Read += from_buf;
	}
	if(sizetoread == 0)return 0;
	
	uint32 StartCluster = fh->pos / cluster_size;
	uint32 ClustersToRead = (sizetoread / cluster_size) + 1;

//	print("StartCluster:\t%d\n", StartCluster);
//	print("ClustersToRead:\t%d\n", ClustersToRead);
//	print("dest:\t%x\n", Dest);
//	print("sizetoread:\t%d\n", sizetoread);
//	print(":\t%d\n", );
	
	FileData_p file = fh->fptr;
	uint32 i = 0;		
	
	uint32 rbuf = (uint32)malloc(ClustersToRead * cluster_size);
	uint32 tmp = rbuf;
	uint32 EC = 0;
	uint32 done = 0;
	
	while(ClustersToRead > 0 && i < FILE_SEGMENTS){
		uint32 read_cl = MinValue(file->Segments[i].Size, ClustersToRead);
		uint32 readsec = read_cl * fh->volume->InfoSector.ClusterSize;
//		print("readsec:%d\n", readsec);
		uint32 sec = file->Segments[i].Position * fh->volume->InfoSector.ClusterSize;
//		print("sec:%d\n", sec);
		if(ReadDevice(fh->volume->DevID, tmp, sec, readsec) != readsec){
			EC = 1;
			break;
		}
		tmp += readsec * 512;
		ClustersToRead -= read_cl;
		StartCluster += read_cl;
		done += read_cl;
		i++;
	}

	if(EC == 0){	
		done *= cluster_size;		
		memcpy((void*)(buf), (void*)(rbuf + fh->pos % cluster_size), sizetoread);
		fh->pos += sizetoread;
		*Read += sizetoread;
	}
	if(!fh->buffer)fh->buffer = malloc(cluster_size);
	if(ClustersToRead > 1)
		memcpy(fh->buffer, (void*)(rbuf + (ClustersToRead - 1) * cluster_size), cluster_size);
	free((void*)rbuf);
	return EC;
}

FileData_p FindFreeFileRecord(SFS_Volume_p vol, uint16 *FNumber){
	if(!SFS_VolumeIsReady(vol))return 0;
	uint32 i = 0;
	for(; i < vol->FilesCount; i++)
		if(vol->FilesTable[i].Date == 0){//plus kilka innych checków
			if(FNumber)*FNumber = i;
			return &vol->FilesTable[i];
		}
	return 0;
}

uint32 SFS_GetFileSize(SFS_Volume_p vol, FileHandle_p FH){
	vol = 0;
	return FH->fptr->Size;
}

uint32 SFS_GetFileDate(SFS_Volume_p vol, FileHandle_p FH){
	vol = 0;
	return FH->fptr->Date;
}

sint32 SFS_GetFileClusterPosition(FileData_p F, uint32 Cluster, uint32 *FirstSector){
	*FirstSector = 0;
	if(!F)return -1;
	
	FilePartInfo_p Part = F->Segments;
	uint32 i = 0;
	uint32 total = 0;
	for(; i < FILE_SEGMENTS; i++, Part++)
		if(total + Part->Size >= Cluster){
			*FirstSector = Part->Position + ((total + Part->Size) - Cluster);
			return i;
		}
	return -1;
}

//#define _SFS_WRITE_FILE_DEBUG_

uint32 SFS_WriteFile(SFS_Volume_p vol, FileHandle_p H, uint32 src_pid, uint32 buf,
			uint32 buf_size, uint32* Written){
	if(!SFS_VolumeIsReady(vol))return 1;
	if(!H || !buf || !buf_size || !Written)return 2;	
	if(!(H->flags & FILE_ACCES_WRITE))return 3;
	if(H->fptr->Attributes & FILE_FLAG_DIR)return 4;

//	uint32 towrite = buf_size;
	*Written = 0;
	uint32 cluster_size = vol->InfoSector.ClusterSize;
	uint32 cluster_size_bytes = cluster_size << 9; //*512
	uint32 firstCl = H->pos / cluster_size_bytes;
	uint32 lastCl = (H->pos + buf_size) / cluster_size_bytes;
	if(H->pos % cluster_size_bytes == 0){
		firstCl++;
		lastCl++;
	}
	uint32 countCl = lastCl - firstCl + 1;
#ifdef _SFS_WRITE_FILE_DEBUG_
	printf("firstCl=%d\n", firstCl);
	printf("lastCl=%d\n", lastCl);
	printf("countCl=%d\n", countCl);
	printf("buf_size=%d\n", buf_size);
#endif

//	printf("=%d\n", );
	
	void* write_buffer = malloc(countCl * cluster_size_bytes);

	if(H->buffer){
		memcpy(write_buffer, H->buffer, cluster_size_bytes);
	}
	
	if(countCl > 1 && H->fptr->Size / cluster_size_bytes >= lastCl){
		uint32 FirstSector;				
		if(SFS_GetFileClusterPosition(H->fptr, lastCl, &FirstSector) >= 0){
			printf("FirstSector=%d\n", FirstSector);	
			ReadDevice(vol->DevID, ((uint32)write_buffer + (countCl - 1) * cluster_size),
					FirstSector, vol->InfoSector.ClusterSize);
		}
	}
	src_pid = 0;
	ReadProcessMemory(src_pid, buf, ((uint32)write_buffer + (H->pos % cluster_size)), buf_size);
	if(!H->buffer)H->buffer = malloc(cluster_size_bytes);
	memcpy(H->buffer, (void*)((uint32)write_buffer + (countCl - 1) * cluster_size_bytes), cluster_size_bytes);
	FilePartInfo_p Segment = H->fptr->Segments;
	uint32 i = 0;
	uint32 totalCl = 0;
	uint32 done = 0;
	while(countCl > 0 && i < FILE_SEGMENTS){
		uint32 startCl = 0;
		uint32 Cl2W = 0;
#ifdef _SFS_WRITE_FILE_DEBUG_
		printf("\nSEGMENT %d = (%3d, %3d)\n", i, Segment->Position, Segment->Size);
#endif
		if(Segment->Size == 0){
			BlockData_t block;
			if(!GetFreeBlock(&vol->FreeBlocks, &block))break;
			Cl2W = MinValue(countCl, block.size);
			startCl = block.start;
			InsertUsedBlock(&vol->FreeBlocks, startCl, Cl2W);
			Segment->Position = startCl;
			Segment->Size = Cl2W;
		} else
		if(totalCl + Segment->Size == firstCl){
			BlockData_t block;
			if(!GetFreeBlock(&vol->FreeBlocks, &block))break;
			if(Segment->Position + Segment->Size != block.start){
				i++;
				Segment++;
				continue;
			}			
			Cl2W = MinValue(countCl, block.size);
			startCl = block.start;
			InsertUsedBlock(&vol->FreeBlocks, startCl, Cl2W);
			Segment->Size += Cl2W;		
		} else
		if(totalCl + Segment->Size > firstCl){
			uint32 d = (totalCl + Segment->Size) - firstCl - 1;
#ifdef _SFS_WRITE_FILE_DEBUG_
			printf("d=%d\n", d);
#endif
			startCl = Segment->Position + d;
			Cl2W = MinValue(countCl, Segment->Size - d);			
		}
#ifdef _SFS_WRITE_FILE_DEBUG_
		printf("startCl=%d\n", startCl);
		printf("Cl2W=%d\n", Cl2W);
#endif
		
		if(WriteDevice(vol->DevID, (uint32)write_buffer, startCl, Cl2W * cluster_size)
					!= Cl2W * cluster_size)
			break;
		done += Cl2W * cluster_size_bytes;
		countCl -= Cl2W;	
		totalCl += Segment->Size;
		i++;
		Segment++;
	}
	
	*Written = MinValue(done, buf_size);
	H->pos += *Written;
	H->fptr->Size = MaxValue(H->fptr->Size, H->pos);
#ifdef _SFS_WRITE_FILE_DEBUG_
	printf("Written=%d\n", *Written);
	printf("H->pos=%d\n", H->pos);
#endif

	free(write_buffer);
	if(!SFS_UpdateFile(vol, H->fptr))return 5;
	return 0;
}

/*
int cVolume::SeekFile(u32 handle, u32 Count, u8 SeekMethod)
{
	cFileHandle *H = (cFileHandle*)handle;	
	if(!H)return 1;	
	if(!Count)return 0;
	switch(SeekMethod)
	{
		case fsmFromBegining:
			H->Position = Count;
		break;
		case fsmFromEnd:
			H->Position = H->FilePointer->Size - Count;
		break;
		case fsmFrmCurrent:
			H->Position += Count;
		break;
		default:
			return 2;	
	}
	return 0;
}*/

uint32 SFS_DeleteFile(SFS_Volume_p vol, char* file){
	uint16 PNumber;
	char *fname = ParsePath(vol, file, &PNumber);
	if(!fname)return 1;	
	FileData_p F = FindFile(vol, fname, PNumber, 0, 0);
	if(!F)return 0;

	FreeFile(vol, F);
	return SFS_UpdateFile(vol, F);
}

/*
int cVolume::CreateDir(char *DName)
{	
	if(FindFile(DName, ADNumber, NULL) != NULL)return 1;	
	u32 dsp, drp;
	if(!FindFreeFileRecord(&dsp, &drp)) return 2;			
	cFileData *D = &FileStructure[dsp].Files[drp];		
	strcpy(D->Name, DName);
	D->CreationDateTime = EncodeDate(2010, 12, 1, 12, 34);
	D->Attributes = At_Directory;
	D->ParentDir = ADNumber;	
	if(!WriteSector(PInfoSector.FileStructPos + dsp, &FileStructure[dsp], image))return 3;	
	return 0;	
}*/
/*
int cVolume::DeleteFile(char *FName)
{
	for(int i = 0; i < PInfoSector.FileStructSize; i++)
		for(int j = 0; j < FilesPerSector; j++)
		{//CompareStrWithPattern(FindData->Pattern, FileStructure[i].Files[j].Name)
			cFileData *F = &FileStructure[i].Files[j];//
			if((!strcmp(F->Name, FName) &&	ADNumber == F->ParentDir &&
				F->CreationDateTime > 0 && !AttribIsSet(F->Attributes, At_Deleted))
				 || FName[0] == '*')
			{
		//		printf("szukany: %s\nznalieziony: %s\ni:%d j:%d\ncomp:%d\n", FName, F->Name, i, j, strcmp(F->Name, FName));
				for(int k = 0; k < FileSegments; k++)
					FreeClusters.AddFreeSpace(FileStructure[i].Files[j].Fragments[k].Position, FileStructure[i].Files[j].Fragments[k].Size);
				
				ZeroMemory(&FileStructure[i].Files[j], sizeof(FileStructure[i].Files[j]));
//				F->Attributes |= At_Deleted;
				if(!WriteSector(PInfoSector.FileStructPos + i, &FileStructure[i], image)) return 2;
				if(FName[0] != '*') return 0;
			}							
		}	
	return (FName[0] == '*')?(0):(1);
}*/
/*
bool myfunction (FullClusterData i,FullClusterData j) { return (i.Position<j.Position); }

void cVolume::ShowFreeSpace()
{
	FreeClusters.ShowFreeSpace();	
	std::vector<FullClusterData> FCD;
	for(int i = 0; i < PInfoSector.FileStructSize; i++)
		for(int j = 0; j < FilesPerSector; j++)
		{
			if(FileStructure[i].Files[j].CreationDateTime == 0)continue;
			cFileData *F = &FileStructure[i].Files[j];			
			for(int k = 0; k < FileSegments; k++)
			{
				if(F->Fragments[k].Position == 0 || F->Fragments[k].Size == 0)continue;
				FullClusterData c;
				c.Position = F->Fragments[k].Position;
				c.Size = F->Fragments[k].Size;
				c.FName = F->Name;
				FCD.push_back(c);
			}
		}
	sort(FCD.begin(), FCD.end(), myfunction);
	
	printf("\nZajete klastry:\n");	
	std::vector<FullClusterData>::iterator it;
	
	for (it = FCD.begin(); it != FCD.end(); it++)
		printf("\tPoz:%5d Roz:%5d Kon:%5d %s\n", it->Position, it->Size, it->Position + it->Size - 1, it->FName);
		
	printf("\n");
}*/

void SFS_ListSizes(){
  printf("writed structures tree with size in bytes:\n\n");  
//  printf("cMasterBootRecord: %d\n", sizeof(cMasterBootRecord));
//  printf("\tcPartitionData: %d\n", sizeof(cPartitionData));
  printf("cFileSector: %d\n", sizeof(FileSector_t));
  printf("\tcFileData: %d\n", sizeof(FileData_t));
  printf("\t\tcFilePartInfo: %d\n", sizeof(FilePartInfo_t));
  printf("PBS: %d\n", sizeof(PBS_t));    
  printf("cPartitionInfoSector: %d\n", sizeof(PartitionInfoSector_t));           
//  printf("cDirData: %d\n", sizeof(cDirData));    
//  printf("cDirData: %d\n", sizeof(cDirData));    
//  printf("cDirData: %d\n", sizeof(cDirData));    
//  printf("cDirData: %d\n", sizeof(cDirData));    
//  printf("cDirData: %d\n", sizeof(cDirData));    
//  printf("cDirData: %d\n", sizeof(cDirData));   
};
