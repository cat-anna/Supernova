#include <stdio.h>
#include "windows.h"
#include <string.h>

using namespace std;

int main(int argc, char *argv[])
{
	HANDLE plik;
	DWORD FSize = 0;
	for(int i = 1; i < argc; i++)
	{
		if(i + 1 > argc)
		{
			printf("wystapil blad w paramerach!\n");
			return 1;
		} else
		if(!strcmp(argv[i], "-o"))
		{
			i++;
			plik = CreateFileA(argv[i], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
		} else 
		if(!strcmp(argv[i], "-i"))
		{
			i++;
			printf("dodaje plik: %s\n", argv[i]);
			HANDLE insert = CreateFileA(argv[i], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
			DWORD read, written;
			void* bufer = malloc(1024);
			
			do{
				ReadFile(insert, bufer, 1024, &read, NULL);
				WriteFile(plik, bufer, read, &written, NULL);
				FSize += written;				
			} while(read > 0);
			free(bufer);
			CloseHandle(insert);
		} else 
		if(!strcmp(argv[i], "-e"))
		{
			i++;
			int count = atoi(argv[i]);
			printf("dodaje pusta przestrzen: %d bajtow\n", count);
			void* mem = malloc(count);
			ZeroMemory(mem, count);
			DWORD written;
			WriteFile(plik, mem, count, &written, NULL);
			FSize += written;
			free(mem);
		}
		if(!strcmp(argv[i], "-d"))
		{
			i++;
			int count = atoi(argv[i]);
			printf("dopelniam z %d do %d\n", FSize, count);
			count -= FSize;
			if(count > 0){
				void* mem = malloc(count);
				ZeroMemory(mem, count);
				DWORD written;
				WriteFile(plik, mem, count, &written, NULL);
				FSize += written;			
				free(mem);
			}
		}
	}
	CloseHandle(plik);
    
    return 0;
}
