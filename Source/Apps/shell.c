#include <Supernova.h>
//#include <stdio.h>

//char *cmd;
//char *path;

//void ParseCommand(char* Cmd);
//void SimpleCommands(char** Paramstr, uint8 CmdId);
//void vfsCommands(char** Paramstr, uint8 CmdId);

//typedef struct CommandData_s{
//	const char *Cmd;
//	void(*Ptr)(char**, uint8);
//	uint8 CmdId;
//	const char *HelpStr;
//} CommandData_t;

//CommandData_t CommandTable[] = {
//	{"clc", &SimpleCommands, 1, "clear console"},
//	{"help", &SimpleCommands, 2, "shows this help"},
//	{"print", &SimpleCommands, 5, "print"},
//	{"exit", &SimpleCommands, 6, "exits form shell"},
//	{"time", &SimpleCommands, 7, "time"},
//
//	{"list", &vfsCommands, 1, "lists directory"},
//	{"go", &vfsCommands, 2, "changes the directory"},
//	{"type", &vfsCommands, 3, "print file"},
//
//	{"test", &SimpleCommands, 0xAA, "test function"},
//	{"debug1", &SimpleCommands, 0xDD, "debug f1"},
//
////	{"", 0, 0, ""},
////	{"", 0, 0, ""},
////	{"", 0, 0, ""},
//	{"die", &SimpleCommands, 0xFF, "no info"},
//	{"", 0, 0, ""}, //puste pole oznacza koniec tablicy
//};
/*
static inline void PrintDate(uint32 Date){
	printf("%;4d\\%;2d\\%;2d %;2d:%;2d", (Date & Date_Year) >> 20, (Date & Date_Month) >> 16,
				(Date & Date_Day) >> 11, (Date & Date_Hour) >> 6, (Date & Date_Minutes));
}*/

//void vfsCommands(char** Paramstr, uint8 CmdId){
//	CmdId = 0;
//	Paramstr = 0;
//	switch (CmdId){
//		case 1:{//list
//			char* pathend = strchr(path, 0);
//			if(Paramstr[1]){
//				if(*(pathend - 1) != '\\')*pathend = '\\';
//				strcat(path, Paramstr[1]);
//			}
//			uint32 dcount = 0, fcount = 0, tsize = 0;
//			SearchRec_t sr;
//			printf("\nDirectory: %s\n\n", path);
//			HANDLE H = FindFirst(path, &sr);
//			if(H != INVALID_HANDLE_VALUE){
//				while(1){
//					PrintDate(sr.date);
//					if(sr.flags & FILE_FLAG_DIR){
//						printf(" %8s", "<DIR>");
//						dcount++;
//					}else {
//						printf(" %8d", sr.size);
//						fcount++;
//						tsize += sr.size;
//					}
//					print(" %s\n", sr.name);
//					if(FindNext(H, &sr) != 0)break;
//				}
//				printf("\n%5d files in\t %8d bytes\n%5d folders\n", fcount, tsize, dcount);
//				printf("%5d free of %5d total kbytes on volume\n", GetFreeVolumeSpace(path) >> 10, GetTotalVolumeSpace(path) >> 10);
//			} else printf("No files found.\n");
//			FindClose(H);
//			*pathend = 0;
//		}break;
//		case 2:{//go
//			char* tmp_path = 0;
//			if(!Paramstr[1])return;
//			char *lastslash = strrchr(Paramstr[1], '\\');
//			if(lastslash && *(lastslash + 1) == 0)*lastslash = 0;
//		    if(Paramstr[1][0] == '.' && Paramstr[1][1] == '.'){
//				char* rev = strrchr(path, '\\');
//				if(*(rev - 1) == ':')break;
//				*rev = 0;
//				if(*(rev + 1) == 0){
//					rev = strrchr(path, '\\');
//					*++rev = 0;
//				}
//				break;
//			} else
//			if(Paramstr[1][0] == '\\'){
//				char* fslash = strchr(path, '\\');
//				if(!fslash)return;
//				if(Paramstr[1][1] == 0){
//					*++fslash = 0;
//					return;
//				}
//				//Paramstr[1]++
//				tmp_path = (char*)malloc(fslash - path + strlen(Paramstr[1]) + 1);
//				*fslash = 0;
//				strcpy(tmp_path, path);
//				strcat(tmp_path, Paramstr[1]);
//				*fslash = '\\';
//			} else {
//				tmp_path = (char*)malloc(strlen(path) + strlen(Paramstr[1]) + 1);
//				strcpy(tmp_path, path);
//				strcat(tmp_path, Paramstr[1]);
//			}
//			if(!DirectoryExists(tmp_path)){
//				printf("wrong path!\n");
//			} else
//				strcpy(path, tmp_path);
//			char* end = strrchr(path, 0);
//			if(*--end != '\\'){
//				*++end = '\\';
//				*++end = 0;
//			}
//			free(tmp_path);
//		}break;
//		case 3:{//type
//			char* pathend = strchr(path, 0);
//			if(Paramstr[1]){
//				char* last = pathend;
//				if(*--last != '\\')*pathend = '\\';
//				strcat(path, Paramstr[1]);
//			} else {
//				printf("Wrong file name!\n");
//				return;
//			}
//			HANDLE h = CreateFile(path, FILE_ACCES_READ, FILE_OPEN_EXISTING);
//			if(h != INVALID_HANDLE_VALUE){
//				uint32 fsize = GetFileSize(h);
//				char* rb = (char*)malloc(fsize + 1);
//				ZeroMemory(rb, fsize + 1);
//				if(ReadFile(h, rb, fsize) != fsize){
//					printf("Error reading file!\n");
//				} else
//					print("%s\n", rb);
//				CloseHandle(h);
//				free(rb);
//			} else
//				printf("Error opening the file!\n");
//			*pathend = 0;
//		}break;
//	}
//}

