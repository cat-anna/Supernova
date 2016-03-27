#ifndef VFS_H
#define VFS_H

#define EncodeDate(Year, Month, Day, Hour, Minute) \
	(Minute + (Hour << 6) + (Day << 11) + (Month << 16) + (Year << 20))

struct VFSNode_s;
typedef struct VFSNode_s VFSNode_t, *VFSNode_p;
struct VFSNode_s{
	const char *Name;
	uint32 flags;
	uint32 type;
	uint32 size;
	uint32 date;

	uint32 owner_data;
	uint32 owner_pid;//pid or procces_p, Be careful

	VFSNode_p next;
	VFSNode_p children;//tylko jeœli flaga VFS_FLAG_DIR
};

extern VFSNode_t VFSRoot;

void VFS_InsertChildren(VFSNode_p node, VFSNode_p parent);
VFSNode_p VFS_GetNodeByPath(VFSNode_p parent, const char* path, char ** PathParam);

void ListVFSTree(VFSNode_p node, uint32 level);

enum {
	VFS_FLAG_SYSTEM			= 0x00000100,//systemowy wêze³ VFS
	VFS_FLAG_VIRTUAL		= 0x00000200,//nie da siê read i write

	VFS_TYPE_UNKNOWN		 = 0,
	VFS_TYPE_FOLDER,
	VFS_TYPE_DEVICE,
	VFS_TYPE_DRIVER,
	VFS_TYPE_VOLUME,
};

//-------------------VOLUME-MANAGER-------------------------

typedef struct volume_s {
	char name[32];
	uint32 VolID;
	uint32 DrvID;
	uint32 FunctionsDump;
	VolumeInfo_p VolInfo;
} volume_t, *volume_p;

extern VFSNode_t VFSVolume;

uint32 GetVolumeByID(uint32 VolID, volume_t **Vol);
uint32 RegisterVolume(const uint32 DriverID, const VolumeInfo_p VolInfo, const char *Name);

//handles management

HANDLE HandleOpen(const char *path, uint32 Mode);
uint32 HandleClose(HANDLE H);

uint32 HandleRead(HANDLE H, void *buf, uint32 count, uint32 *read);

uint32 HandleGetSize(HANDLE H);

#endif
