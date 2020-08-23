#include "stdafx.h"
#include <Windows.h>
#include <stdlib.h>

#include "../ShareSecDll/ShareSecDll.h" 
#ifdef _WIN64
#	ifdef _DEBUG 
#		pragma comment(lib, "../0.bin/x64/Debug/ShareSecDll.lib")
#	else
#		pragma comment(lib, "../0.bin/x64/Release/ShareSecDll.lib")
#	endif
#else
#	ifdef _DEBUG 
#		pragma comment(lib, "../0.bin/x86/Debug/ShareSecDll.lib")
#	else
#		pragma comment(lib, "../0.bin/x86/Release/ShareSecDll.lib")
#	endif
#endif

void main()
{
	char szIn[512];
	while (true)
	{
		gets_s(szIn);
		if (_stricmp(szIn, "quit") == 0)
			break;

		int nLen = (int)strlen(szIn);
		if (nLen > 2 && ((szIn[0] == 'w' || szIn[0] == 'W') && szIn[1] == ':'))
			Share_WriteBuff(szIn + 2, nLen - 2);
		else
		{
			Share_ReadBuff(szIn, 511);
			printf("R:%s\n", szIn);
		}
	}
}