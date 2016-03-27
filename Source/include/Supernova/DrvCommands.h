#ifndef _DRIVER_COMMANDS_H_
#define _DRIVER_COMMANDS_H_

#ifndef _BUILDING_SSM_
#include <Supernova/ErrorCodes.h>
#endif

//device commands

enum {
	COMMAND_DEVICE_UNKNOWN				= 0,
};

//File system commands

enum{
	COMMAND_FILESYSTEM_UNKNOWN			= 0,
	COMMAND_FILESYSTEM_DETECT,

	EC_FILESYSTEM_UNKNOWN				= EC_DRIVER_DEPEND_UNKNOWN,
	EC_FILESYSTEM_UNKNOWN_FILESYSTEM,
};

//-------------DMA--------------------------------------------
enum {
	DMA_COMMAND_SET_TRANSFER		= 1,

	DMA_EC_UNKNOWN					= EC_DRIVER_DEPEND_UNKNOWN,
	DMA_EC_DMA_NOT_AVAILABLE,
	DMA_EC_CROSS_64K,

};

//-------------FDD-------------------------------------------
enum {
	FDD_EC_UNKNOWN					= EC_DRIVER_DEPEND_UNKNOWN,
	FDD_EC_FATAL,
	FDD_EC_PROCESS_SECTORS_FAILED,
	FDD_EC_DMA_NOT_AVAILABLE,
	FDD_EC_SEEK,
	FDD_EC_CALIBRATE,
	FDD_EC_COMMAND,
};

//-------------FAT-------------------------------------------
enum {
	FAT_EC_UNKNOWN					= EC_DRIVER_DEPEND_UNKNOWN,
	FAT_READ_ROOT_ERROR,
	FAT_READ_FAT_ERROR,

	FAT_MSG_UNKNOWN				= 0,
	FAT_MSG_INIT_VOLUME,
};


#endif
