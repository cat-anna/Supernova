#makefile kernela

THIS			= kernel
OUT_FILE		= Supernova.elf
VERSION_HEADER	= $(SRC_ROOT)/kernel/headers/version.h
VERSION_NAME	= "SupernovaKernel"

ASM_SRC=	start.obj		\
			interrupts.obj

C_SRC=		kernel.o		\
			interrupts.o	\
			task.o			\
			console.o 		\
			memmgr.o 		\
			vfs.o 			\
			time.o 			\
			int0x80.o 		\
			libmgr.o		\
			elf.o			\
			DeviceMgr.o		\
			cpu.o			\
			ipc.o			\
			klibc.o

CFLAGS=-Werror 
LDFLAGS=-n -T kernel.ld
		
include $(INC_FILE)
