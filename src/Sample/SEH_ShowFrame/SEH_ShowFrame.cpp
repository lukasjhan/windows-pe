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



void ShowSEHFrame(PEXCEPTION_REGISTRATION_RECORD pEF)
{
	DWORD_PTR dwImageBase = FindImageBase((DWORD_PTR)pEF->Handler);
	if (dwImageBase == 0)
	{
		printf("----> Invalid Handler: 0x%08X\n", pEF->Handler);
		return;
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

	CComBSTR bszFunc, bszHandler;
	if (pdi != NULL)
	{
		CComPtr<IDiaSymbol> pISymb;
		DWORD dwRVA = (DWORD)pEF->Handler - dwImageBase;
		HRESULT	hr = pdi->SESSION->findSymbolByRVA(dwRVA, SymTagNull, &pISymb);
		if (hr == S_OK)
		{
			pISymb->get_name(&bszHandler);
			pISymb = 0;

			if ((INT)pEF->Next != -1)
			{
				DWORD dwRetAddr = *PDWORD((PBYTE)pEF + 20);
				INT nRelVal = *PINT((PBYTE)dwRetAddr - 4);
				dwRetAddr += nRelVal;

				dwRVA = dwRetAddr - dwImageBase;
				hr = pdi->SESSION->findSymbolByRVA(dwRVA, SymTagNull, &pISymb);
				if (hr == S_OK)
				{
					pISymb->get_name(&bszFunc);
					pISymb = 0;
				}
			}
		}
	}

	if ((INT)pEF->Next != -1)
		printf("==> Function \"%S\" has SEH Frame\n", bszFunc);
	else
		printf("==> LAST SEH Frame\n");
	printf("    Frame Address=0x%08X, Previous Frame=0x%08X\n", pEF, pEF->Next);
	printf("    Handler=0x%08X (%S)\n\n", pEF->Handler, bszHandler);
}

void WalkSEHFrames()
{
	PEXCEPTION_REGISTRATION_RECORD pFrame = NULL;
	__asm
	{
		mov  eax, fs:[0]
		mov  [pFrame], eax
	}

	while (0xFFFFFFFF != (unsigned)pFrame)
	{
		ShowSEHFrame(pFrame);
		pFrame = pFrame->Next;
	}
}

void TestTryExcept()
{
	__try
	{
		__try
		{
			__try
			{
				WalkSEHFrames();
			}
			__except(EXCEPTION_CONTINUE_SEARCH)
			{
			}
		}
		__finally
		{
		}
	}
	__except(EXCEPTION_CONTINUE_SEARCH)
	{
	}
}

void _tmain()
{
	CoInitialize(NULL);

	////////////////////////////////////////////////////////////////////////////////////////////////
	//
	__try
	{
		TestTryExcept();
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		printf("Caught Exception in main\n");
	}
	////////////////////////////////////////////////////////////////////////////////////////////////

	UnloadInterface();
	CoUninitialize();
}
