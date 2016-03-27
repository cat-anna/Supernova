#ifndef DMA_H_
#define DMA_H_

#include <Supernova.h>
#include <Supernova/di.h>

enum {
	DMA_FLAG_UNAVILAIBLE		= (1 << 0),
	DMA_FLAG_16BIT				= (1 << 1),
	DMA_FLAG_READY				= (1 << 2),

	DMA_FLAG_IN_USE				= (1 << 8),
	DMA_FLAG_BUFFER_ALLOCATED	= (1 << 9),
};

typedef struct DMAInfo_s {
	unsigned VAddr;
	unsigned PhAddr;
	unsigned Flags;
} DMAInfo_t, *DMAInfo_p;

uint32 DMA_init(void);

uint32 DMA_Open(uint32 DevID, uint32 *Handle, const char *SubPath, uint32 Mode);
uint32 DMA_Close(uint32 DevID, uint32 handle);
uint32 DMA_Read(uint32 DevID, uint32 Handle, uint32 Src, void *Buffer, uint32 Count, uint32* Read);
uint32 DMA_Write(uint32 DevID, uint32 Handle, uint32 Src, void *Buffer, uint32 Count, uint32* Written);
uint32 DMA_Seek(uint32 DevID, uint32 Handle, uint32 offset, uint32 method, uint32 *NewPos);
uint32 DMA_Command(uint32 DevID, uint32 Handle, uint32 cmd, uint32 hparam, uint32 lparam);

//HANDLE DMA_OpenChannel(uint32 channel);
//uint32 DMA_SetTransfer(HANDLE h, uint32 mode, uint32 count);
//uint32 DMA_ReadBuffer(HANDLE h_DMA, uint32 dest, uint32 count);
//uint32 DMA_WriteBuffer(HANDLE h_DMA, uint32 src, uint32 count);
//void DMA_FreeBuffer(uint32 DMABufChannel);

#endif /* DMA_H_ */