//char die_msg1[] = "\n\n\n\n PawlOS has died...\n Now is your turn...\n";
//char die_msg2[] = "\n\n\n\n beware of darkness...";

//void SimpleCommands(char** Paramstr, uint8 CmdId){
//	CmdId = 0;
//	Paramstr = 0;
//	switch (CmdId){
//		case 1:	Console_Clear(); break;
//		case 2: //pomoc
//			puts("\nList of available commands:");
//			uint32 i = 0;
//			for(; CommandTable[i].Ptr != 0; i++)
//				printf("\t%s - %s\n", CommandTable[i].Cmd, CommandTable[i].HelpStr);
//			break;
//		case 5: print(Paramstr[1]); break;
//		case 6://exit
//			PostQuitMessage(0);
//			break;
//		case 7:{//time
//			SystemTime_t st;
//			GetSystemTime(&st);
//			printf("%;4d\\%;2d\\%;2d %;2d:%;2d:%;2d\n", st.year, st.month, st.day, st.hour, st.min, st.sec);
//			break;
//		}
//		case 0xAA:{//test
//			HANDLE H = CreateFile("POS:\\test.txt", FILE_ACCES_FULL, FILE_CREATE_NEW);
//			void* buf = malloc(2048);
//			ZeroMemory(buf, 2048);
//			if(H != INVALID_HANDLE_VALUE){
//				printf("WriteFile1 %d\n\n", WriteFile(H, buf, 2048));
//				printf("WriteFile2 %d\n\n", WriteFile(H, buf, 2048));
//				printf("WriteFile3 %d\n", WriteFile(H, buf, 2048));
//				printf("WriteFile4 %d\n", WriteFile(H, buf, 2048));
//				printf("WriteFile5 %d\n", WriteFile(H, buf, 2048));
//			}else printf("CreateFile error\n");
//			free(buf);
//			CloseHandle(H);
//		}break;
//		case 0xDD:{
//			DoInt0x80(SYSCALL_DEBUG_HEAP, 0, 0, 0);
//			break;
//		}
//		case 0xFF:{//die
//			print("As you whish...");
//			Sleep(500);
//			print("\n               ...my lord.\n");
//			Sleep(500);
//			rand_seed = GetTickCount();
//			uint8 *light = (uint8*)malloc(80);
//			uint8 *dark = (uint8*)malloc(80);
//			ZeroMemory(light, 80);
//			ZeroMemory(dark, 80);
//			char* scr = (char*)malloc(80*25);
//			ZeroMemory(scr, 80*25);
//			uint32 l = 0;
//			while(1){
//				l++;
//				if((l % 20) == 0)Sleep(0);
//				uint32 k = (rand() % 160)/2;
//				uint32 m = (rand() >> 5) %2;
//				if(m == 1 && light[k] <= dark[k])m = 0;
//				if(m == 0){
//					if(light[k] >= 25) m = 1;
//					else {
//						Console_Goto(k, light[k]);
//						print("%12f%5b%c", 0xDB);
//						light[k]++;
//						continue;
//					}
//				}
//				if(m == 1){
//					if((k < 80 && dark[k] >= 25) || (k == 79 && dark[k] == 24)){
//						uint32 i = 0, c = 0;
//						for(; i<80; i++)if(dark[i] == 25)c++;
//						if(dark[79] == 24)c++;
//						if(c == 80)break;
//						continue;
//					}
//					else {
//						Console_Goto(k, dark[k]);
//						print("%4f%4b ");
//						dark[k]++;
//					}
//				}
//
//			}
//			Sleep(150);
//			Console_Clear();
//			Sleep(150);
//			print("%0f%4b");
//			char* msg = die_msg1;
//			while(*msg){
//				putch(*msg);
//				msg++;
//				Sleep(100);
//			}
//			Sleep(3000);
//			msg = die_msg2;
//			while(*msg){
//				putch(*msg);
//				msg++;
//				Sleep(100);
//			}
//			print("%0f%0b");
//			*((uint32*)0) = 0;
//		}
//	}
//}

