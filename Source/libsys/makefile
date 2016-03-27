.SUFFIXES: .asm
#makefile libsys

include $(INC_FILE)

BIN_DIR = $(BIN_ROOT)/libsys
  
all: bin-dir crt0.o

bin-dir:
	mkdir -p $(BIN_DIR)

crt0.o: crt0.c
	${CC} ${CFLAGS} -c $*.c -o $(BIN_DIR)/$*.o

%.o: %.c
	${CC} ${CFLAGS} -c $*.c -o $(BIN_DIR)/$*.o
	
%.obj: %.asm
	nasm $*.asm -o $(BIN_DIR)/$*.obj -f elf
	
clean:
	rm -f $(BIN_DIR)/crt0.o
	