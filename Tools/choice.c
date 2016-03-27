#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers/FindCmdLineSwitch.c"

void print_help()
{
    puts("choice v1.0\n"
         "Reads value typped by user and return it to system\n"
         "\n"
         "usage: choice [options]\n"
         "\n"
         "Options:\n"
         "\t-force\t\tForce proper answer\n"
         "\t-max value\tSpecify max value\n"
         "\t-min value\tSpecify min value\n"
		 "\t--help\t\tPrints this message and exits\n"
         "\n"
		 "\nThis application is part of Supernova tools");
}

int main(int argc, char** argv)
{
    int max;
    int min;
    if(FindCmdSwitch(argc, argv, "--help", 1)){
        print_help();
        return 0;
    }
    unsigned options =
        ((FindCmdSwitch(argc, argv, "-force", 1))?(0x01):(0));
    int ti = FindCmdSwitch(argc, argv, "-min", 1);
    if(ti && ++ti < argc){
        options |= 0x02;
        min = atoi(argv[ti]);
    }
    ti = FindCmdSwitch(argc, argv, "-max", 1);
    if(ti && ++ti < argc){
        options |= 0x04;
        max = atoi(argv[ti]);
    }
    char tmp[50] = {0};

    while(1){
        printf("Type answer: ");
        scanf("%s", tmp);
        char *t = tmp;
        if(!(options & 0x01))break;
        int wrong = 0;
        while(*t)
            if(*t < '0' || *t > '9'){
                wrong = 1;
                break;
            } else t++;
        if(!wrong){
            int value = atoi(tmp);
            if((options & 0x02) && value < min)
                wrong |= 0x02;
            if((options & 0x04) && value > max)
                wrong |= 0x04;

            if(!wrong)return value;
        }
        if(!(options & 0x01))return -1;
        printf("Wrong answer! ");
    }
    return -1;
}