//void ParseCommand(char* Command){
//	char *Params[10];
//	uint32 i = 0;
//	for(; i < 10; i++)Params[i] = 0;
//	Params[0] = Command;
//	Params[1] = strchr(Command, ' ');
//	if(Params[1] != 0){
//		*Params[1] = 0;
//		Params[1]++;
//	}
//
//	if(Params[0]){
//		char* pathend = strchr(path, 0);
//		if(*(pathend - 1) != '\\')*pathend = '\\';
//		strcat(path, Params[0]);
//		if(FileExists(path)){
//			if(!ExecuteFile(path, Params[1], true))
//				printf("Error has occur while executing file.\n");
//			*pathend = 0;
//			return;
//		}
//		*pathend = 0;
//	}
//	char *p = Params[1];
//	char** pt = Params;
//	pt++;
//	while(p && *p)
//		if(*p == ' '){
//			*p++ = 0;
//			*++pt = p;
//		} else p++;
//
//	for(i = 0; CommandTable[i].Ptr != 0;i++)
//	{
//		if(strcmp(Params[0], CommandTable[i].Cmd) == 0){
//			CommandTable[i].Ptr(Params, CommandTable[i].CmdId);
//			return;
//		}
//	}
//
//	printf("Unknown command or file\n");
//}

int main(/*sint32 argc, char** argcv*/){	
//	cmd = (char*)malloc(250);
//	path = (char*)malloc(250);
//	GetCurrentDirectory(path);

putf("blalba");
Breakpoint();


//	SystemInfo_t sysi;
//	ZeroMemory(&sysi, sizeof(SystemInfo_t));
//	sysi.size = sizeof(SystemInfo_t);
//	GetSystemInformation(&sysi);

//	printf("\n\nWelcome to Supernova %d.%d build %d\n", sysi.version.major, sysi.version.minor, sysi.version.build);
	/*uint32 i = 0;
	while(argc > 0){
		printf("\t|%s|\n", argcv[i]);
		i++;
		argc--;
	}*/
//if(argc == 1)
	//	ParseCommand("shell.elf nope");
	
//	TimeSetEvent(1, 1000);

	//fopen("bla", "r");

	//msg_prot_p msg;
	//while(1){
		//Breakpoint();
	//	putf("bla");
	//	Sleep(1000);
	/*	printf("\n%s>", path);
		gets(cmd);
		ParseCommand(cmd);				
		if(GetMessage(&msg) == 0){
			switch (msg->type){
				case MSG_EXIT: return ((message_p)msg)->hparam;
				case MSG_TIMER:
					printf("timer %d\n", GetTickCount());
					//TimeSetEvent(1, 1000);
					break;
			}			
		}	*/
//	}
	
	return 0;
}

int blabla(){
//	putf("blabla\n");
	//fopen("bla", "r");
	return 0;
}
