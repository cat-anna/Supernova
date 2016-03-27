#include "fdd.h"
#include "FDD_driver.h"
#include <headers/BDA.h>

static void FDD_Check_IRQ(FDD_p Dev, uint32 *st0, uint32 *cyl);
static void FDD_write_cmd(FDD_p Dev, uint8 cmd);
static uint8 FDD_read_data(FDD_p Dev);
static uint32 FDD_Calibrate(FDD_p Dev);
static uint32 FDD_Seek(FDD_p Dev, uint8 cyli, uint8 head);

void FDD_motor(FDD_p Dev, uint32 onoff);

static inline void FDD_Error(FDD_p Dev, uint32 EC){	
	Dev->State |= FDD_ERROR;
	Dev->ErrorCode = EC;
}

unsigned char FDD_DMA_CMD[] = {0x46, 0x4a};

uint32 FDD_ProcessSectors(FDD_p Dev, CHS_p chs, uint8 SecCount, floppy_dir dir){
    // transfer command, set below

//	kprintf("FDD_ProcessSectors %d (%d:%d:%d)\n", SecCount, chs->c, chs->h, chs->s);
    uint8 cmd;
    // Read is MT:MF:SK:0:0:1:1:0, write MT:MF:0:0:1:0:1
    // where MT = multitrack, MF = MFM mode, SK = skip deleted
// Specify MFM mode
    static const uint8 flags = 0x40;    
    if(dir == FDD_WRITE)
    	cmd = CMD_WRITE_DATA | flags;
    else //dir == FDD_READ
    	cmd = CMD_READ_DATA | flags;

    // seek both heads
	FDD_motor(Dev, 1);
    if(FDD_Seek(Dev, chs->c, 0)) return FDD_EC_SEEK;
    if(FDD_Seek(Dev, chs->c, 1)) return FDD_EC_SEEK;

	uint32 i = 0;
    while(++i < 10){
//    	kprintf("FDD_ProcessSectors %d %d\n", i, SecCount<<9);
  //  	Breakpoint();
        if(DMASetTransfer(FDD_DMA_CMD[dir], SecCount << 9) != SUCCES)continue;
	    Sleep(10); // give some time (10ms) to settle after the seeks
        FDD_write_cmd(Dev, cmd);  // set above for current direction
        FDD_write_cmd(Dev, chs->h << 2);    // 0:0:0:0:0:HD:US1:US0 = head and drive
        FDD_write_cmd(Dev, chs->c);    // cylinder
        FDD_write_cmd(Dev, chs->h);    // first head (should match with above)
        FDD_write_cmd(Dev, chs->s);    // first sector, strangely counts from 1
        FDD_write_cmd(Dev, 2);    // bytes/sector, 128*2^x (x=2 -> 512)
        FDD_write_cmd(Dev, 18);   // number of tracks to operate on
        FDD_write_cmd(Dev, 0x1b); // GAP3 length, 27 is default for 3.5"
        FDD_write_cmd(Dev, 0xff); // data length (0xff if B/S != 0)
        FDD_WaitForIRQ();
    //    unsigned char st0 = 0, st1 = 0, st2 = 0//, rcy = 0, rhe = 0, rse = 0*//*, bps = 0;
    // first read status information
        unsigned char st0 = FDD_read_data(Dev);
        unsigned char st1 = FDD_read_data(Dev);
        unsigned char st2 = FDD_read_data(Dev);
	// These are cylinder/head/sector values, updated with some
	// rather bizarre logic, that I would like to understand.
       /* unsigned char rcy = */FDD_read_data(Dev);
       /* unsigned char rhe = */FDD_read_data(Dev);
       /* unsigned char rse = */FDD_read_data(Dev);
	// bytes per sector, should be what we programmed in
     	unsigned char bps = FDD_read_data(Dev);

     	uint32 error = 0;
        if(st0 & 0xC0) error |= (st0 >> 6);//print("floppy_do_sector: status = { 0, "error", "invalid command", "drive not ready" };
        if(st1 & 0x80) error |= 0x0008;//print("floppy_do_sector: end of cylinder\n");
        if(st0 & 0x08) error |= 0x0010;//print("floppy_do_sector: drive not ready\n");
        if(st1 & 0x20) error |= 0x0020;//print("floppy_do_sector: CRC error\n");
        if(st1 & 0x10) error |= 0x0040;//print("floppy_do_sector: controller timeout\n");
        if(st1 & 0x04) error |= 0x0080;//print("floppy_do_sector: no data found\n");
        if((st1|st2) & 0x01) error |= 0x0100;//print("floppy_do_sector: no address mark found\n");
        if(st2 & 0x40) error |= 0x0200;//print("floppy_do_sector: deleted address mark\n");
        if(st2 & 0x20) error |= 0x0400;//print("floppy_do_sector: CRC error in data\n");
        if(st2 & 0x10) error |= 0x0800;//print("floppy_do_sector: wrong cylinder\n");
        if(st2 & 0x04) error |= 0x1000;//print("floppy_do_sector: uPD765 sector not found\n");
        if(st2 & 0x02) error |= 0x2000;//print("floppy_do_sector: bad cylinder\n");
        if(bps != 0x2) error |= 0x4000;//print("floppy_do_sector: wanted 512B/sector, got %d", (1<<(bps+7)));
        if(st1 & 0x02) error |= 0x8000;//print("floppy_do_sector: not writable\n");

        if(!error){
        	FDD_motor(Dev, 0);
        	return SUCCES;
        }
    }
    FDD_motor(Dev, 0);
    FDD_Error(Dev, FDD_EC_FATAL);
    return FDD_EC_FATAL;
}

