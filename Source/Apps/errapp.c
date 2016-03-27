#include "pawlos.h"

uint32 atoi(char* s){
	if(!s)return 0;
	uint32 r = 0;
	while(*s >= '0' && *s <= '9'){
		r *= 10;
		r += *s++ - '0';
	}
	return r;
}

uint32 glob_zero;

int main(sint32 argc, char** argcv){	

	if(argc < 2){
		printf("Not enough parameters to cause error!\n");
		return 1;
	}
	uint32 no = atoi(argcv[2]);
	
	if(!strcmp(argcv[1], "isr")){
		switch(no){
			case 0:{
				printf("division by %d\n", glob_zero);
				no = 80 / glob_zero;
				printf("%d\n", no);
				break;
			}
			case 14:*((uint32*)0) = 0;	//pagefault
				break;
		}
	}

	printf("Error has failed!\n");
	return 0;
}
