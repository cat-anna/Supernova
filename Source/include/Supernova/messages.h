#ifndef _SUPERNOVA_MESSAGES_H
#define _SUPERNOVA_MESSAGES_H

enum{
	MSG_EXIT			= 0,

	MSG_NEW_DEVICE		= 0x10,
};

typedef struct message_s {
	void* handle;		//pointer to the message handler function
	uint32 sender;
	uint32 time;
	uint32 message;
	uint32 lparam;
	uint32 hparam;
	uint32 reserved1;
	uint32 reserved2;
} message_t, *message_p;

#endif
