#ifndef _INCLUDE_SUPERNOVA_DI_H
#define _INCLUDE_SUPERNOVA_DI_H

#include <sysdef.h>
#include <inlines.h>
#include <Supernova/DrvCommands.h>

typedef uint32(*DriverEntry_p)(uint32 DriverID);
typedef uint32(*IRQHandler_p)(uint32 IRQno);

typedef struct DriverFunctions_s {
	uint32(*Initialize)();
	uint32(*Finalize)();
	uint32(*Reset)();
	uint32(*Scan)();
	uint32(*Command)(uint32 cmd, uint32 hparam, uint32 lparam);
}DriverFunctions_t, *DriverFunctions_p;


typedef struct DeviceInfo_s *DeviceInfo_p;
typedef struct DeviceFunction_s {
	uint32(*Open)(uint32 DevID, uint32 *Handle, const char *SubPath, uint32 OpenMode);
	uint32(*Close)(uint32 DevID, uint32 Handle);
	uint32(*Read)(uint32 DevID, uint32 Handle, uint32 Src, void *Buffer, uint32 Count, uint32* Read);
	uint32(*Write)(uint32 DevID, uint32 Handle, uint32 Src, void *Buffer, uint32 Count, uint32* Written);
	uint32(*Seek)(uint32 DevID, uint32 Handle, uint32 offset, uint32 method, uint32* NewPos);

	uint32(*Command)(uint32 DevID, uint32 Handle, uint32 cmd, uint32 hparam, uint32 lparam);
}DeviceFunction_t, *DeviceFunction_p;

typedef struct DeviceInfo_s {
	uint32 DevID;
	const char *Name;
	uint32 DevType;
	uint32 State;
	DeviceFunction_t Functions;
} DeviceInfo_t;


typedef struct VolumeInfo_s *VolumeInfo_p;
typedef struct VolumeFunctions_s {
	uint32(*OpenFile)(VolumeInfo_p vol, const char* path, uint32 *Handle, uint32 Mode);
	uint32(*CloseFile)(VolumeInfo_p vol, uint32 Handle);
	uint32(*ReadFile)(VolumeInfo_p vol, uint32 Handle, uint32 fSrc, void *destBuf, uint32 Count, uint32 *Read);
	uint32(*WriteFile)(VolumeInfo_p vol, uint32 Handle, uint32 fDest, void *SrcBuf, uint32 Count, uint32 *Written);
	uint32(*SeekFile)(VolumeInfo_p vol, uint32 Handle, uint32 Pos, uint32 Mode);
	uint32(*GetFileSize)(VolumeInfo_p vol, uint32 Handle);

	//deletefile / deletedir
	//createdir

	uint32(*Command)(VolumeInfo_p vol, uint32 cmd, uint32 hparam, uint32 lparam);
}VolumeFunctions_t, *VolumeFunctions_p;

typedef struct VolumeInfo_s {
	uint32 DriverID;
	uint32 DeviceID;
	uint32 VolumeID;
	uint32 State;
	void* VolumePointer;
	VolumeFunctions_t Functions;
}VolumeInfo_t;


enum{
	DEV_STATE_UNKNOWN,
	DEV_STATE_NOTPRESENT,
	DEV_STATE_READY,
	DEV_STATE_BUSY,
};

uint32 RegisterDriver(uint32 DrvID, DriverFunctions_p DrvFun, const char* Name, uint32 DrvInfo);
uint32 RegisterDevice(uint32 DriverID, DeviceInfo_p DevInfo);
uint32 RegisterVolume(uint32 DriverID, VolumeInfo_p VolInfo, const char *Name);
uint32 RegisterIRQ(uint32 IRQ, IRQHandler_p Handler);

void ReleaseLock(SPINLOCK *sp);
uint32 AcquireLock(SPINLOCK *sp);

void kprintf(const char*, ...);

uint32 Lowkmalloc(uint32 sz, uint32 *Vaddr, uint32 *PhAddr);
void Lowkfree(uint32 Vaddr, uint32 PhAddr);

void *kmalloc(uint32 sz);
void kfree(void *p);

HANDLE DirectOpen(uint32 DrvID, const char* Dev, uint32 Mode);
uint32 DirectRead(HANDLE handle, void* buffer, uint32 count, uint32 *read);
uint32 DirectWrite(HANDLE handle, void* buffer, uint32 count, uint32 *written);
uint32 DirectSeek(HANDLE handle, uint32 offset, uint32 method);
uint32 DirectCommand(HANDLE handle, uint32 command, uint32 hparam, uint32 lparam);

uint32 InternalMessage(uint32 DrvID, uint32 msg, uint32 hparam, uint32 lparam);
uint32 InternalGetMessage(message_p msg);

uint32 GetDeviceName(uint32 DevID, char *NameBuf);

#define TellFunc {kprintf("%s[%3d]: %s\n", __FILE__, __LINE__, __FUNCTION__);}

enum {
	DEVICE_UNKNOWN,

	DEVICE_REMOVABLE_DRIVE,

	DEVICE_FDD_CONTROLLER,
	DEVICE_FDD_DRIVE = DEVICE_REMOVABLE_DRIVE,

	DEVICE_DMA,
};

enum {
	DRIVER_INFO_DEVICE			= (1 <<  8),
	DRIVER_INFO_FILE_SYSTEM		= (1 <<  9),
};

#endif
