#include "stdafx.h"
#include <windows.h>
#include <Shlobj.h>

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



////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define HDLR_C_SPECIFIC_HANDLER		0
#define HDLR_GS_HANDLER_CHECK		1
#define HDLR_GS_HANDLER_CHECK_SEH	2
#define HDLR_CXX_FRAME_HANDLER3		3
#define HDLR_GS_HANDLER_CHECK_EH	4
#define HDLR_MAX_COUNT				5
PCTSTR GPSZ_STD_VC_HDLRS[HDLR_MAX_COUNT] =
{
	L"__C_specific_handler",
	L"__GSHandlerCheck",
	L"__GSHandlerCheck_SEH",
	L"__CxxFrameHandler3",
	L"__GSHandlerCheck_EH"
};
////////////////////////////////////////////////////////////////////////////////////////////////////

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
			dwFrmSize += sizeof(PVOID);
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

			case UWOP_SAVE_NONVOL_FAR:	// 5	// 3 nodes
			case UWOP_SAVE_XMM128_FAR:	// 9	// 3 nodes
			j++;
			case UWOP_SAVE_NONVOL:		// 4	// 2 nodes
			case UWOP_SAVE_XMM128:		// 8	// 2 nodes
			j++;
			break;

			case UWOP_PUSH_MACHFRAME:	//10	// 1 node
			break;
		}
	}
	return dwFrmSize;
}

void ShowSEHFrame(PRUNTIME_FUNCTION prf, PBYTE pLangSpec, 
				  DWORD64 ulStackPtr, DWORD64 ulTargetPc, DWORD64 ulImgBase)
{
	DWORD64 ulExpHdlr = *((PDWORD)pLangSpec) + ulImgBase;
	pLangSpec += sizeof(DWORD);
	DWORD64 ulExpData = (DWORD64)pLangSpec;

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

	CComBSTR bszFunc, bszHandler;
	int nHdrType = -1;
	if (pdi != NULL)
	{
		CComPtr<IDiaSymbol> pISymb;
		DWORD dwRVA = (DWORD)(ulExpHdlr - ulImgBase);
		HRESULT	hr = pdi->SESSION->findSymbolByRVA(dwRVA, SymTagNull, &pISymb);
		if (hr == S_OK)
		{
			pISymb->get_name(&bszHandler);
			pISymb = 0;

			nHdrType = HDLR_C_SPECIFIC_HANDLER;
			for (; nHdrType < HDLR_MAX_COUNT; nHdrType++)
			{
				if (_wcsicmp(bszHandler, GPSZ_STD_VC_HDLRS[nHdrType]) == 0)
					break;
			}
			if (nHdrType == HDLR_MAX_COUNT)
				nHdrType = -1;

			hr = pdi->SESSION->findSymbolByRVA(prf->BeginAddress, SymTagNull, &pISymb);
			if (hr == S_OK)
			{
				pISymb->get_name(&bszFunc);
				pISymb = 0;
			}
		}
	}
	printf("==> Function \"%S\" has SEH Frame\n", bszFunc);
	printf("    StackPointer=0x%I64X, FuncAddress=0x%I64X\n", ulStackPtr, ulImgBase + prf->BeginAddress);
	printf("    Handler=0x%I64X (%S)\n", ulExpHdlr, bszHandler);

	if (nHdrType == HDLR_C_SPECIFIC_HANDLER || nHdrType == HDLR_GS_HANDLER_CHECK_SEH)
	{
		PC_SCOPE_TABLE pcst = (PC_SCOPE_TABLE)ulExpData;
		DWORD dwRVA = (DWORD)(ulTargetPc - ulImgBase);
		for (DWORD Index = 0; Index < pcst->Count; Index++)
		{
			C_SCOPE_TABLE_ENTRY& ste = pcst->ScopeRecord[Index];
			if (dwRVA <  ste.BeginAddress || dwRVA >= ste.EndAddress)
				continue;

			if (ste.HandlerAddress == 1)
			{
				printf("      -> ScopeTable[%u] : Except=EXCEPTION_CONTINUE_SEARCH\n", Index);
			}
			else
			{
				CComPtr<IDiaSymbol> pISymb;
				HRESULT	hr = pdi->SESSION->findSymbolByRVA
					(ste.HandlerAddress, SymTagNull, &pISymb);
				if (hr == S_OK)
				{
					pISymb->get_name(&bszFunc);
					pISymb = 0;
				}
				else
					bszFunc = L"<no-named>";

				DWORD64 dwHandler = ste.HandlerAddress + ulImgBase;
				DWORD64 dwJmpTrgt = ste.JumpTarget + ulImgBase;
				if (ste.JumpTarget > 0)
					printf("      -> ScopeTable[%u] : Except=0x%I64X (%S), Target=0x%I64X\n",
						Index, dwHandler, bszFunc, dwJmpTrgt);
				else
					printf("      -> ScopeTable[%u] : Finally=0x%I64X (%S), Target=0\n",
						Index, dwHandler, bszFunc);
			}
		}
	}
}

void WalkSEHFrames()
{
	CONTEXT ctx;
	RtlCaptureContext(&ctx);
	DWORD64 ulCtrlPc = ctx.Rip;
	DWORD64 ulStcPtr = ctx.Rsp;

	for (int nIndex = 0; ulCtrlPc != 0; nIndex++)
	{
		DWORD64 ulImgBase = 0;
		PRUNTIME_FUNCTION prf = LookupRTF(ulCtrlPc, ulImgBase);
		if (ulImgBase == 0)
		{
			printf("----> Invalid ControlPC: 0x%016I64X\n", ulCtrlPc);
			return;
		}

		if (prf == NULL || (prf->UnwindInfoAddress & RUNTIME_FUNCTION_INDIRECT))
		{
			ulCtrlPc = *((PDWORD64)ulStcPtr);
			ulStcPtr += sizeof(DWORD64);
			continue;
		}

		PUNWIND_INFO pui = (PUNWIND_INFO)(prf->UnwindInfoAddress + ulImgBase);
		if (pui->Flags & (UNW_FLAG_EHANDLER | UNW_FLAG_UHANDLER))
		{
			int nUiSize = sizeof(UNWIND_CODE) * pui->CountOfCodes;
			if ((pui->CountOfCodes & 1) > 0)
				nUiSize += sizeof(UNWIND_CODE);
			ShowSEHFrame(prf, (PBYTE)pui->UnwindCode + nUiSize, ulStcPtr, ulCtrlPc, ulImgBase);
		}

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

		ulStcPtr += dwFrmSize;
		ulCtrlPc = *((PDWORD64)ulStcPtr);
		ulStcPtr += sizeof(DWORD64);
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
