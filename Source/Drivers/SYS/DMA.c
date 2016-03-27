#include "DMA.h"
#include <string.h>
#include <Supernova/DrvCommands.h>

DMAInfo_t DMATable[8];

enum {
	DMA_BUFFER_SIZE		= 64 * 1024,
};

enum {
	DMA_REGISTER_8BIT_STATUS		= 0x08,
	DMA_REGISTER_8BIT_COMMAND		= 0x08,
	DMA_REGISTER_8BIT_REQUEST		= 0x09,
	DMA_REGISTER_8BIT_SINGLE_MASK	= 0x0A,
	DMA_REGISTER_8BIT_MODE			= 0x0B,
	DMA_REGISTER_8BIT_FLIP_FLOP		= 0x0C,
	DMA_REGISTER_8BIT_INTERMEDIATE	= 0x0D,
	DMA_REGISTER_8BIT_MASTE_RESET	= 0x0D,
	DMA_REGISTER_8BIT_MASK_RESET	= 0x0E,
	DMA_REGISTER_8BIT_MULTICHANNEL	= 0x0F,

	DMA_REGISTER_16BIT_STATUS		= 0xD0,
	DMA_REGISTER_16BIT_COMMAND		= 0xD0,
	DMA_REGISTER_16BIT_REQUEST		= 0xD2,
	DMA_REGISTER_16BIT_SINGLE_MASK	= 0xD4,
	DMA_REGISTER_16BIT_MODE			= 0xD6,
	DMA_REGISTER_16BIT_FLIP_FLOP	= 0xD8,
	DMA_REGISTER_16BIT_INTERMEDIATE	= 0xDA,
	DMA_REGISTER_16BIT_MASTE_RESET	= 0xDA,
	DMA_REGISTER_16BIT_MASK_RESET	= 0xDC,
	DMA_REGISTER_16BIT_MULTICHANNEL	= 0xDE,
};
/*
 Channel no
0-3		4-7
IO Port	IO Port	Size	Read or Write	Function
0x00	0xC0	Word	W				Start Address Register channel 0/4 (unusable)
0x01	0xC2	Word	W				Count Register channel 0/4 (unusable)
0x02	0xC4	Word	W				Start Address Register channel 1/5
0x03	0xC6	Word	W				Count Register channel 1/5
0x04	0xC8	Word	W				Start Address Register channel 2/6
0x05	0xCA	Word	W				Count Register channel 2/6
0x06	0xCC	Word	W				Start Address Register channel 3/7
0x07	0xCE	Word	W				Count Register channel 3/7

0x08	0xD0	Byte	R				Status Register
0x08	0xD0	Byte	W				Command Register
0x09	0xD2	Byte	W				Request Register
0x0A	0xD4	Byte	W				Single Channel Mask Register
0x0B	0xD6	Byte	W				Mode Register
0x0C	0xD8	Byte	W				Flip-Flop Reset Register
0x0D	0xDA	Byte	R				Intermediate Register
0x0D	0xDA	Byte	W				Master Reset Register
0x0E	0xDC	Byte	W				Mask Reset Register
0x0F	0xDE	Byte	RW				MultiChannel Mask Register (reading is undocumented, but it works!)

0x87	Channel 0 Page Address Register (unusable)
0x83	Channel 1 Page Address Register
0x81	Channel 2 Page Address Register
0x82	Channel 3 Page Address Register
0x8F	Channel 4 Page Address Register (unusable)
0x8B	Channel 5 Page Address Register
0x89	Channel 6 Page Address Register
0x8A	Channel 7 Page Address Register
*/

static uint32 DMA8bitSetTransfer(uint8 ch, uint32 mode, uint32 count, uint32 buff){
	--count; // -1 because of DMA counting
	// check that address is at most 24-bits (under 16MB)
	// check that count is at most 16-bits (DMA limit)
	// check that if we add count and address we don't get a carry
	// (DMA can't deal with such a carry, this is the 64k boundary limit)
	if((buff & 0xFF000000) || (count & 0xFFFF0000) || (((buff & 0xFFFF) + count) & 0xFFFF0000)){
		return DMA_EC_CROSS_64K;
	}
//	kprintf("DMASetTransfer %x %d\n", buff, count);
	mode &= 0xFC;						//mask DMA channel
	mode |= (ch & 0xF);		//set proper DMA channel
//	Breakpoint();
	outb(0x0a, 0x04 | ch);				// mask channel
	outb(0x0c, 0xff);					// reset flip-flop
	outb(0x04, ((buff >> 0 ) & 0xFF));	//  - address low byte
	outb(0x04, ((buff >> 8 ) & 0xFF));	//  - address high byte
	outb(0x81, ((buff >> 16) & 0xFF));	// external page register
	outb(0x0c, 0xff);					// reset flip-flop
	outb(0x05, ((count >> 0) & 0xFF));	//  - count low byte
	outb(0x05, ((count >> 8) & 0xFF));	//  - count high byte
	outb(0x0b, mode);					// set mode
	outb(0x0a, ch);						// unmask channel

	return 0;
}

