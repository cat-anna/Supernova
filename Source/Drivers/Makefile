#makefile root drivers

export CFLAGS+=-fpic
export LDFLAGS+=-e DrvEntry $(SHARED) -pie $(BIN_ROOT)/drivers/di.lib

all: 
	make -f di.mk all
	make -C SYS all
	make -C FDD all
	make -C FAT all

clean:
	make -f di.mk clean
	make -C FDD clean
	make -C FAT clean
	make -C SYS clean