// Seek for a given cylinder, with a given head
static uint32 FDD_Seek(FDD_p Dev, uint8 cyli, uint8 head){
	if(Dev->track == cyli)return SUCCES;
    uint32 i, st0 = 0, cyl = 30000; // set to bogus cylinder
    for(i = 0; i < 10; i++) {
        // Attempt to position to given cylinder
        // 1st byte bit[1:0] = drive, bit[2] = head
        // 2nd byte is cylinder number
        FDD_write_cmd(Dev, CMD_SEEK);
        FDD_write_cmd(Dev, head << 2);
        FDD_write_cmd(Dev, cyli);
        FDD_WaitForIRQ();
        FDD_Check_IRQ(Dev, &st0, &cyl);
        if(st0 & 0xC0) {
			FDD_Error(Dev, FDD_EC_SEEK);
            continue;
        }		
        if(cyl == cyli) {
			Dev->track = cyli;
            return SUCCES;
        }
    }
    return FDD_EC_SEEK;
}

void FDD_motor(FDD_p Dev, uint32 onoff){
    if(onoff) {
    //    if(Dev->State & FDD_STATE_MOTOR_TIMER){// need to turn on
   //         Dev->State &= ~FDD_STATE_MOTOR_TIMER;
	///		TimeKillEvent(h_Msg, (uint32)Dev);
   //     }else{// need to turn on
            outb(Dev->Base + FLOPPY_DOR, 0x1c);
            Sleep(20); // wait 200 ms = hopefully enough for modern drives
            Dev->State |= FDD_STATE_MOTOR;
	//	}
    } else {
    	/*if(!(Dev->State & FDD_STATE_MOTOR_TIMER)){
			TimeSetEvent(h_Msg,  (uint32)Dev, 2000);
			Dev->State |= FDD_STATE_MOTOR_TIMER;
		}*/
    }
}
/*
void FDD_TurnOffMotor(FDD_p Dev){
	if(!Dev)return;
	outb(Dev->Base + FLOPPY_DOR, 0x0c);
	Dev->State &= ~FDD_STATE_MOTOR;	
	Dev->State &= ~FDD_STATE_MOTOR_TIMER;
}*/

