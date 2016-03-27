#makefile libs

THIS=drivers

OUT_FILE=di.lib
C_SRC=di.o

LDFLAGS=-shared

include $(INC_FILE)
