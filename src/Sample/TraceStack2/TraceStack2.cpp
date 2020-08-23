#include "stdafx.h"
#include <windows.h>
#include <Shlobj.h>

#if !defined _M_IX86
#error	"Not support on X64"
#endif


#include <atlbase.h>
#include <atlstr.h>
#include "..\..\Solution\PEPlus\PEPlus.h"
#include "..\..\Solution\PEPdb\PEPdb.h"
#pragma warning( disable : 4996 )

#ifdef _DEBUG 
#	pragma comment(lib, "../../3.lib/x86/Debug/PEPlus.lib")
#	pragma comment(lib, "../../3.lib/x86/Debug/PEPdb.lib")
#else
#	pragma comment(lib, "../../3.lib/x86/Release/PEPlus.lib")
#	pragma comment(lib, "../../3.lib/x86/Release/PEPdb.lib")
#endif


#include <map>
typedef std::map<DWORD_PTR, PEPdb*> MOD_DIA_MAP;
MOD_DIA_MAP G_MD_MAP;

#define GSZ_APP_TMP_CACHE	_T("%s\\Temp\\SymbolCache")
TCHAR SYM_PATH[MAX_PATH];

PEPdb* LoadInterface(DWORD_PTR ulImgBase)
{
	if (SYM_PATH[0] == 0)
	{
		PWSTR pszAppData = NULL;
		SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &pszAppData);
		wsprintf(SYM_PATH, GSZ_APP_TMP_CACHE, pszAppData);
	}

	try
	{
		DWORD dwTimeStamp = 0;
		PIMAGE_PDB_INFO ppi = PEPlus::GetPdbInfo((PBYTE)ulImgBase, &dwTimeStamp, false);
		if (ppi == NULL)
			return NULL;
		return new PEPdb(ppi, dwTimeStamp, SYM_PATH);
	}
	catch (HRESULT hr)
	{
		printf("Error occured, code=0x%08X\n", hr);
		return NULL;
	}
}

void UnloadInterface()
{
	for (MOD_DIA_MAP::iterator it = G_MD_MAP.begin(); it != G_MD_MAP.end(); it++)
		delete it->second;
}

