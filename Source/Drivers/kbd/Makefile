#makefile drivers

include $(INC_FILE)

BUILD_DIR = $(BUILD_ROOT)/drivers/kbd

kbd_SRC = kbd_drv.o 

all: bin-dir kbd.drv

bin-dir:
	mkdir -p $(BUILD_DIR)

kbd.drv: $(kbd_SRC)
	i586-elf-ld $(LDFLAGS) -o $(BUILD_DIR)/kbd.drv $(addprefix ${BUILD_DIR}/, ${kbd_SRC}) $(SHARED_LIB)
	
%.o: %.c
	${CC} ${CFLAGS} -c $*.c -o $(BUILD_DIR)/$*.o
	
clean:
	rm -f $(BUILD_DIR)/*.drv $(BUILD_DIR)/*.o
