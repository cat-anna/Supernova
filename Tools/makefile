#Supernova tools makefile

all:	choice.exe version.exe SSM.exe

choice.exe: choice.c
	gcc choice.c -o choice.exe
	
version.exe: version.c helpers/minini/minini.c
	gcc version.c helpers/minini/minini.c -o version.exe
	
SSMSRC=SSM.cpp lua_scripts.cpp IFILES_Volume.cpp FAT_Volume.cpp VolumeInterface.cpp 
	#Supernova_Volume.cpp 
SSMSRC_PATH=$(addprefix SSM/, ${SSMSRC})
	
SSM.exe: $(SSMSRC_PATH)
	g++ $(SSMSRC_PATH) -o SSM.exe -llua5.1

clean:
	rm -f *.exe
	