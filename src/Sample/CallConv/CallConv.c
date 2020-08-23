#include <windows.h>
#include <tchar.h>
#include <stdio.h>

int __cdecl Test_CDecl(BOOL bAdd, __int64 a, int b, short c)
{
	int nRet   = 0;
	int nVal64 = (int)a;

	if (bAdd)
		nRet = (nVal64 + b + c);
	else
		nRet = (nVal64 - b - c);

	return nRet;
}

int __stdcall Test_StdCall(BOOL bAdd, __int64 a, int b, short c)
{
	int nRet = 0;
	int nVal64 = (int)a;

	if (bAdd)
		nRet = (nVal64 + b + c);
	else
		nRet = (nVal64 - b - c);

	return nRet;
}

int __fastcall Test_FastCall(BOOL bAdd, __int64 a, short b, short c)
{
	int nRet = 0;
	int nVal64 = (int)a;

	if (bAdd)
		nRet = (nVal64 + b + c);
	else
		nRet = (nVal64 - b - c);

	return nRet;
}


void _tmain()
{
	int c = Test_CDecl(TRUE, 5, 6, 7);

	int s = Test_StdCall(FALSE, 5, 6, 7);

	int f = Test_FastCall(TRUE, 5, 6, 7);

	printf("cdecl: %d, std: %d, fast: %d\n", c, s, f);
}
