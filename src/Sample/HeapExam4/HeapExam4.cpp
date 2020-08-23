#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>


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

void ShellCode(void*)
{
	MessageBoxA(NULL, "Your program HACKED!!!", NULL, MB_ICONERROR);
}

void _tmain(void)
{
	printf(".... Attack using UAF Overflow.\n");

	Test* t1 = (Test*)malloc(sizeof(Test));
	t1->print = (PFN_PRINT)printName;
	strcpy(t1->name, "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB");
	t1->print(t1);
	free(t1);

	char* p1 = (char*)malloc(sizeof(Test));
	*((PDWORD)p1) = (DWORD)ShellCode;
	t1->print(t1);
	free(p1);
}
