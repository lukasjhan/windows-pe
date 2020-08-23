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


UCHAR RtlpUnwindOpSlotTable[] = 
{
	1,          // UWOP_PUSH_NONVOL
	2,          // UWOP_ALLOC_LARGE (or 3, special cased in lookup code)
	1,          // UWOP_ALLOC_SMALL
	1,          // UWOP_SET_FPREG
	2,          // UWOP_SAVE_NONVOL
	3,          // UWOP_SAVE_NONVOL_FAR
	0,          // UWOP_SPARE_CODE1
	0,          // UWOP_SPARE_CODE2
	2,          // UWOP_SAVE_XMM128
	3,          // UWOP_SAVE_XMM128_FAR
	1           // UWOP_PUSH_MACHFRAME
};

PRUNTIME_FUNCTION UnwindPrologue
(
	IN ULONG64 ImageBase,
	IN ULONG64 ControlPc,
	IN ULONG64 FrameBase,
	IN PRUNTIME_FUNCTION FunctionEntry,
	IN OUT PCONTEXT ContextRecord
)
{

	PM128A FloatingAddress;
	PM128A FloatingRegister;
	ULONG FrameOffset;
	ULONG Index;
	PULONG64 IntegerAddress;
	PULONG64 IntegerRegister;
	BOOLEAN MachineFrame;
	ULONG OpInfo;
	ULONG PrologOffset;
	PULONG64 ReturnAddress;
	PULONG64 StackAddress;
	PUNWIND_INFO UnwindInfo;
	ULONG UnwindOp;

	// Process the unwind codes.
	FloatingRegister = &ContextRecord->Xmm0;
	IntegerRegister = &ContextRecord->Rax;
	Index = 0;
	MachineFrame = FALSE;
	PrologOffset = (ULONG)(ControlPc - (FunctionEntry->BeginAddress + ImageBase));
	UnwindInfo = (PUNWIND_INFO)(FunctionEntry->UnwindData + ImageBase);
	while (Index < UnwindInfo->CountOfCodes)
	{
		UnwindOp = UnwindInfo->UnwindCode[Index].UnwindOp;
		OpInfo = UnwindInfo->UnwindCode[Index].OpInfo;
		if (PrologOffset >= UnwindInfo->UnwindCode[Index].CodeOffset)
		{
			switch (UnwindOp)
			{
				case UWOP_PUSH_NONVOL:
					IntegerAddress = (PULONG64)(ContextRecord->Rsp);
					IntegerRegister[OpInfo] = *IntegerAddress;
					ContextRecord->Rsp += 8;
				break;

				case UWOP_ALLOC_LARGE:
					Index += 1;
					FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset;
					if (OpInfo != 0)
					{
						Index += 1;
						FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);
					}
					else
					{
						FrameOffset *= 8;
					}
					ContextRecord->Rsp += FrameOffset;
				break;

				case UWOP_ALLOC_SMALL:
					ContextRecord->Rsp += (OpInfo * 8) + 8;
				break;

				case UWOP_SET_FPREG:
					ContextRecord->Rsp = IntegerRegister[UnwindInfo->FrameRegister];
					ContextRecord->Rsp -= UnwindInfo->FrameOffset * 16;
				break;

				case UWOP_SAVE_NONVOL:
					Index += 1;
					FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset * 8;
					IntegerAddress = (PULONG64)(FrameBase + FrameOffset);
					IntegerRegister[OpInfo] = *IntegerAddress;
				break;

				case UWOP_SAVE_NONVOL_FAR:
					Index += 2;
					FrameOffset = UnwindInfo->UnwindCode[Index - 1].FrameOffset;
					FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);
					IntegerAddress = (PULONG64)(FrameBase + FrameOffset);
					IntegerRegister[OpInfo] = *IntegerAddress;
				break;

				case UWOP_SAVE_XMM128:
					Index += 1;
					FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset * 16;
					FloatingAddress = (PM128A)(FrameBase + FrameOffset);
					FloatingRegister[OpInfo].Low = FloatingAddress->Low;
					FloatingRegister[OpInfo].High = FloatingAddress->High;
				break;

				case UWOP_SAVE_XMM128_FAR:
					Index += 2;
					FrameOffset = UnwindInfo->UnwindCode[Index - 1].FrameOffset;
					FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);
					FloatingAddress = (PM128A)(FrameBase + FrameOffset);
					FloatingRegister[OpInfo].Low = FloatingAddress->Low;
					FloatingRegister[OpInfo].High = FloatingAddress->High;
				break;

				case UWOP_PUSH_MACHFRAME:
					MachineFrame = TRUE;
					ReturnAddress = (PULONG64)(ContextRecord->Rsp);
					StackAddress = (PULONG64)(ContextRecord->Rsp + (3 * 8));
					if (OpInfo != 0)
					{
						ReturnAddress += 1;
						StackAddress += 1;
					}
					ContextRecord->Rip = *ReturnAddress;
					ContextRecord->Rsp = *StackAddress;
				break;

				default:
				_ASSERT(FALSE);
				break;
			}
			Index += 1;
		}
		else
		{
			Index += RtlpUnwindOpSlotTable[UnwindOp];

			switch (UnwindOp)
			{
				case UWOP_ALLOC_LARGE:
					if (OpInfo != 0)
					{
						Index += 1;
					}
				break;

				default:
				break;
			}
		}
	}

	if ((UnwindInfo->Flags & UNW_FLAG_CHAININFO) != 0)
	{
		Index = UnwindInfo->CountOfCodes;
		if ((Index & 1) != 0)
		{
			Index += 1;
		}

		FunctionEntry = (PRUNTIME_FUNCTION)(&UnwindInfo->UnwindCode[Index]);
		return UnwindPrologue(ImageBase, ControlPc, 
					FrameBase, FunctionEntry, ContextRecord);
	}
	else
	{
		if (MachineFrame == FALSE)
		{
			ContextRecord->Rip = *(PULONG64)(ContextRecord->Rsp);
			ContextRecord->Rsp += 8;
		}
		return FunctionEntry;
	}
}

