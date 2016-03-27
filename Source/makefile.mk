#Supernova makefile include
.SUFFIXES: .asm;

SRC_DIR = $(SRC_ROOT)/$(THIS)
BIN_DIR = $(BIN_ROOT)/$(THIS)
BIN_DIR_WIN = $(BIN_DIR_WINBIN_ROOT_WIN)/$(THIS)

LDFLAGS+=-lc
NASMFLAGS=-f elf
CFLAGS+=-Wall -Wextra -O2 -fomit-frame-pointer -I. -fomit-frame-pointer -fno-builtin -fno-leading-underscore \
			-I$(SRC_ROOT)/include -Wno-unused-but-set-parameter

CC=$(SUPERNOVA_TARGET)-gcc
LD=$(SUPERNOVA_TARGET)-ld
AS=$(SUPERNOVA_TARGET)-as
ELFEDIT=$(SUPERNOVA_TARGET)-elfedit
NASM=nasm

all: bindir $(addprefix ${BIN_DIR}/, ${OUT_FILES}) $(addprefix ${BIN_DIR}/, ${OUT_FILE})

IN_SRC=$(addprefix ${BIN_DIR}/, ${ASM_SRC}) $(addprefix ${BIN_DIR}/, ${C_SRC})
ifdef IN_OBJS
IN_SRC+=$(addprefix ${BIN_DIR}/, ${IN_OBJS})
endif
${BIN_DIR}/${OUT_FILE}: $(IN_SRC)
ifdef VERSION_HEADER
	$(VER) -inc_build -add_date -add_time -s $(SRC_ROOT)/Supernova.ini -d $(VERSION_HEADER) -name $(VERSION_NAME)
endif
	$(LD) -o ${BIN_DIR}/${OUT_FILE} $(IN_SRC) $(LDFLAGS) -Map ${BIN_DIR}/${OUT_FILE}.map

bindir:
	mkdir -p $(BIN_DIR)
	
$(BIN_DIR)/%.obj: %.asm
	$(NASM) $*.asm -o $(BIN_DIR)/$*.obj $(NASMFLAGS)

$(BIN_DIR)/%.o: %.c
	${CC} ${CFLAGS} -c $*.c -o $(BIN_DIR)/$*.o
	
$(BIN_DIR)/%.out: %.S
	${AS} $*.S -o $(BIN_DIR)/$*.out
	
$(BIN_DIR)/%.elf: %.c
	$(CC) $(CFLAGS) $(SHARED) -o $(BIN_DIR)/$*.elf $*.c 
	
clean:
	rm -f $(BIN_DIR)/*.*
	