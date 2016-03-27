#makefile drivers

include $(INC_FILE)

BUILD_DIR = $(BUILD_ROOT)/drivers/serial

serial_SRC = serial.o	

all: bin-dir serial.drv

bin-dir:
	mkdir -p $(BUILD_DIR)

serial.drv: $(serial_SRC)
	i586-elf-ld $(LDFLAGS) -o $(BUILD_DIR)/serial.drv $(addprefix ${BUILD_DIR}/, ${serial_SRC}) $(SHARED_LIB)
	
%.o: %.c
	${CC} ${CFLAGS} -c $*.c -o $(BUILD_DIR)/$*.o
	
clean:
	rm -f $(BUILD_DIR)/*.drv $(BUILD_DIR)/*.o
