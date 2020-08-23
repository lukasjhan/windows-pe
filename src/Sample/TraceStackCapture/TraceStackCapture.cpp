#include "stdafx.h"
#include <windows.h>
#include <Shlobj.h>

#include <atlbase.h>
#include <atlstr.h>

#if !defined _M_X64
#error	"Not support on x86-32"
#endif


#include <atlbase.h>
#include <atlstr.h>
#include "..\..\Solution\PEPlus\PEPlus.h"
#include "..\..\Solution\PEPdb\PEPdb.h"
#pragma warning( disable : 4996 )

#ifdef _DEBUG 
#	pragma comment(lib, "../../3.lib/x64/Debug/PEPlus.lib")
#	pragma comment(lib, "../../3.lib/x64/Debug/PEPdb.lib")
#else
#	pragma comment(lib, "../../3.lib/x64/Release/PEPlus.lib")
#	pragma comment(lib, "../../3.lib/x64/Release/PEPdb.lib")
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
	CONTEXT ctx;
	RtlCaptureContext(&ctx);
	printf("Function \"F3\"\tcalled, RSP=0x%016I64X\n", ctx.Rsp);

	printf("\n\nCall Stack :\n");

	PVOID arpfns[10];
	USHORT nCapCnt = RtlCaptureStackBackTrace(0, 10, arpfns, NULL);
	for (USHORT i = 0; i < nCapCnt; i++)
	{
		DWORD64 ulCtrlPc = (DWORD64)arpfns[i];
		DWORD64 ulImgBase = FindImageBase((DWORD_PTR)ulCtrlPc);
		if (ulImgBase == 0)
		{
			printf("----> Invalid ControlPC: 0x%016I64X\n", ulCtrlPc);
			return;
		}

		PEPdb* pdi = NULL;
		MOD_DIA_MAP::iterator it = G_MD_MAP.find(ulImgBase);
		if (it == G_MD_MAP.end())
		{
			pdi = LoadInterface(ulImgBase);
			if (pdi != NULL)
				G_MD_MAP.insert(std::make_pair(ulImgBase, pdi));
		}
		else
			pdi = it->second;

		CComBSTR bszFunc = L"<no-name>";
		if (pdi != NULL)
		{
			DWORD dwRVA = (DWORD)(ulCtrlPc - ulImgBase);
			CComPtr<IDiaSymbol> pISymb;
			if( pdi->SESSION->findSymbolByRVA(dwRVA, SymTagNull, &pISymb) == S_OK)
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
				pISymb->get_name(&bszFunc);
				pISymb = 0;
			}
		}
		printf("%d\t%-20S: 0x%016I64X\n", i, bszFunc, ulCtrlPc);
	}
}

void F2()
{
	CONTEXT ctx;
	RtlCaptureContext(&ctx);
	printf("Function \"F2\"\tcalled, RSP=0x%016I64X\n", ctx.Rsp);

	F3();
}


void F1()
{
	CONTEXT ctx;
	RtlCaptureContext(&ctx);
	printf("Function \"F1\"\tcalled, RSP=0x%016I64X\n", ctx.Rsp);

	F2();
}

void _tmain()
{
	CoInitialize(NULL);

	CONTEXT ctx;
	RtlCaptureContext(&ctx);
	printf("Function \"main\"\tcalled, RSP=0x%016I64X\n", ctx.Rsp);

	F1();

	UnloadInterface();
	CoUninitialize();
}
