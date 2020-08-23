#include "stdafx.h"
#include "string.h"
#include <windows.h>

void _tmain(int argc, _TCHAR* argv[])
{
	char szBuff[16];
	strcpy(szBuff, "1234567891");
	printf("Test: %s\n", szBuff);
}

