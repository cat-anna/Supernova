#Supernova master makefile

SUPERNOVA_TARGET=
#i586-pc-supernova

SUPERNOVA_ROOT = /cygdrive/d/Programowanie/Projekty/OS/Supernova
#/home/Admin/Supernova
SUPERNOVA_ROOT_WIN = d\:/Programowanie/Projekty/OS/Supernova
#d:/!prog/cygwin/home/Admin/Supernova

SRC_ROOT = $(SUPERNOVA_ROOT)/Source
SRC_ROOT_WIN = $(SUPERNOVA_ROOT_WIN)/Source

BIN_ROOT =  $(SUPERNOVA_ROOT)/Binary
BIN_ROOT_WIN = $(SUPERNOVA_ROOT_WIN)/Binary

SSM_ROOT = $(SUPERNOVA_ROOT)/Tools
INC_FILE = $(SRC_ROOT)/makefile.mk

SSM = $(SSM_ROOT)/ssm.exe
VER = $(SSM_ROOT)/version.exe

export 	INC_FILE		SSM_ROOT			\
		SUPERNOVA_ROOT	SUPERNOVA_ROOT_WIN 	\
		BIN_ROOT		BIN_ROOT_WIN		\
		SRC_ROOT		SRC_ROOT_WIN		\
		SSM				VER					\
		SUPERNOVA_TARGET
		
export SHARED = $(BIN_ROOT)/Shared/Supernova.lib 

all:
	mkdir -p $(BIN_ROOT)
	make -C Shared all
	make -C SNLoader all
	make -C Drivers all
	make -C kernel all
	make -C Apps all
	
	$(SSM) -do "$(SRC_ROOT)/Supernova.lua:BuildImage"
	
clean:
	make -C SNLoader clean
	make -C Shared clean
	make -C Apps clean
	make -C Drivers clean
	make -C kernel clean
