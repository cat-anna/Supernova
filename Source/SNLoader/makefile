#Supernova loader makefile

THIS=SNLoader
OUT_FILES=bootsector_FAT.bin SNLoader.bin	

include $(INC_FILE) 	

$(BIN_DIR)/bootsector_FAT.bin: bootsector_FAT.asm 
	$(NASM) -o $(BIN_DIR)/bootsector_FAT.bin -f bin bootsector_FAT.asm 
	
$(BIN_DIR)/loader16.obj: loader16.asm SFS.asm FAT.asm ReadSector.asm IA-32.asm String16.asm 
	$(NASM) -o $(BIN_DIR)/loader16.obj -f elf loader16.asm 
	
LOADER_FILES = loader16.obj			\
			   loader32.obj 		\
			   loader32.o
LOADER_FILES_PATH = $(addprefix ${BIN_DIR}/, ${LOADER_FILES})
	
$(BIN_DIR)/SNLoader.bin: $(LOADER_FILES_PATH)
	$(LD) -Map $(BIN_DIR)/SNLoader.map -n -T SNLoader.ld -o $(BIN_DIR)/SNLoader.bin $(LOADER_FILES_PATH)
	
