#include "stdafx.h"
#include <stdio.h>

void main()
{
	__asm
	{
		mov eax, 324342;
		mov edx, 352;
		bt eax, edx;
	}

	int k = 0;
}
