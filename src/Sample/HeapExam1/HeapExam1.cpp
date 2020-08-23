#include "stdafx.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>


void _tmain()
{
	char* ps1 = (char*)malloc(32);
	char* ps2 = (char*)malloc(64);

	strcpy(ps1, "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
				"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
				"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
				"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
				"AAAAAAAAAAAAAAAA");
	strcpy(ps2, "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB");

	printf("%s\n", ps1);
	printf("%s\n", ps2);

	free(ps1);
	free(ps2);
}
