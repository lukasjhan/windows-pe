#include "stdafx.h"
#include "string.h"
#include <windows.h>

#define POP_POP_RET_RVA		0x00222D70	//wmp.dll


void TestSEH(char* pszVal, int nBuffLen)
{
	__try
	{
		char szBuff[260];
		int nLen = nBuffLen;
		memcpy(szBuff, pszVal, nLen);
		szBuff[nLen] = 0;

		nLen /= 0;
		//int nPos = 0;
		//while (nPos < nLen)
		//{
		//	nPos += printf("  => %s\n", szBuff + nPos);
		//}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("Access violation caught by handler..\n");
	}
}

void _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		printf("GSExam2 \"mu3 file\"\n");
		return;
	}

	HINSTANCE hModWmp = LoadLibrary(L"wmp.dll");
	if (hModWmp == NULL)
	{
		printf("wmp.dll load failed...\n");
		return;
	}
	PBYTE pJmpSP = (PBYTE)hModWmp + POP_POP_RET_RVA;
	printf("POP_POP_RET instruction address: 0x%p\n", pJmpSP);
	printf("Input enter to continue...\n");
	getchar();

	FILE* fp = _wfopen(argv[1], L"rt");
	if (fp != NULL)
	{
		fseek(fp, 0, SEEK_END);
		int fileLen = ftell(fp);
		PCHAR pszBuff = new char[fileLen + 1];

		fseek(fp, 0L, SEEK_SET);
		fgets(pszBuff, fileLen + 1, fp);
		printf("Read buffer ====================\n");
		printf("%s\n", pszBuff);
		printf("Read buffer ====================\n\n");

		TestSEH(pszBuff, fileLen);
		delete[] pszBuff;
		fclose(fp);
	}
	FreeLibrary(hModWmp);
}

