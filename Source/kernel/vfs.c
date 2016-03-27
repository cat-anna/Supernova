#include "kernel.h"

VFSNode_t VFSRoot;
const char VFSRoot_name[] = "root";

uint32 VFS_init(){
	memset(&VFSRoot, 0, sizeof(VFSNode_t));
	VFSRoot.Name = VFSRoot_name;
	VFSRoot.flags = FILE_FLAG_DIR | VFS_FLAG_SYSTEM;
	VFSRoot.date = SFS_BuildDateTime;
	VFSRoot.type = VFS_TYPE_FOLDER;

	return SUCCES;
}

void VFS_InsertChildren(VFSNode_p node, VFSNode_p parent){
	if(!node || !parent)return;
	if(!parent->children){
		parent->children = node;
		return;
	}
	VFSNode_p child = parent->children;
	while(child->next)child = child->next;
	child->next = node;
}

void ListVFSTree(VFSNode_p node, uint32 level){
	while(node){
		uint32 i = level;
		for(; i > 0;i--)putch('\t');
		kprintf("%s s:%d f:%x\n", node->Name, node->size, node->flags);
		if(node->flags & FILE_FLAG_DIR) ListVFSTree(node->children, level+1);
		node = node->next;
	}
}

VFSNode_p VFS_GetNodeByPath(VFSNode_p parent, const char* path, char ** PathParam){
	if(!parent || !path)return 0;
	VFSNode_p node = parent->children;

	char *next = strchr(path + 1, '/');
	if(!next) next = strchr(path, '\0');

	while(node){
		if(strncasecmp(node->Name, path + 1, next - path - 1) != 0){
			node = node->next;
		} else {
			parent = node;
			node = node->children;
			if(!*next)break;
			path = next;
			next = strchr(path + 1, '/');
			if(!next) next = strchr(path, '\0');
			if(path == next) break;
		}
	}
	if(PathParam)*PathParam = (char*)path;
	if(node)return node;
	else return parent;
}

//-------------------VOLUME-MANAGER-------------------------

uint32 RegisterVolume(const uint32 DriverID, const VolumeInfo_p VolInfo, const char *Name){
	driver_p drv = CheckDrvID(DriverID);
	if(!VolInfo || !drv) return ERRORCODE_FATAL;

	volume_t v;
	v.FunctionsDump = 0;
	TestFunction(VolInfo, OpenFile, &v, FUNCTION_OPEN);
	TestFunction(VolInfo, CloseFile, &v, FUNCTION_CLOSE);
	TestFunction(VolInfo, ReadFile, &v, FUNCTION_READ);
	TestFunction(VolInfo, WriteFile, &v, FUNCTION_WRITE);
	TestFunction(VolInfo, SeekFile, &v, FUNCTION_SEEK);
	TestFunction(VolInfo, Command, &v, FUNCTION_COMMAND);
	TestFunction(VolInfo, GetFileSize, &v, FUNCTION_GETSIZE);

	if(!TestFlags(v.FunctionsDump, FUNCTIONS_OPEN_CLOSE_READ_WRITE_SEEK))
		return ERRORCODE_FATAL;

	volume_p vmem = (volume_p)kmalloc(sizeof(volume_t));
	vmem->VolID = VolInfo->VolumeID;
	vmem->DrvID = DriverID;
	vmem->VolInfo = VolInfo;
	vmem->FunctionsDump = v.FunctionsDump;
	strncpy(vmem->name, Name, 32);

	VFSNode_p node = (VFSNode_p)kmalloc(sizeof(VFSNode_t));
	memset(node, 0, sizeof(VFSNode_t));
	node->Name = vmem->name;
	node->flags = VFS_FLAG_SYSTEM;
	node->type = VFS_TYPE_VOLUME;
	node->owner_data = (uint32)vmem;
	node->owner_pid = (uint32)drv->proc;
	VFS_InsertChildren(node, &VFSVolume);

	return SUCCES;
}

