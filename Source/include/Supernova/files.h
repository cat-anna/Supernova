#ifndef _SUPERNOVA_FILES_H_
#define _SUPERNOVA_FILES_H_

#define FILE_NAME_LENGTH 18

enum{
	FILE_FLAG_READ_ONLY 	= 0x00000002,
	FILE_FLAG_HIDDEN 		= 0x00000004,
	FILE_FLAG_DELETED		= 0x00000008,	
	FILE_FLAG_SYSTEM		= 0x00000010,
	FILE_FLAG_DIR 			= 0x00000020,
};

enum{
	ACCES_READ 		= 0x01,
	ACCES_WRITE 	= 0x02,
	ACCES_FULL	 	= ACCES_WRITE | ACCES_READ,

	FILE_OPEN				= 0x10,
	FILE_CREATE				= 0x20,
	FILE_ALLWAYS			= 0x40,
	FILE_EXISTING			= 0x80,
	
	OPEN_EXISTING			= FILE_EXISTING,

	FILE_OPEN_ALLWAYS		= FILE_OPEN | FILE_ALLWAYS,
	FILE_OPEN_EXISTING		= FILE_OPEN | FILE_EXISTING,
	FILE_CREATE_ALLWAYS		= FILE_CREATE | FILE_ALLWAYS,
	FILE_CREATE_NEW			= FILE_CREATE | FILE_EXISTING,

//seek methods
	SEEK_BEGINING = 0,
	SEEK_CURRENT,
	SEEK_END,
};

enum{
	Date_Minutes 	= 0x3f,
	Date_Hour 		= 0x7c0,
	Date_Day 		= 0xf800,
	Date_Month 		= 0xf0000,
	Date_Year 		= 0xfff00000,
};
/*
typedef uint32 FileTime;

static inline FileTime EncodeDate(uint32 Year, uint32 Month, uint32 Day, uint32 Hour, uint32 Minute){
	return Minute + (Hour << 6)	+ (Day << 11) + (Month << 16) + (Year << 20);
};

typedef struct {
	char name[FILE_NAME_LENGTH];
	uint32 size;
	FileTime date;
	uint32 flags;
} SearchRec_t, *SearchRec_p;*/

#endif
