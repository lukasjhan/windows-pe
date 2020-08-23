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

void ShowSEHFrame(PRUNTIME_FUNCTION prf, DWORD64 ulExpHdlr, DWORD64 ulExpData,
				  DWORD64 ulStackPtr, DWORD64 ulTargetPc, DWORD64 ulImgBase)
{
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

void PrintNovalRegs(PCONTEXT pctx)
{
	printf("\t RBX=0x%I64X, RSP=0x%I64X, RBP=0x%I64X, RSI=0x%I64X, RDI=0x%I64X\n",
		pctx->Rbx, pctx->Rsp, pctx->Rbp, pctx->Rsi, pctx->Rdi);
	printf("\t R12=0x%I64X, R13=0x%I64X, R14=0x%I64X, R15=0x%I64X\n",
		pctx->R12, pctx->R13, pctx->R14, pctx->R15);
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
		PRUNTIME_FUNCTION prf = RtlLookupFunctionEntry(ulCtrlPc, &ulImgBase, NULL);
		if (prf == NULL)
		{
			ulCtrlPc = ctx.Rip = *((PDWORD64)ulStcPtr);
			ctx.Rsp += sizeof(DWORD64);
			ulStcPtr = ctx.Rsp;
			continue;
		}

		CONTEXT ctx2 = ctx;
		DWORD64 ulHandlerdata = 0,ulEstablisherFrame = 0;
		PEXCEPTION_ROUTINE pfnHdlr = RtlVirtualUnwind
		(
			UNW_FLAG_EHANDLER | UNW_FLAG_UHANDLER,
			ulImgBase,
			ulCtrlPc,
			prf,
			&ctx,
			(PVOID*)&ulHandlerdata,
			&ulEstablisherFrame,
			NULL
		);
		if (pfnHdlr != NULL)
		{
			ShowSEHFrame(prf, (DWORD64)pfnHdlr, ulHandlerdata, 
						ulEstablisherFrame, ulCtrlPc, ulImgBase);
			PrintNovalRegs(&ctx2);
		}

		ulStcPtr = ctx.Rsp;
		ulCtrlPc = ctx.Rip;
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
