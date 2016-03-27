#makefile drivers

include $(INC_FILE)

BUILD_DIR = $(BUILD_ROOT)/drivers/SFS

SFS_SRC = SFS.o SFS_drv.o 

all: bin-dir SFS.drv 

bin-dir:
	mkdir -p $(BUILD_DIR)

SFS.drv: $(SFS_SRC)
	i586-elf-ld $(LDFLAGS) -o $(BUILD_DIR)/SFS.drv $(addprefix ${BUILD_DIR}/, ${SFS_SRC}) $(SHARED_LIB)
	
%.o: %.c
	${CC} ${CFLAGS} -c $*.c -o $(BUILD_DIR)/$*.o
	
clean:
	rm -f $(BUILD_DIR)/*.drv $(BUILD_DIR)/*.o
