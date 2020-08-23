#include "stdafx.h"
#include "string.h"
#include <windows.h>

void _tmain()
{
	PBYTE pVal = NULL;
	__try
	{
		getchar();
		*pVal = 32;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("Access violation caught by handler..\n");
	}
}

