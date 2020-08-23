#include "stdafx.h"
#include "PEDoc.Types.h"



DBG_VMOBJ::DBG_VMOBJ()
{
	memset(&Mbi, 0, sizeof(Mbi));
	ObjType = PAGE_TYPE::PT_NONE;
	IsNew = FALSE;
}


DBG_DASM::DBG_DASM()
{
	SecIdx = -1;
	Parent = NULL;
	HasBPoint = HasBMark = 0;
}


DBG_CBASE::DBG_CBASE()
{
	CodeType	= CT_NONE;
	BeginRVA	= 0;
	CodeSize	= 0;
	Module		= NULL;
	SectIdx		= 0xFF;
}

DBG_CBASE::DBG_CBASE(DWORD dwBeginRVA, DWORD dwCodeSize, BYTE btSecIdx, DBG_MODULE* pdm)
{
	CodeType	= CT_NONE;
	BeginRVA	= dwBeginRVA;
	CodeSize	= dwCodeSize;
	Module		= pdm;
	SectIdx		= btSecIdx;
}


DBG_THUNK::DBG_THUNK() : DBG_CBASE()
{
	CodeType	= CT_THUNK;
	ThunkType	= 0;
	AsmCode		= NULL;
}

DBG_THUNK::DBG_THUNK(BOOL bType, DWORD dwBeginRVA, DWORD dwCodeSize, BYTE btSecIdx, DBG_MODULE* pdm)
	: DBG_CBASE(dwBeginRVA, dwCodeSize, btSecIdx, pdm)
{
	CodeType = CT_THUNK;
	ThunkType = bType;
	AsmCode = NULL;

	if (pdm->DiaRef != NULL)
	{
		CComPtr<IDiaSymbol>	pISym;
		if (pdm->DiaRef->SESSION->findSymbolByRVA
			(dwBeginRVA, SymTagNull, &pISym) == S_OK)
		{
			CComBSTR bszName;
			if (pISym->get_name(&bszName) == S_OK)
				ThunkName = bszName;
			pISym = 0;
		}
		if (ThunkName.IsEmpty())
			ThunkName.Format(L"Thunk_%08X", dwBeginRVA);
	}
}

DBG_THUNK::~DBG_THUNK()
{
	if (AsmCode != NULL)
		delete AsmCode;
}


DBG_DUMMY::DBG_DUMMY() : DBG_CBASE()
{
	CodeType = CT_DUMMY;
	AsmCodes  = NULL;
	CodeCount = 0;
}

DBG_DUMMY::DBG_DUMMY(DWORD dwBeginRVA, DWORD dwCodeSize, BYTE btSecIdx, DBG_MODULE* pdm)
	: DBG_CBASE(dwBeginRVA, dwCodeSize, btSecIdx, pdm)
{
	CodeType = CT_DUMMY;
	AsmCodes  = NULL;
	CodeCount = 0;
}

DBG_DUMMY::~DBG_DUMMY()
{
	if (AsmCodes != NULL)
	{
		for (int i = 0; i < CodeCount; i++)
			delete AsmCodes[i];
		delete AsmCodes;
	}
}

DBG_BLOCK::DBG_BLOCK() : DBG_DUMMY()
{
	CodeType = CT_BLOCK;
	CodeCount = 0;
	Childs = NULL;
	Parent = NULL;
	Module = NULL;
}

DBG_BLOCK::DBG_BLOCK(DWORD dwBeginRVA, DWORD dwCodeSize, BYTE btSecIdx, DBG_MODULE* pdm)
	: DBG_DUMMY(dwBeginRVA, dwCodeSize, btSecIdx, pdm)
{
	CodeType = CT_BLOCK;
	Childs = NULL;
	Parent = NULL;
}

DBG_BLOCK::~DBG_BLOCK()
{
	if (Childs != NULL)
		delete Childs;
}


DBG_FUNC::DBG_FUNC() : DBG_BLOCK()
{
	CodeType = CT_FUNC;
}

DBG_FUNC::DBG_FUNC(DWORD dwCodeRva, DWORD dwCodeSize, BYTE btSecIdx, DBG_MODULE* pdm)
	: DBG_BLOCK(dwCodeRva, dwCodeSize, btSecIdx, pdm)
{
	CodeType = CT_FUNC;
}



DBG_MODULE::DBG_MODULE()
{
	ImageFile	= INVALID_HANDLE_VALUE;
	ImageMap	= NULL;
	ImageBase	= NULL;
	ImageSize	= 0;
	EntryPoint	= 0;
	DiaRef		= NULL;
	Process		= NULL;

	IsExe		= FALSE;
	Unicode		= TRUE;
	FromPE		= FALSE;
	Is32Bit		= FALSE;

	Header		= NULL;
#ifdef _WIN64
	ExptData	= NULL;
#endif
	CodeSecs	= NULL, CSecCnt = 0;
	CBaseList	= NULL;
	CBaseCnt	= 0;
}

DBG_MODULE::DBG_MODULE(DBG_PROCESS* pdp) : DBG_MODULE()
{
	Process = pdp;
}

