#ifndef _FDD_DRIVER_H_
#define _FDD_DRIVER_H_

#include <Supernova.h>
#include "Supernova/di.h"

extern HANDLE DMA_handle;

void FDD_WaitForIRQ();
uint32 DMASetTransfer(uint32 mode, uint32 count);

#endif