DWORD_PTR FindImageBase(DWORD_PTR ulCtrlPc)
{
	DWORD_PTR ulImageBase = (DWORD_PTR)GetModuleHandle(L"ntdll");
	PIMAGE_NT_HEADERS pnh = PIMAGE_NT_HEADERS(ulImageBase + PIMAGE_DOS_HEADER(ulImageBase)->e_lfanew);
	if (ulCtrlPc > ulImageBase && ulCtrlPc < ulImageBase + pnh->OptionalHeader.SizeOfImage)
		return ulImageBase;

	ulImageBase = (DWORD_PTR)GetModuleHandle(NULL);
	pnh = PIMAGE_NT_HEADERS(ulImageBase + PIMAGE_DOS_HEADER(ulImageBase)->e_lfanew);
	if (ulCtrlPc > ulImageBase && ulCtrlPc < ulImageBase + pnh->OptionalHeader.SizeOfImage)
		return ulImageBase;

	PIMAGE_DATA_DIRECTORY pdd = &pnh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR pids = (PIMAGE_IMPORT_DESCRIPTOR)(ulImageBase + pdd->VirtualAddress);
	int nItemCnt = pdd->Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);

	for (int i = 0; i < nItemCnt; i++)
	{
		if (pids[i].Name == 0)
			continue;

		DWORD_PTR ulDllBase = (DWORD_PTR)GetModuleHandleA((PCSTR)(pids[i].Name + ulImageBase));
		if (ulDllBase == 0)
			continue;

		pnh = PIMAGE_NT_HEADERS(ulDllBase + PIMAGE_DOS_HEADER(ulDllBase)->e_lfanew);
		if (ulCtrlPc > ulDllBase && ulCtrlPc < ulDllBase + pnh->OptionalHeader.SizeOfImage)
			return ulDllBase;
	}
	return 0;
}


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
	printf("Function \"F3\" called, EBP=0x%08X\n\n\n",  dwEBP);

	printf("EntryPtr : 0x%08X, PEB      : 0x%08X\n", dwEntPos, dwPEB);
	printf("ImageBase: 0x%p, StackBase: 0x%08X\n", pPEBase, dwBase);
	printf("Call Stack :\n");
	printf("Index\t%-23s\tBeginAddr\tEBP(FP)\t\tPrevEBP\t\tRetAddr\n", "Func");

	int nIndex = 0;
	DWORD dwIter = dwEBP;
	while (dwIter < dwBase)
	{
		DWORD dwPrevEbp = *((PDWORD)dwIter);
		DWORD dwRetAddr = *((PDWORD)(dwIter + 4));
		if (dwRetAddr == 0)
			break;

		DWORD_PTR dwImageBase = FindImageBase((DWORD_PTR)dwRetAddr);
		if (dwImageBase == 0)
		{
			printf("----> Invalid code address: 0x%08X\n", dwRetAddr);
			break;
		}

		PEPdb* pdi = NULL;
		MOD_DIA_MAP::iterator it = G_MD_MAP.find(dwImageBase);
		if (it == G_MD_MAP.end())
		{
			pdi = LoadInterface(dwImageBase);
			if (pdi != NULL)
				G_MD_MAP.insert(std::make_pair(dwImageBase, pdi));
		}
		else
			pdi = it->second;

		DWORD dwFncAddr = 0;
		WCHAR szFncName[64] = { 0, };
		if (pdi != NULL)
		{
			CComPtr<IDiaSymbol> pISymb;
			DWORD dwRVA = dwRetAddr - dwImageBase;
			if (pdi->SESSION->findSymbolByRVA(dwRVA, SymTagNull, &pISymb) == S_OK)
			{
				enum SymTagEnum tag;
				pISymb->get_symTag((PDWORD)&tag);
				if (tag == SymTagBlock)
				{
					CComPtr<IDiaSymbol> pIUpSym;
					pISymb->get_lexicalParent(&pIUpSym);
					pISymb = pIUpSym;
					pIUpSym = 0;
				}

				CComBSTR bszFunc;
				if (pISymb->get_name(&bszFunc) == S_OK)
					wcscpy(szFncName, bszFunc);
				if (pISymb->get_relativeVirtualAddress(&dwRVA) == S_OK)
					dwFncAddr = (DWORD)dwImageBase + dwRVA;
				pISymb = 0;
			}
		}
		if (szFncName[0] == 0)
			wsprintf(szFncName, L"??_%d", nIndex);

		if (dwFncAddr == 0)
		{
			dwFncAddr = dwRetAddr - 4;
			if (*((PBYTE)dwFncAddr - 1) == 0xE8)
			{
				int nFncOffs = *((PINT)dwFncAddr);
				dwFncAddr = dwRetAddr + nFncOffs;
			}
			else
				dwFncAddr = 0;
		}
		printf("%d\t%-23S\t0x%08X\t0x%08X\t0x%08X\t0x%08X\n", 
			nIndex, szFncName, dwFncAddr, dwIter, dwPrevEbp, dwRetAddr);

		dwIter = dwPrevEbp;
		nIndex++;
	}
}

void F2()
{
	DWORD dwEBP = 0;
	__asm mov dwEBP, ebp
	printf("Function \"F2\" called, EBP=0x%08X\n", dwEBP);

	F3();
}


void F1()
{
	DWORD dwEBP = 0;
	__asm mov dwEBP, ebp
	printf("Function \"F1\" called, EBP=0x%08X\n", dwEBP);

	F2();
}

void _tmain()
{
	CoInitialize(NULL);

	DWORD dwEBP = 0;
	__asm mov dwEBP, ebp
	printf("Function \"main\" called, EBP=0x%08X\n", dwEBP);

	F1();

	UnloadInterface();
	CoUninitialize();
}