uint32 DMA_Open(uint32 DevID, uint32 *Handle, const char *SubPath, uint32 Mode){
	if(!Handle)return ERRORCODE_WRONG_INPUT;
	if(*SubPath == '/')++SubPath;
	uint32 DMAch = *SubPath - '0';
	if(DMAch >= 8)return DMA_EC_DMA_NOT_AVAILABLE;
	DMAInfo_p dma = DMATable + DMAch;
	if(dma->Flags & (DMA_FLAG_UNAVILAIBLE | DMA_FLAG_IN_USE))return DMA_EC_DMA_NOT_AVAILABLE;
	Lowkmalloc(DMA_BUFFER_SIZE, &dma->VAddr, &dma->PhAddr);
	dma->Flags |= DMA_FLAG_IN_USE | DMA_FLAG_BUFFER_ALLOCATED | DMA_FLAG_READY;
	*Handle = DMAch;
	DevID = Mode = 0;	//ignore unused parameters
	return SUCCES;
}

uint32 DMA_Close(uint32 DevID, uint32 Handle){
	kprintf("dma: %s\n", __FUNCTION__);
	DevID = Handle = 0;
	return SUCCES;
}

uint32 DMA_Read(uint32 DevID, uint32 Handle, uint32 Src, void *Buffer, uint32 Count, uint32* Read){
//	kprintf("dma: %s\n", __FUNCTION__);
	if(Handle >= 8)return EC_WRONG_HANDLE;
	DMAInfo_p dma = DMATable + Handle;
	if(!(dma->Flags & DMA_FLAG_READY))return EC_WRONG_HANDLE;
	char *cb = (char*)Buffer;
	if(Src > 0x10000) return DMA_EC_CROSS_64K;
//	kprintf("DMA read %x %x %d\n", Buffer, Src, Count);
	memcpy(cb + Src, (void*)dma->VAddr, Count);
	*Read = Count;
	DevID = 0;
	return SUCCES;
}

uint32 DMA_Write(uint32 DevID, uint32 Handle, uint32 Src, void *Buffer, uint32 Count, uint32* Written){
	kprintf("dma: %s\n", __FUNCTION__);
	DevID = Handle = Count = Src = 0;
	Buffer = 0;
	*Written = 512;
	return SUCCES;
}

uint32 DMA_Seek(uint32 DevID, uint32 Handle, uint32 offset, uint32 method, uint32 *Pos){
//	kprintf("dma: %s\n", __FUNCTION__);
	DevID = Handle = 0;

	switch(method){
	case SEEK_CURRENT:
		*Pos += offset;
		break;
	case SEEK_END:
		*Pos = DMA_BUFFER_SIZE - offset;
		break;
	case SEEK_BEGINING:
		*Pos = offset;
		break;
	default:
		return DEV_EC_SEEK;
	}
	if(*Pos > DMA_BUFFER_SIZE) *Pos %= DMA_BUFFER_SIZE;

	return SUCCES;
}

uint32 DMA_Command(uint32 DevID, uint32 Handle, uint32 cmd, uint32 hparam, uint32 lparam){
//	kprintf("dma: %s %x %x %x %x\n", __FUNCTION__, Handle, cmd, hparam, lparam);
	switch (cmd){
	case DMA_COMMAND_SET_TRANSFER:{
		if(Handle >= 8)return EC_WRONG_HANDLE;
		DMAInfo_p dma = DMATable + Handle;
		if(!(dma->Flags & DMA_FLAG_READY))return EC_WRONG_HANDLE;

		if(dma->Flags & DMA_FLAG_16BIT){
			return EC_WRONG_HANDLE;
		} else {
			return DMA8bitSetTransfer(Handle, hparam, lparam, dma->PhAddr);
		}
	}
	}
	DevID = 0;
	return SUCCES;
}

uint32 DMA_init(){
	memset(DMATable, 0x0, 8 * sizeof(DMAInfo_t));

	DMATable[0].Flags |= DMA_FLAG_UNAVILAIBLE;
	DMATable[4].Flags |= DMA_FLAG_UNAVILAIBLE;

	uint32 i;
	for(i = 4; i < 7; ++i)
		DMATable[i].Flags |= DMA_FLAG_16BIT;

	return SUCCES;
}