DBG_MODULE::~DBG_MODULE()
{
	if (CBaseList != NULL)
	{
		for (int i = 0; i < CBaseCnt; i++)
			delete CBaseList[i];
		delete[] CBaseList;
	}
	if (DiaRef != NULL)
		delete DiaRef;

	if (CodeSecs != NULL)
		delete[] CodeSecs;
#ifdef _WIN64
	if (ExptData != NULL)
		VirtualFree(ExptData, 0, MEM_RELEASE);
#endif
	if (Header != NULL)
		delete [] Header;
}

DBG_CALLSITE::DBG_CALLSITE()
{
	ExecAddr = EstFrame = NULL;
}

DBG_CALLSITE::DBG_CALLSITE(PBYTE pFA, PBYTE pEF, PCONTEXT pCtx, PCWSTR pszName)
{
	ExecAddr = pFA;
	EstFrame = pEF;
	FuncName = pszName;
	memcpy(&Context, pCtx, sizeof(CONTEXT));
}

#ifndef _X86_
DBG_CALLSITE::DBG_CALLSITE(PBYTE pFA, PBYTE pEF, PWOW64_CONTEXT pCtx, PCWSTR pszName)
{
	ExecAddr = pFA;
	EstFrame = pEF;
	FuncName = pszName;
	memcpy(&CtxWow, pCtx, sizeof(WOW64_CONTEXT));
}
#endif


DBG_THREAD::DBG_THREAD()
{
	ThreadId	= 0;
	Thread		= NULL;
	StartAddr	= NULL;
	Priority	= 0;
	IsMain		= Active = InTrap = FALSE;
	ExitCode	= 0;
	Process		= NULL;
	PrevBP		= NULL;

	StackBase	= StackPtr = InstPtr = NULL;
}

DBG_THREAD::DBG_THREAD(DBG_PROCESS* pdp) : DBG_THREAD()
{
	Process = pdp;
}

DBG_THREAD::~DBG_THREAD()
{
	for (DBG_CALL_STACK::iterator it = CallStack.begin(); it != CallStack.end(); it++)
		delete (*it);
}


DBG_PROCESS::DBG_PROCESS() : DBG_MODULE()
{
	ProcessId = 0;
	Process = NULL;
	MainThread = NULL;
	Priority = 0;
	IsExe = 1;
	Active = TRUE;
	ExitCode = 0;
}


DBG_BRKPNT::DBG_BRKPNT()
{
	HaltRVA	= 0;
	OrgCode = 0;
	Active	= 1;
	IsTemp	= 0;
	Loaded  = 1;
	Resvrd  = 0;
	Module	= NULL;
}

DBG_BRKPNT::DBG_BRKPNT(DWORD dwHaltRva, PCWSTR pModPath, int nPathLen, PCWSTR pszLabel) : DBG_BRKPNT()
{
	HaltRVA = dwHaltRva;
	ModPath = new WCHAR[nPathLen + 1];
	wcscpy_s(ModPath, nPathLen + 1, pModPath);
	Loaded = 0;

	if (pszLabel == NULL)
	{
		PWSTR pPos = wcsrchr(ModPath, L'\\');
		if (pPos == NULL) pPos = ModPath;
		else pPos++;
		Label.Format(L"%s:BP_0x%08X", pPos, HaltRVA);
	}
	else
		Label = pszLabel;
}

DBG_BRKPNT::DBG_BRKPNT(DWORD dwHaltRva, PDBG_MODULE pdm, bool bIsTemp) : DBG_BRKPNT()
{
	HaltRVA = dwHaltRva;
	Module	= pdm;
	IsTemp	= bIsTemp;
}

DBG_BRKPNT::DBG_BRKPNT(PBYTE pHaltAddr, PDBG_MODULE pdm, bool bIsTemp)
	: DBG_BRKPNT((DWORD)(pHaltAddr - pdm->ImageBase), pdm, bIsTemp)
{
}

DBG_BRKPNT::~DBG_BRKPNT()
{
	if (!Loaded && ModPath != NULL)
		delete[] ModPath;
}



DBG_BKMARK::DBG_BKMARK()
{
	MarkRVA = 0;
	Module  = NULL;
	Loaded  = true;
}

DBG_BKMARK::DBG_BKMARK(DWORD dwMarkRva, PCWSTR pModPath, int nPathLen, PCWSTR pszLabel)
{
	MarkRVA = dwMarkRva;
	ModPath = new WCHAR[nPathLen + 1];
	wcscpy_s(ModPath, nPathLen + 1, pModPath);
	Loaded  = false;

	if (pszLabel == NULL)
	{
		PWSTR pPos = wcsrchr(ModPath, L'\\');
		if (pPos == NULL) pPos = ModPath;
		else pPos++;
		Label.Format(L"%s:BM_0x%08X", pPos, MarkRVA);
	}
	else
		Label = pszLabel;
}

DBG_BKMARK::DBG_BKMARK(DWORD dwRva, PDBG_MODULE pdm)
{
	MarkRVA = dwRva;
	Module = pdm;
	Label.Format(L"%s:BM_0x%08X", Module->ModuleName, dwRva);
	Loaded = true;
}

DBG_BKMARK::~DBG_BKMARK()
{
	if (!Loaded && ModPath != NULL)
		delete[] ModPath;
}
