#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void _tmain()
{
	char* a = (char*)malloc(0x30);
	char* b = (char*)malloc(0x30);
	char* c = (char*)malloc(0x30);

	strcpy(a, "AAAAAAAAAAAAAAAAAAAAAAA");
	strcpy(b, "BBBBBBBBBBBBBBBBBBBBBBB");
	strcpy(c, "CCCCCCCCCCCCCCCCCCCCCCC");

	printf("a => 0x%p:%s\n", a, a);
	printf("b => 0x%p:%s\n", b, b);
	printf("c => 0x%p:%s\n", c, c);

	free(c);
	free(b);
	free(a);
}
