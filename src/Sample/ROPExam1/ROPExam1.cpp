#include "stdafx.h"
#include <string.h>
#include <windows.h>


void TestList(char* pszVal, int nBuffLen)
{
	char szBuff[260];
	int nLen = nBuffLen;
	memcpy(szBuff, pszVal, nLen);
	szBuff[nLen] = 0;

	int nPos = 0;
	while (nPos < nLen)
	{
		nPos += printf("  => %s\n", szBuff + nPos);
	}
}

void _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		printf("ROPExam2 \"mu3 file\"\n");
		return;
	}

	HMODULE hMod = GetModuleHandle(L"kernel32");
	DWORD_PTR dwFncAddr = (DWORD_PTR)GetProcAddress(hMod, "VirtualProtect");
	printf("   VirtualProtect = 0x%p\n", dwFncAddr);

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

		TestList(pszBuff, fileLen);
		delete[] pszBuff;
		fclose(fp);
	}
}
