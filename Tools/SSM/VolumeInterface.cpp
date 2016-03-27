#include "VolumeInterface.h"
#include <time.h>
#include <sys/stat.h>
#include <string.h>

//--------------------------cImage------------------------------------------
/*
class cImage
{

public:
	int ReadBlock(int BlockNo, int BlockCount, void *Buffer);
	int WriteBloc(int BlockNo, int BlockCount, void *Buffer);
};*/

cImage::cImage(){
	FImageSize = FBlockSize = FBlockCount = Ready = 0;
	FFileName = 0;

}

cImage::~cImage(){
	if(Ready){
		fclose(fptr);
	}
	Ready = 0;
}

int cImage::GetImageSize()
{
	return FImageSize;
}

bool cImage::IsReady()
{
	return Ready;
}

char const *cImage::GetFileName()
{
	return FFileName;
}

bool cImage::OpenFile(const char* FileName, int BlockSize){
	if(Ready)return false;

	FFileName = FileName;
	fptr = fopen(FFileName, "r+");
	if(fptr == NULL)return false;

	fseek(fptr, 0, SEEK_END);
	FImageSize = ftell(fptr);

	if(FImageSize == 0 || (FImageSize % BlockSize) > 0){
		fclose(fptr);
		return false;
	}

	FBlockSize = BlockSize;
	FBlockCount = FImageSize / FBlockSize;
	Ready = true;
}

bool cImage::CreateImage(const char* FileName, int BlocksCount, int BlockSize){
	if(Ready)return false;

	if(FileName)
		FFileName = FileName;

	fptr = fopen(FFileName, "w+");
	if(fptr == NULL)return false;

	FBlockSize = BlockSize;
	FBlockCount = BlocksCount;
	FImageSize = BlocksCount * BlockSize;

	char* buf = new char[FBlockSize];
	memset(buf, 0, FBlockSize);
	for(int i = 0; i < BlocksCount; ++i)
		fwrite(buf, FBlockSize, 1, fptr);
	delete buf;

	Ready = true;
	return true;
}

int cImage::ReadBlock(int BlockNo, int BlockCount, void *Buffer){
	if(BlockNo + BlockCount >= FBlockCount)return 0;
	fseek(fptr, BlockNo * FBlockSize, SEEK_SET);
	return fread(Buffer, FBlockSize, BlockCount, fptr);
}

int cImage::WriteBlock(int BlockNo, int BlockCount, void *Buffer){
	if(BlockNo + BlockCount >= FBlockCount)return 0;
	fseek(fptr, BlockNo * FBlockSize, SEEK_SET);
	return fwrite(Buffer, FBlockSize, BlockCount, fptr);
}

//----------------------VolumeInterface-------------------------------------

VolumeInterface::VolumeInterface(cImage *image)
{
    FLastError = 0;
    FImage = image;
}

VolumeInterface::~VolumeInterface()
{
    //dtor
}

void VolumeInterface::List(const char *path)
{
	GlobalSearchRecord sr;
	if(!path)path = "/*";
	FindFirst(sr, path);
	if(FLastError > 0){
		printf("'%s' not found\n", path);
		return;
	}

	printf("\nWolumin %s\n\n", Name());

	while(FindNext(sr), !FLastError)
		printf("%s\n", sr.Name);

	FindClose(sr);
}

time_t GetSysFileTime(const char * f_p){
	struct stat buf;
//	struct tm * timeinfo;
	if (stat(f_p, &buf))return 0;
//	timeinfo = localtime (&buf.st_mtime);
	return buf.st_mtime;
};



/*void SFS_Volume::List()
{
	unsigned ErrorCode = 0;
	if(!(ErrorCode = vol->FindFirst(pat, &sr)))
	{

		int fcount = 0, dcount = 0, fsize = 0;
		PrintDate(sr.CreationDateTime);
		if(sr.Attributes & FILE_FLAG_DIR)
		{
			printf("\t%10s", "<DIR>");
			dcount++;
		} else {
			printf("\t%10d", sr.Size);
			fcount++;
		}
		printf(" %s\n", sr.Name);
		fsize += sr.Size;
		while(!(ErrorCode = vol->FindNext(&sr)))
		{
			PrintDate(sr.CreationDateTime);
			if(sr.Attributes & FILE_FLAG_DIR)
			{
				printf("\t%10s", "<DIR>");
				dcount++;
			} else {
				printf("\t%10d", sr.Size);
				fcount++;
			}
			printf(" %s\n", sr.Name);
			fsize += sr.Size;
		}
		printf("\n%d plik(ow)\t%10d bajtow\n%d katalog(ow)\t%10d bajtow wolnych\n",
				fcount, fsize, dcount, vol->FreeSpace());
	}
	ErrorCode = vol->FindClose(&sr);*/
//	return ErrorCode;
