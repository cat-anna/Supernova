/*
	Supernova kernel shared library subfile
	This file can only by included by Supernova kernel library
	This file contain methods for handles
*/

HANDLE lib_handle_OpenVFS(char* fname){
	return DoInt0x80(SYSCALL_OPEN_VFS, (uint32)fname, 0, 0);
}

HANDLE lib_handle_OpenHandle(char* fname, uint32 mode){
	HANDLE h_vfs = lib_handle_OpenVFS(fname);
	if(h_vfs == INVALID_HANDLE_VALUE)return INVALID_HANDLE_VALUE;
	char* apath = strchr(fname, '/');

	device_request_t req;
	req.request = REQUEST_OPEN_HANDLE;
	req.Stack.OpenHandle.flags = mode;
	req.Stack.OpenHandle.path = (apath)?(apath):(fname);
	req.Stack.OpenHandle.path_size = strlen(req.Stack.OpenHandle.path);

	uint32 h_file;
	uint32 result = DeviceRequest(h_vfs, &req, &h_file);
	CloseHandle(h_vfs);
	if(result)return INVALID_HANDLE_VALUE;
	return (HANDLE)h_file;
}

uint32 lib_handle_ReadHandle(HANDLE H, void* buffer, uint32 count){
	device_request_t req;
	req.request = REQUEST_HANDLE_READ;
	req.Stack.Read.count = count;
	req.Stack.Read.dst = (uint32)buffer;

	uint32 size;
	uint32 result = DeviceRequest(H, &req, &size);
	if(result)return 0;
	return size;
}

uint32 lib_handle_GetHandleSize(HANDLE H){
	device_request_t req;
	req.request = REQUEST_HANDLE_GET_SIZE;
	req.Stack.h = H;
	uint32 size;
	uint32 result = DeviceRequest(H, &req, &size);
	if(result)return 0;
	return size;
}

/*
enum{
	HANDLE_MAGIC_FIND	= 0xFAED5619,
	HANDLE_MAGIC_FILE	= 0xFAFE2852,
};

typedef struct{
	uint32 magic;
	VolumeReg_t VolAcces;
	uint32 h;
}FileHandle_t, *FileHandle_p;

uint32 WriteFile(HANDLE H, void* buffer, uint32 count){
	FileHandle_p FH = (FileHandle_p)H;
	if(FH->magic != HANDLE_MAGIC_FILE)return 0;
	return FH->VolAcces.WriteFile(FH->VolAcces.VolumeID, FH->h, (uint32)buffer, count);
}

typedef struct{
	uint32 magic;
	VolumeReg_t VolAcces;
	uint32 h;
}FindHandle_t, *FindHandle_p;

HANDLE FindFirst(char* pat, SearchRec_p sr){
	if(!pat)return INVALID_HANDLE_VALUE;

	FindHandle_p FH = (FindHandle_p)malloc(sizeof(FindHandle_t));
	ZeroMemory(FH, sizeof(FindHandle_t));
	FH->magic = HANDLE_MAGIC_FIND;

	if(!GetVolumeFunctions(pat, &FH->VolAcces))return INVALID_HANDLE_VALUE;
	
	HANDLE VH = FH->VolAcces.FindFirst(FH->VolAcces.VolumeID, strchr(pat, ':') + 1, sr);
//	printf("LIB FF %x |%s|%s|\n", VH, pat, strchr(pat, ':') + 1);	
	if(VH == INVALID_HANDLE_VALUE){
		free(FH);
		return INVALID_HANDLE_VALUE;
	}
	FH->h = VH;
	return (uint32)FH;
}

uint32 FindNext(HANDLE H, SearchRec_p sr){
	FindHandle_p FH = (FindHandle_p)H;
	if(FH->magic != HANDLE_MAGIC_FIND)return 0;
	return FH->VolAcces.FindNext(FH->VolAcces.VolumeID, FH->h, sr);
}

void FindClose(HANDLE H){
	if(!H)return;
	FindHandle_p FH = (FindHandle_p)H;
	if(FH->magic != HANDLE_MAGIC_FIND){
	//	CloseHandle(H);
		return;
	}
	FH->VolAcces.FindClose(FH->VolAcces.VolumeID, FH->h);
	free(FH);
}*/
