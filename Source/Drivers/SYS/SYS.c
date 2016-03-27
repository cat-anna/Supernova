#include <Supernova.h>
#include <Supernova/di.h>
#include <string.h>

#include "DMA.h"

uint32 DriverID;
DeviceInfo_t DMADev;

const char DMA_Name[] = "DMA";

unsigned SYS_Initialize(){
	memset(&DMADev, 0, sizeof(DeviceInfo_t));
	uint32 res = DMA_init();
	if(res == SUCCES){
		DMADev.DevType = DEVICE_DMA;
		DMADev.Name = DMA_Name;
		DMADev.State = DEV_STATE_READY;

		DMADev.Functions.Open = DMA_Open;
		DMADev.Functions.Close = DMA_Close;
		DMADev.Functions.Read = DMA_Read;
		DMADev.Functions.Write = DMA_Write;
		DMADev.Functions.Seek = DMA_Seek;
		DMADev.Functions.Command = DMA_Command;

		RegisterDevice(DriverID, &DMADev);

	} else {
		DMADev.State = res;
	}
	return SUCCES;
}

unsigned SYS_Scan(){
	return SUCCES;
}

volatile unsigned Drv_state;

uint32 DrvEntry(uint32 DrvID){
	DriverID = DrvID;

	DriverFunctions_t DrvFun;
	memset(&DrvFun, 0, sizeof(DriverFunctions_t));
	DrvFun.Initialize = &SYS_Initialize;
	DrvFun.Scan = &SYS_Scan;
	RegisterDriver(DrvID, &DrvFun, "SYS", DRIVER_INFO_DEVICE);

	Drv_state = 1;
	while(Drv_state > 0){
		Sleep(1000);
	}
	return 0;
}
