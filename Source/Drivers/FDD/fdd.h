#ifndef FDD_H
#define FDD_H

#include <Supernova.h>
#include <Supernova/di.h>
#include <Supernova/DrvCommands.h>
#include <headers/chs.h>

#define FLOPPY_PRIMARY_BASE     0x03F0
#define FLOPPY_SECONDARY_BASE   0x0370

#define FDD_PARAMETER_ADDRESS   0x000fefc7

//static const char * calibrate_status[] = { 0, "error", "invalid", "drive" };
//            print("floppy_calibrate: status = %s\n", status[st0 >> 6]);

//char const *drive_type[] = {"no floppy drive","360kb 5.25in","1.2mb 5.25in","720kb 3.5in","1.44mb 3.5in","2.88mb 3.5in"};

//static const char * seek_status[] =
//            { "normal", "error", "invalid", "drive" };
//            print("floppy_seek: status = %s\n", status[st0 >> 6]);


enum {
	FDD_NOT_EXISTS			= 0x0001,
	FDD_NOT_INITIALIZED		= 0x0002,

	FDD_STATE_MOTOR		  	= 0x0010,
	FDD_STATE_MOTOR_TIMER 	= 0x0020,
	
	FDD_ERROR		   	  	= 0x0100,
};


typedef enum {
    FDD_READ 	= 0,
    FDD_WRITE 	= 1,
} floppy_dir;

typedef struct FDD_s {
	uint32 Type;
	uint32 FDDNo;
	uint32 Base;
	uint32 State;
	uint32 track;
	uint32 ErrorCode;
	DeviceInfo_t DevInfo;
	SPINLOCK lock;
	uint32 handle;
} FDD_t, *FDD_p;

typedef struct{
	uint8 steprate_headunload;
	uint8 headload_ndma;
	uint8 motor_delay_off; //specified in clock ticks
	uint8 bytes_per_sector;
	uint8 sectors_per_track;
	uint8 gap_length;
	uint8 data_length; //used only when bytes per sector == 0
	uint8 format_gap_length;
	uint8 filler;
	uint8 head_settle_time; //specified in milliseconds
	uint8 motor_start_time; //specified in 1/8 seconds
}__attribute__((packed)) floppy_parameters_t;

enum floppy_commands {
	CMD_SPECIFY = 3,            // SPECIFY
	CMD_WRITE_DATA = 5,         // WRITE DATA
	CMD_READ_DATA = 6,          // READ DATA
	CMD_RECALIBRATE = 7,        // RECALIBRATE
	CMD_SENSE_INTERRUPT = 8,    // SENSE INTERRUPT
	CMD_SEEK = 15,              // SEEK
};

enum floppy_registers {
	FLOPPY_DOR  = 2,  // digital output register
	FLOPPY_MSR  = 4,  // master status register, read only
	FLOPPY_FIFO = 5,  // data FIFO, in DMA operation for commands
	FLOPPY_CCR  = 7,  // configuration control register, write only
};

uint32 InitFDD(FDD_p Dev);

//void FDD_TurnOffMotor(FDD_p Dev);
uint32 FDD_ProcessSectors(FDD_p Dev, CHS_p chs, uint8 SecCount, floppy_dir dir);

#endif 
