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

PRUNTIME_FUNCTION LookupRTF(DWORD64 ulCtrlPc, DWORD64& ulImageBase)
{
	PRUNTIME_FUNCTION prf = NULL;
	if (ulImageBase == 0)
	{
		ulImageBase = FindImageBase(ulCtrlPc);
		if (ulImageBase == 0)
			return prf;
	}

	PIMAGE_NT_HEADERS pnh = PIMAGE_NT_HEADERS(ulImageBase + PIMAGE_DOS_HEADER(ulImageBase)->e_lfanew);
	PIMAGE_DATA_DIRECTORY pdd = &pnh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
	PRUNTIME_FUNCTION prfs = (PRUNTIME_FUNCTION)(ulImageBase + pdd->VirtualAddress);

	DWORD dwPCRva = (DWORD)(ulCtrlPc - ulImageBase);
	int nItemCnt = pdd->Size / sizeof(RUNTIME_FUNCTION);
	int low = 0;
	int high = nItemCnt - 1;

	while (low <= high)
	{
		int i = (low + high) >> 1;
		if (dwPCRva >= prfs[i].BeginAddress && dwPCRva < prfs[i].EndAddress)
		{
			prf = &prfs[i];
			break;
		}

		if (dwPCRva < prfs[i].BeginAddress)
			high = i - 1;
		else
			low = i + 1;
	}
	return prf;
}

DWORD GetFrameSize(PUNWIND_INFO pui)
{
	DWORD dwFrmSize = 0;
	for (int j = 0; j < pui->CountOfCodes; j++)
	{
		PUNWIND_CODE puc = &pui->UnwindCode[j];
		switch (puc->UnwindOp)
		{
			case UWOP_PUSH_NONVOL:	// 0	// 1 node
				dwFrmSize += 8;
			break;

			case UWOP_ALLOC_LARGE:	// 1	// 2 or 3 nodes
			case UWOP_ALLOC_SMALL:	// 2	// 1 node
			{
				int size = 0;
				if (puc->UnwindOp == UWOP_ALLOC_SMALL)
					size = puc->OpInfo * 8 + 8;
				else
				{
					PBYTE pVal = PBYTE(&pui->UnwindCode[j + 1]);
					if (puc->OpInfo == 0)
					{
						size = *PUSHORT(pVal) * 8;
						j++;
					}
					else
					{
						size = *PUINT(pVal);
						j += 2;
					}
				}
				dwFrmSize += size;
			}
			break;

			case UWOP_SAVE_NONVOL_FAR:		// 5	// 3 nodes
			case UWOP_SAVE_XMM128_FAR:		// 9	// 3 nodes
				j++;
			case UWOP_SAVE_NONVOL:			// 4	// 2 nodes
			case UWOP_SAVE_XMM128:			// 8	// 2 nodes
				j++;
			break;

			case UWOP_PUSH_MACHFRAME:	//10	// 1 node
			break;
		}
	}
	return dwFrmSize;
}


void F3()
{
	CONTEXT ctx;
	RtlCaptureContext(&ctx);
	printf("Function \"F3\"\tcalled, RSP=0x%016I64X\n", ctx.Rsp);

	DWORD64 ulCtrlPc = ctx.Rip;
	DWORD64 ulStcPtr = ctx.Rsp;
	printf("\n\nCall Stack :\n");
	printf("Index\tFunction            \tBeginAddr\tStackPointer\tRetAddr\n");

	for (int nIndex = 0; ulCtrlPc != 0; nIndex++)
	{
		DWORD64 ulImgBase = 0;
		PRUNTIME_FUNCTION prf = LookupRTF(ulCtrlPc, ulImgBase);
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

		if (prf == NULL || (prf->UnwindInfoAddress & RUNTIME_FUNCTION_INDIRECT))
		{
			ulCtrlPc = *((PDWORD64)ulStcPtr);
			ulStcPtr += sizeof(DWORD64);
			continue;
		}

		PUNWIND_INFO pui = (PUNWIND_INFO)(prf->UnwindInfoAddress + ulImgBase);
		DWORD dwFrmSize = GetFrameSize(pui);
		if (pui->Flags & UNW_FLAG_CHAININFO)
		{
			do
			{
				int nUiSize = sizeof(UNWIND_CODE) * pui->CountOfCodes;
				if ((pui->CountOfCodes & 1) > 0)
					nUiSize += sizeof(UNWIND_CODE);

				PRUNTIME_FUNCTION prf2 = (PRUNTIME_FUNCTION)((PBYTE)pui->UnwindCode + nUiSize);
				prf2 = LookupRTF(prf2->BeginAddress + ulImgBase, ulImgBase);

				pui = (PUNWIND_INFO)(prf2->UnwindInfoAddress + ulImgBase);
				dwFrmSize += GetFrameSize(pui);
			}
			while (pui->Flags & UNW_FLAG_CHAININFO);
		}

		CComBSTR bszFunc = L"<no-name>";
		if (pdi != NULL)
		{
			CComPtr<IDiaSymbol> pISymb;
			HRESULT	hr = pdi->SESSION->findSymbolByRVA(prf->BeginAddress, SymTagNull, &pISymb);
			if (hr == S_OK)
			{
				pISymb->get_name(&bszFunc);
				pISymb = 0;
			}
		}
		printf("%d\t%-20S\t0x%I64X\t0x%I64X\t0x%I64X\n", nIndex, 
			bszFunc, ulImgBase + prf->BeginAddress, ulStcPtr, ulCtrlPc);

		ulStcPtr += dwFrmSize;
		ulCtrlPc = *((PDWORD64)ulStcPtr);
		ulStcPtr += sizeof(DWORD64);
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
