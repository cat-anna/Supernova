#ifndef DRVMGR_H
#define DRVMGR_H

#include <Supernova/di.h>

enum{
	FUNCTION_INITIALIZE		= (1 << 16),
	FUNCTION_FINALIZE		= (1 << 17),
	FUNCTION_RESET			= (1 << 18),
	FUNCTION_SCAN			= (1 << 19),
	FUNCTION_COMMAND		= (1 << 20),
	FUNCTION_OPEN			= (1 << 21),
	FUNCTION_CLOSE			= (1 << 22),
	FUNCTION_READ			= (1 << 23),
	FUNCTION_WRITE			= (1 << 24),
	FUNCTION_SEEK			= (1 << 25),
	FUNCTION_GETSIZE		= (1 << 26),

	FUNCTIONS_MASK			= 0xFFFF0000,

	FUNCTIONS_OPEN_CLOSE_READ_WRITE_SEEK		= FUNCTION_OPEN | FUNCTION_CLOSE |
												  FUNCTION_READ | FUNCTION_WRITE |
												  FUNCTION_SEEK,

	DRIVER_ENTRY_VALID		= (1 <<  0),

	DEVICE_ENTRY_VALID		= (1 <<  0),
};

#define CheckFunctions(A,B) ((A->FunctionsDump & (B)) == (B))

typedef struct driver_s{
	uint32 Flags;
	uint32 Info;
	char Name[DEFAULT_NAME_SIZE];
	process_p proc;
	DriverFunctions_t Functions;
	uint32 FunctionsDump;
} driver_t;

typedef struct device_s{
	uint32 Flags;
	char Name[DEFAULT_NAME_SIZE];
	driver_p Driver;
	DeviceFunction_t Functions;
	uint32 FunctionsDump;
	uint32 DevType;
	uint32 DevID;
} device_t;

uint32 InstallDriver(void *Data, const char *Name);
uint32 Supernova_DeviceManager(void);

#define DevMgrIPC ((void*)DEVMGR_IPC_LOCATION)
#define DriversTable ((driver_p)DRIVERS_TABLE_LOCATION)
#define DevicesTable ((device_p)DEVICES_TABLE_LOCATION)
#define TestFunction(A,B,C,D) {if((A)->Functions.B) (C)->FunctionsDump |= (D);}

static inline driver_p CheckDrvID(uint32 DrvID){
	if(DrvID >= DRIVERS_TABLE_COUNT)return 0;
	return DriversTable +  DrvID;
}

static inline device_p CheckDevID(uint32 DevID){
	if(DevID >= DEVICES_TABLE_SIZE)return 0;
	return DevicesTable +  DevID;
}

#endif 