uint32 InitFDD(FDD_p Dev){
	//if not calibrated
	if(!(BiosDataAreaVirt->FDDCalibration & (1 << Dev->FDDNo))){
		//calibrate controller
		outb(Dev->Base + FLOPPY_DOR, 0x00); // disable controller
		outb(Dev->Base + FLOPPY_DOR, 0x0C); // enable controller
		FDD_WaitForIRQ();
		uint32 st0, cyl; // ignore these here..
		FDD_Check_IRQ(Dev, &st0, &cyl);
	}

	//set transfer speed 500kb/s
   outb(Dev->Base + FLOPPY_CCR, 0x00);
    //  - 1st byte is: bits[7:4] = steprate, bits[3:0] = head unload time
    //  - 2nd byte is: bits[7:1] = head load time, bit[0] = no-DMA
    //  steprate    = (8.0ms - entry*0.5ms)*(1MB/s / xfer_rate)
    //  head_unload = 8ms * entry * (1MB/s / xfer_rate), where entry 0 -> 16
    //  head_load   = 1ms * entry * (1MB/s / xfer_rate), where entry 0 -> 128
    FDD_write_cmd(Dev, CMD_SPECIFY);
    FDD_write_cmd(Dev, 0xdf); // steprate = 3ms, unload time = 240ms
    FDD_write_cmd(Dev, 0x02); // load time = 16ms, no-DMA = 0
// it could fail...
    if(FDD_Calibrate(Dev) == SUCCES){
    	Dev->State &= FDD_NOT_INITIALIZED | FDD_ERROR;
    	Dev->ErrorCode = 0;
    	return SUCCES;
    } else {
    	return ERRORCODE_FATAL;
    }
}

// Move to cylinder 0, which calibrates the drive..
static uint32 FDD_Calibrate(FDD_p Dev){
    uint32 i, st0, cyl = -1; // set to bogus cylinder
    FDD_motor(Dev, 1);
    for(i = 0; i < 10; i++) {
        // Attempt to positions head to cylinder 0
        FDD_write_cmd(Dev, CMD_RECALIBRATE);
        FDD_write_cmd(Dev, 0); // argument is drive, we only support 0
        FDD_WaitForIRQ();
		FDD_Check_IRQ(Dev, &st0, &cyl);
        if(st0 & 0xC0) {
            continue;
        }
        if(!cyl) { // found cylinder 0 ?
            FDD_motor(Dev, 0);
            return SUCCES;
        }
    }
	FDD_Error(Dev, FDD_EC_CALIBRATE);
	FDD_motor(Dev, 0);
	Dev = 0;
    return FDD_EC_CALIBRATE;
}

static void FDD_Check_IRQ(FDD_p Dev, uint32 *st0, uint32 *cyl){
    FDD_write_cmd(Dev, CMD_SENSE_INTERRUPT);
    *st0 = FDD_read_data(Dev);
    *cyl = FDD_read_data(Dev);
}

static void FDD_write_cmd(FDD_p Dev, uint8 cmd){
// do timeout, 6 seconds
	uint32 i;
    for(i = 0; i < 600; i++) {
        Sleep(10); // sleep 10 ms
        if(0x80 & inb(Dev->Base + FLOPPY_MSR)) {
			outb(Dev->Base + FLOPPY_FIFO, cmd);
            return;
        }
    }
	FDD_Error(Dev, FDD_EC_COMMAND);
}

static uint8 FDD_read_data(FDD_p Dev){
// do timeout, 6 seconds
	uint32 i;
    for(i = 0; i < 600; i++) {
        if(0x80 & inb(Dev->Base + FLOPPY_MSR))
			return inb(Dev->Base + FLOPPY_FIFO);
        Sleep(10); // sleep 10 ms
    }
	FDD_Error(Dev, FDD_EC_COMMAND);
    return 0;
}
