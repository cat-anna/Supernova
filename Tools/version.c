#include <stdio.h>
#include <string.h>
#include <time.h>

#include "helpers/FindCmdLineSwitch.c"
#include "helpers/minini/minini.h"

void print_help(){
    puts("version v1.0\n"
         "Creates header with version definitions\n"
         "\n"
         "usage: version [OPTIONS] -s SOURCE -d HEADER [-name NAME]\n"
         "\n"
         "Options:\n"
         "\t-s SOURCE\tSpecify source ini file\n"
         "\t-d HEADER\tSpecify destination header\n"
         "\t-name NAME\tSpecify target name, optional\n"
		 "\t-dec_XXX\tDecrease value specified by XXX\n"
		 "\t-inc_XXX\tIncrease value specified by XXX\n"
		 "\t-add_date\tCurrent date will be saved to header\n"
		 "\t-add_time\tCurrent time will be saved to header\n"
		 "\t--help\t\tPrints this message and exits\n"
         "\n"		 
		 "\tNote: XXX can only be on of four version value:\n"
		 "\t\tmajor, minor, release, build\n"
		 "\nThis application is part of Supernova tools");
}

int main(int argc, char** argv)
{    
	char* dest_header = "version.h";
	char* src_ini = "version.ini";
    char* src_name = "version";
    if(FindCmdSwitch(argc, argv, "--help", 1)){
        print_help();
        return 0;
    }
	
    unsigned options =
        ((FindCmdSwitch(argc, argv, "-inc_major",	1))?(0x001):(0))	|
        ((FindCmdSwitch(argc, argv, "-inc_minor",	1))?(0x002):(0))	|
        ((FindCmdSwitch(argc, argv, "-inc_release",	1))?(0x004):(0))	|
        ((FindCmdSwitch(argc, argv, "-inc_build",	1))?(0x008):(0))	|
		
		((FindCmdSwitch(argc, argv, "-dec_major",	1))?(0x010):(0))	|
        ((FindCmdSwitch(argc, argv, "-dec_minor",	1))?(0x020):(0))	|
        ((FindCmdSwitch(argc, argv, "-dec_release",	1))?(0x040):(0))	|
        ((FindCmdSwitch(argc, argv, "-dec_build",	1))?(0x080):(0))	|
		
        ((FindCmdSwitch(argc, argv, "-useenum",		1))?(0x100):(0))	|
        ((FindCmdSwitch(argc, argv, "-add_date",	1))?(0x200):(0))	|
        ((FindCmdSwitch(argc, argv, "-add_time",	1))?(0x400):(0));
		
	int i = 1;
	for (; i < argc; i++) {
		if (!strcmp(argv[i], "-s")) {
			src_ini = argv[++i];
		} else
		if (!strcmp(argv[i], "-d")) {
			dest_header = argv[++i];
		} else
		if(!strcmp(argv[i], "-name")){
			src_name = argv[++i];
		}
	}	
		
//long  ini_getl(const TCHAR *Section, const TCHAR *Key, long DefValue, const TCHAR *Filename);
	int major = ini_getl(src_name, "version_major", 0, src_ini);
	int minor = ini_getl(src_name, "version_minor", 0, src_ini);
	int release = ini_getl(src_name, "version_release", 0, src_ini);
	int build = ini_getl(src_name, "version_build", 0, src_ini);
		
	if(options & 0x001)	major++;
	if(options & 0x002)	minor++;
	if(options & 0x004) release++;
	if(options & 0x008)	build++;
	
	if((options & 0x010) && major > 0)		major--;
	if((options & 0x020) && minor > 0)		minor--;
	if((options & 0x040) && release > 0)	release--;
	if((options & 0x080) && build > 0)		build--;
	
	ini_putl(src_name, "version_major", major, src_ini);
	ini_putl(src_name, "version_minor", minor, src_ini);
	ini_putl(src_name, "version_release", release, src_ini);
	ini_putl(src_name, "version_build", build, src_ini);
	ini_putl(src_name, "version_lastcommand", options, src_ini);
	
	char *tmp = src_name;
	//make uppercase
	while(*tmp){
		if(*tmp >= 'a' && *tmp <= 'z')
			*tmp ^= 0x20;
		tmp++;
	}
	
	FILE *f_h = fopen(dest_header, "w");
	fprintf(f_h, "#ifndef _%s_VERSION_DEFINITONS_\n"
				 "#define _%s_VERSION_DEFINITONS_\n"
				 "\n"
				 "#define VERSION_MAJOR\t%4d\n"
				 "#define VERSION_MINOR\t%4d\n"
				 "#define VERSION_RELEASE\t%4d\n"
				 "#define VERSION_BUILD\t%4d\n"
				 "\n",
				 src_name, src_name, major, minor, release, build);
	
	time_t rawtime;
	struct tm * timeinfo;
	time (&rawtime);
	timeinfo = localtime (&rawtime);
	
	if(options & 0x200)
		fprintf(f_h, "#define COMPILE_DATE_YEAR\t%4d\n"
					 "#define COMPILE_DATE_MONTH\t%4d\n"
					 "#define COMPILE_DATE_DAY\t%4d\n"
					 "#define COMPILE_DATE_STRING\t__DATE__\n"
					 "\n",
					 timeinfo->tm_year, timeinfo->tm_mon + 1, timeinfo->tm_mday);
	
	if(options & 0x400)
		fprintf(f_h, "#define COMPILE_TIME_HOUR\t%4d\n"
					 "#define COMPILE_TIME_MINUTE\t%4d\n"
					 "#define COMPILE_TIME_SECOND\t%4d\n"
					 "#define COMPILE_TIME_STRING\t__TIME__\n"
					 "\n",
					 timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
					 
	fprintf(f_h, "#endif\n");

	fclose(f_h);
	return 0;
}
