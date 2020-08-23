#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void _tmain()
{
	int* heap1 = (int*)malloc(256);
	int* heap2 = (int*)malloc(256);

	printf("heap1 : 0x%p\n", heap1);
	printf("heap2 : 0x%p\n", heap2);

	*heap2 = 0x1234;
	*(heap2 + 1) = 0x5678;
	*(heap2 + 2) = 0x9ABC;
	printf("heap2 number : %d\n", *heap2);

	free(heap2);
	printf("free heap2\n");

	int* heap3 = (int*)malloc(256);
	printf("new heap : 0x%p\n", heap3);
	printf("new heap number: 0x%08X, 0x%08X, 0x%08X\n", *heap3, *(heap3 + 1), *(heap3 + 2));
}