void PrintNovalRegs(PCONTEXT pctx)
{
	printf("\t RBX=0x%I64X, RSP=0x%I64X, RBP=0x%I64X, RSI=0x%I64X, RDI=0x%I64X\n", 
		pctx->Rbx, pctx->Rsp, pctx->Rbp, pctx->Rsi, pctx->Rdi);
	printf("\t R12=0x%I64X, R13=0x%I64X, R14=0x%I64X, R15=0x%I64X\n", 
		pctx->R12, pctx->R13, pctx->R14, pctx->R15);
}

void ShowSEHFrame(PRUNTIME_FUNCTION prf, PBYTE pLangSpec, DWORD64 ulStackPtr,
	DWORD64 ulTargetPc, DWORD64 ulImgBase)
{
	CComBSTR bszFunc, bszHandler;
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
		PSCOPE_TABLE_AMD64 pcst = (PSCOPE_TABLE_AMD64)ulExpData;
		DWORD dwRVA = (DWORD)(ulTargetPc - ulImgBase);
		for (DWORD Index = 0; Index < pcst->Count; Index++)
		{
			if (dwRVA <  pcst->ScopeRecord[Index].BeginAddress ||
				dwRVA >= pcst->ScopeRecord[Index].EndAddress)
				continue;

			if (pcst->ScopeRecord[Index].HandlerAddress != 1)
			{
				CComPtr<IDiaSymbol> pISymb;
				HRESULT	hr = pdi->SESSION->findSymbolByRVA
					(pcst->ScopeRecord[Index].HandlerAddress, SymTagNull, &pISymb);
				if (hr == S_OK)
				{
					pISymb->get_name(&bszFunc);
					pISymb = 0;
				}
				else
					bszFunc = L"<no-named>";

				DWORD64 dwHandler = pcst->ScopeRecord[Index].HandlerAddress + ulImgBase;
				DWORD64 dwJmpTrgt = pcst->ScopeRecord[Index].JumpTarget + ulImgBase;
				if (dwJmpTrgt != ulImgBase)
					printf("      -> ScopeTable[%u] : Except =0x%I64X (%S), Target=0x%I64X\n",
					Index, dwHandler, bszFunc, dwJmpTrgt);
				else
					printf("      -> ScopeTable[%u] : Finally=0x%I64X (%S) , Target=0\n",
					Index, dwHandler, bszFunc);
			}
			else
				printf("      -> ScopeTable[%u] : Except=EXCEPTION_CONTINUE_SEARCH\n", Index);
		}
	}
}

void UnwindSEHFrames()
{
	CONTEXT ctx;
	RtlCaptureContext(&ctx);

	for (int nIndex = 0; ctx.Rip != 0; nIndex++)
	{
		DWORD64 ulImgBase = 0;
		PRUNTIME_FUNCTION prf = LookupRTF(ctx.Rip, ulImgBase);
		if (ulImgBase == 0)
		{
			printf("----> Invalid ControlPC: 0x%016I64X\n", ctx.Rip);
			return;
		}

		if (prf == NULL || (prf->UnwindInfoAddress & RUNTIME_FUNCTION_INDIRECT))
		{
			ctx.Rip = *((PDWORD64)ctx.Rsp);
			ctx.Rsp += sizeof(DWORD64);
			continue;
		}

		PUNWIND_INFO pui = (PUNWIND_INFO)(prf->UnwindInfoAddress + ulImgBase);
		if (pui->Flags & (UNW_FLAG_EHANDLER | UNW_FLAG_UHANDLER))
		{
			int nUiSize = sizeof(UNWIND_CODE) * pui->CountOfCodes;
			if ((pui->CountOfCodes & 1) > 0)
				nUiSize += sizeof(UNWIND_CODE);
			ShowSEHFrame(prf, (PBYTE)pui->UnwindCode + nUiSize, ctx.Rsp, ctx.Rip, ulImgBase);
			PrintNovalRegs(&ctx);
		}
		UnwindPrologue(ulImgBase, ctx.Rip, ctx.Rsp, prf, &ctx);
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
				UnwindSEHFrames();
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
