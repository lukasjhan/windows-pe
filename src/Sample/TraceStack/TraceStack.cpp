#include "stdafx.h"
#include <windows.h>

#if !defined _M_IX86
#error	"Not support on X64"
#endif

#include <map>
typedef std::map<DWORD, PCSTR> BP_FNC_MAP;
BP_FNC_MAP g_bfMap;

void F3()
{
	PBYTE pPEBase = (PBYTE)GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS32 pnh = (PIMAGE_NT_HEADERS32)(pPEBase + ((PIMAGE_DOS_HEADER)pPEBase)->e_lfanew);
	DWORD dwEntPos = (DWORD)(pPEBase + pnh->OptionalHeader.AddressOfEntryPoint);

	DWORD dwEBP = 0, dwBase = 0, dwPEB;
	__asm
	{
		mov dwEBP, ebp
		mov eax, fs:[04h]
		mov dwBase, eax
		mov eax, fs : [30h]
		mov dwPEB, eax
	}
	g_bfMap.insert(std::make_pair(dwEBP, "F3"));
	printf("Function \"F3\" called, EBP=0x%08X\n\n\n",  dwEBP);

	printf("EntryPtr : 0x%08X, PEB      : 0x%08X\n", dwEntPos, dwPEB);
	printf("ImageBase: 0x%p, StackBase: 0x%08X\n", pPEBase, dwBase);
	printf("Call Stack :\n");
	printf("Index\tFunc\tBeginAddr\tEBP(FP)\t\tPrevEBP\t\tRetAddr\n");

	int nIndex = 0;
	DWORD dwIter = dwEBP;
	while (dwIter < dwBase)
	{
		DWORD dwPrevEbp = *((PDWORD)dwIter);
		DWORD dwRetAddr = *((PDWORD)(dwIter + 4));
		if (dwRetAddr == 0)
			break;

		char szFncName[64];
		BP_FNC_MAP::iterator it = g_bfMap.find(dwIter);
		if (it != g_bfMap.end())
			strcpy_s(szFncName, it->second);
		else
			sprintf_s(szFncName, "??_%d", nIndex);

		DWORD dwFncAddr = dwRetAddr - 4;
		if (*((PBYTE)dwFncAddr - 1) == 0xE8)
		{
			int nFncOffs = *((PINT)dwFncAddr);
			dwFncAddr = dwRetAddr + nFncOffs;
		}
		else
			dwFncAddr = 0;

		printf("%d\t%s\t0x%08X\t0x%08X\t0x%08X\t0x%08X\n", 
			nIndex, szFncName, dwFncAddr, dwIter, dwPrevEbp, dwRetAddr);

		dwIter = dwPrevEbp;
		nIndex++;
	}
}

void F2()
{
	DWORD dwEBP = 0;
	__asm mov dwEBP, ebp
	g_bfMap.insert(std::make_pair(dwEBP, "F2"));
	printf("Function \"F2\" called, EBP=0x%08X\n", dwEBP);

	F3();
}


void F1()
{
	DWORD dwEBP = 0;
	__asm mov dwEBP, ebp
	g_bfMap.insert(std::make_pair(dwEBP, "F1"));
	printf("Function \"F1\" called, EBP=0x%08X\n", dwEBP);

	F2();
}

void _tmain()
{
	DWORD dwEBP = 0;
	__asm mov dwEBP, ebp
	g_bfMap.insert(std::make_pair(dwEBP, "main"));
	printf("Function \"main\" called, EBP=0x%08X\n", dwEBP);

	F1();
}
