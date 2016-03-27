#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "VolumeInterface.h"
#include "FAT_Volume.h"

#include "lua_scripts.h"

int strcmp2delim(const char *s1, const char *s2, const char delim) {
    unsigned char uc1, uc2;
    /* Move s1 and s2 to the first differing characters
    in each string, or the ends of the strings if they
    are identical.  */
    while (*s1 && *s1 == *s2 && *s1 != delim && *s2 != delim) {
        s1++;
        s2++;
    }
    /* Compare the characters as unsigned char and
    return the difference.  */
    if((*s1 == 0 || *s1 == delim)&&(*s2 == 0 || *s2 == delim))return 0;
    uc1 = (*(unsigned char *) s1);
    uc2 = (*(unsigned char *) s2);
    return ((uc1 < uc2) ? -1 : (uc1 > uc2));
}

char savetext[] = "to jest testowy tekst.\nZostanie on zapisany do pliku.\nKiedys.\nW koñcu.";

int main(int argc, char *argv[]) {
    srand(time(NULL));
    char *iname = 0;

    for(int i = 1; i < argc - 1; i++) {
        if(!strcmp(argv[i], "-do")) {
            char* fncName = strrchr(argv[++i], ':');
            if(!fncName) {
                printf("SSM: Function name not found!\n");
                return 1;
            }
            *fncName++ = 0;
            unsigned EC = 0;
            char* scrn = strrchr(argv[i], '/');
            if(!scrn)scrn = strrchr(argv[i], '\\');
            if(!scrn)scrn = argv[i];
            if(*scrn == '\\' || *scrn == '/')scrn++;
            char* p = ((i < argc - 1)?(argv[i + 1]):(0));
            switch((EC = DoScript(argv[i], fncName, p))) {
            case 0:
                printf("SSM: %s::%s Function completed\n", scrn, fncName);
                break;
            case 1:
                printf("SSM: %s:%s: Error opening file!\n", scrn, fncName);
                break;
            case 2:
                printf("SSM: %s:%s: Error executing function!\n", scrn, fncName);
                break;
            default:
                printf("SSM: %s:%s: Function has return with error code %d\n", scrn, fncName, EC - 100);
            }
            return EC;
        } else if(!strcmp(argv[i], "-o")) {
            iname = new char[strlen(argv[++i]) + 1];
            strcpy(iname, argv[i]);
        }
    }

    if(!iname) {
        char in[] = "binary/Supernova.img";
        iname = new char[strlen(in) + 1];
        strcpy(iname, in);
        printf("SSM: No selected Supernova image. Defaulting to \"%s\"\n", iname);
    }

    char* cmd = (char*)malloc(100);
    cImage *Image = new cImage();
    Image->OpenFile(iname, 512);

    if(!Image->IsReady()){
    	printf("Error opening image\n");
    	return 2;
    }

    VolumeInterface *Volume = new FAT_Volume(Image);
    unsigned ec;
    if((ec = Volume->LastError())){
    	printf("Unknown image format!  ec:%d\n", ec);
    	return 1;
    }

    Volume->Format("Supernova", &PredefinedGeometry[Geometry_Fdd144M]);

 //   Volume->List(0);
 /*   cout << "\nCreateTest:\n";

    unsigned h = Volume->CreateFile("/test.txt", FILE_OPEN_ALLWAYS);
    printf("CreateFile: %x LE:%x\n", h, Volume->LastError());

    unsigned w;
    unsigned res =  Volume->WriteFile(h, savetext, strlen(savetext), &w);
    printf("WriteFile: %x LE:%x\n", res, Volume->LastError());
    */

 //   cout << "\nInsertTest:\n";

/*	while(1) {
		printf("\n>");
		gets(cmd);
		if(!strcmp2delim("list", cmd, ' '))
			Volume->List(0);
		else if(!strcmp2delim("exit", cmd, ' '))break;
		else
			printf("Unknown command");
	}*/
    Volume->InsertFile("/osr.pdf", "binary/osr.pdf");

    delete Volume;
    delete cmd;
    delete iname;
    return 0;
}