uint32 GetVolumeByID(uint32 VolID, volume_t **Vol){
	if(!Vol)return ERRORCODE_WRONG_INPUT;
	VFSNode_p n = VFSVolume.children;
	while(n && n->type == VFS_TYPE_VOLUME){
		volume_p v = (volume_p)n->owner_data;
		if(v && v->VolID == VolID){
			*Vol = v;
			return SUCCES;
		}
		n = n->next;
	}
	return ERRORCODE_FATAL;
}

VFSNode_t VFSVolume;
const char VFSVolume_name[] = "vol";

uint32 VolMgr_init(){
	memset(&VFSVolume, 0, sizeof(VFSNode_t));
	VFSVolume.Name = VFSVolume_name;
	VFSVolume.flags = FILE_FLAG_DIR | VFS_FLAG_SYSTEM;
	VFSVolume.date = SFS_BuildDateTime;
	VFSVolume.type = VFS_TYPE_FOLDER;
	VFS_InsertChildren(&VFSVolume, &VFSRoot);

	return SUCCES;
}

//handles management

HANDLE HandleOpen(const char *path, uint32 Mode){
	if(!path)return INVALID_HANDLE_VALUE;

	char *Param;
	VFSNode_p node;

	if(*path != '/'){
		char *colon = strchr(path, ':');
		if(!colon)return INVALID_HANDLE_VALUE;
		char VolName[50] = "/";
		strncpy(VolName+1, path, colon - path);
		node = VFS_GetNodeByPath(&VFSVolume, VolName, 0);
		Param = colon + 1;
	} else {
		node = VFS_GetNodeByPath(&VFSRoot, path, &Param);
	}

	if(!node)return INVALID_HANDLE_VALUE;

	process_p proc = CurrentThread->OwnerProcess;
	handle_p h = GetFreeHandleEntry(proc);
	uint32 HandleParam = 0;
	if(!h)return INVALID_HANDLE_VALUE;

	switch (node->type){
	case VFS_TYPE_VOLUME:{
		volume_p vol = (volume_p)node->owner_data;
		if(!vol)return INVALID_HANDLE_VALUE;

		h->flags |= vol->FunctionsDump & FUNCTIONS_MASK;
		uint32 r;
		r = vol->VolInfo->Functions.OpenFile(vol->VolInfo, Param, &HandleParam, Mode);
		if(r != SUCCES){
			h->flags = 0;
			return INVALID_HANDLE_VALUE;
		}

		h->type = HANDLE_TYPE_FILE;
		h->data.volume = vol;
		break;
	}
	default:
		return INVALID_HANDLE_VALUE;
	}

//	device_p dev = (device_p)node->owner_data;
//	if(!dev->Functions.Open)return INVALID_HANDLE_VALUE;

	h->position = 0;
	h->param = HandleParam;
	return h;
}

uint32 HandleClose(HANDLE H){
	handle_p h = CheckHandle(H, HANDLE_TYPE_UNKNOWN, FUNCTION_CLOSE);
	if(!h)return 0;

	switch(h->type){
	case HANDLE_TYPE_FILE:{
		volume_p vol = h->data.volume;
		vol->VolInfo->Functions.CloseFile(vol->VolInfo, h->param);
		break;
	}
	}
	h->flags = 0;
	return SUCCES;
}

uint32 HandleRead(HANDLE H, void *buf, uint32 count, uint32 *read){
	handle_p h = CheckHandle(H, HANDLE_TYPE_UNKNOWN, FUNCTION_READ);
	if(!h)return 0;

	switch(h->type){
	case HANDLE_TYPE_FILE:{
		volume_p vol = h->data.volume;
		return vol->VolInfo->Functions.ReadFile(vol->VolInfo, h->param, h->position, buf, count, read);
	}
	default:
		return 1;
	}
	return 1;
}

uint32 HandleGetSize(HANDLE H){
	handle_p h = CheckHandle(H, HANDLE_TYPE_UNKNOWN, FUNCTION_GETSIZE);
	if(!h)return 0;

	switch(h->type){
	case HANDLE_TYPE_FILE:{
		if(!(h->flags & FUNCTION_GETSIZE))return 0;
		volume_p vol = h->data.volume;
		return vol->VolInfo->Functions.GetFileSize(vol->VolInfo, h->param);
	}
	default:
		return 0;
	}
}

