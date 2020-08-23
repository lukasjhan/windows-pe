#include "stdafx.h"
#include <string.h>
#include <windows.h>

#define JMP_RSP_RVA	0x000F61C9	//wmp.dll
#define JMP_ESP_RVA	0x0000260C	//wmp.dll


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

#ifndef _X86_
bool MakeStackExecutable()
{
	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_CONTROL;
	GetThreadContext(GetCurrentThread(), &ctx);

	MEMORY_BASIC_INFORMATION mbi;
	PBYTE pStkAddr = (PBYTE)ctx.Rsp;
	VirtualQuery(pStkAddr, &mbi, sizeof(mbi));

	DWORD dwProtect;
	if (!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &dwProtect))
	{
		printf("VirtualProtect failed, code = 0x%08X\n", GetLastError());
		return false;
	}
	return true;
}
#endif

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
#ifdef _X86_
	PBYTE pJmpSP = (PBYTE)hModWmp + JMP_ESP_RVA;
#else
	PBYTE pJmpSP = (PBYTE)hModWmp + JMP_RSP_RVA;
#endif
	printf("JMP_SP instruction address: 0x%p\n", pJmpSP);
	printf("Input enter to continue...\n");
	getchar();

#ifndef _X86_
	MakeStackExecutable();
#endif

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
	FreeLibrary(hModWmp);
}
