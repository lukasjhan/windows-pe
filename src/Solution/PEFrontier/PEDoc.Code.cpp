#include "stdafx.h"
#include "PEApp.h"
#include "PEDoc.h"
#include "PEFrame.h"
#include <TlHelp32.h>

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif

#define DISASM_UNIT	256

#ifdef _WIN64
#define SIZE64_PREFIX 0x48
#define ADD_IMM8_OP 0x83
#define ADD_IMM32_OP 0x81
#define JMP_IMM8_OP 0xeb
#define JMP_IMM32_OP 0xe9
#define JMP_IND_OP 0xff
#define LEA_OP 0x8d
#define REP_PREFIX 0xf3
#define POP_OP 0x58
#define RET_OP 0xc3
#define RET_OP_2 0xc2

#define IS_REX_PREFIX(x) (((x) & 0xf0) == 0x40)

UCHAR CPEDoc::G_UWND_OOP_SLOTS[] =
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


PRUNTIME_FUNCTION CPEDoc::LookupRTF(PBYTE pCtrlPC, PDBG_MODULE pdm)
{
	PIMAGE_DATA_DIRECTORY pdd = PEPlus::GetDataDir(pdm->Header, IMAGE_DIRECTORY_ENTRY_EXCEPTION);
	PRUNTIME_FUNCTION prfs = (PRUNTIME_FUNCTION)pdm->ExptData;

	DWORD dwPcRva = (DWORD)(pCtrlPC - pdm->ImageBase);
	int nItemCnt  = pdd->Size / sizeof(RUNTIME_FUNCTION);
	int low = 0;
	int high = nItemCnt - 1;

	PRUNTIME_FUNCTION prf = NULL;
	while (low <= high)
	{
		int i = (low + high) >> 1;
		if (dwPcRva >= prfs[i].BeginAddress && dwPcRva < prfs[i].EndAddress)
		{
			prf = &prfs[i];
			break;
		}

		if (dwPcRva < prfs[i].BeginAddress)
			high = i - 1;
		else
			low = i + 1;
	}
	return prf;
}

PUNWIND_CODE CPEDoc::GetUnwindData(HANDLE hProc, PBYTE puiAddr, UNWIND_INFO& ui)
{
	ReadProcessMemory(hProc, puiAddr, &ui, sizeof(UNWIND_INFO), NULL);
	DWORD dwSize = SIZE_OF_UWND_INFO + ui.CountOfCodes * sizeof(UNWIND_CODE);
	if (ui.CountOfCodes & 1)
		dwSize += sizeof(UNWIND_CODE);
	if ((ui.Flags & UNW_FLAG_CHAININFO) != 0)
		dwSize += sizeof(RUNTIME_FUNCTION);
	PBYTE pucData = (PBYTE)VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	ReadProcessMemory(m_dp.Process, puiAddr + SIZE_OF_UWND_INFO, pucData, dwSize, NULL);
	return (PUNWIND_CODE)pucData;
}

PUNWIND_INFO CPEDoc::LookupPrimaryUnwindInfo(PRUNTIME_FUNCTION prtf, PBYTE pImgBase, PRUNTIME_FUNCTION* pPrimEnt)
{
	UNWIND_INFO ui, *pui = NULL;
	PRUNTIME_FUNCTION pitr = prtf;
	DWORD dwUwndRVA = pitr->UnwindData;
	while (true)
	{
		pui = (PUNWIND_INFO)(pImgBase + dwUwndRVA);
		PUNWIND_CODE pucs = GetUnwindData(m_dp.Process, (PBYTE)pui, ui);
		if ((ui.Flags & UNW_FLAG_CHAININFO) == 0)
		{
			VirtualFree(pucs, 0, MEM_RELEASE);
			break;
		}

		ULONG uIdx = ui.CountOfCodes;
		if ((uIdx & 1) != 0)
			uIdx += 1;
		pitr = (PRUNTIME_FUNCTION)&pucs[uIdx];
		dwUwndRVA = pitr->UnwindData;
		VirtualFree(pucs, 0, MEM_RELEASE);
	}

	*pPrimEnt = pitr;
	return pui;
}

PRUNTIME_FUNCTION CPEDoc::SameFunction(PRUNTIME_FUNCTION prtf, PBYTE pImgBase, ULONG64 pCtrlPc)
{
	PRUNTIME_FUNCTION prtfPrim = NULL;
	ULONG64 uTrgImgBase = 0;

	PUNWIND_INFO pui1 = LookupPrimaryUnwindInfo(prtf, pImgBase, &prtfPrim);

	PDBG_MODULE pdm = FindModule((PBYTE)pCtrlPc);
	if (pdm == NULL)
		return NULL;
	PRUNTIME_FUNCTION prtfTrg = LookupRTF((PBYTE)pCtrlPc, pdm);
	if (prtfTrg == NULL)
		return NULL;
	PUNWIND_INFO pui2 = LookupPrimaryUnwindInfo(prtfTrg, pdm->ImageBase, &prtfPrim);
	if (pui1 == pui2)
		return prtfPrim;
	return NULL;
}

PBYTE CPEDoc::VirtualUnwind(PDBG_MODULE pdm, ULONG64 pCtrlPc, PRUNTIME_FUNCTION prtf, PCONTEXT pctx)
{
	PBYTE pEstFrame = NULL;
	ULONG64 uImgBase = (ULONG64)pdm->ImageBase;
	UNWIND_INFO ui;
	PUNWIND_CODE pucs = GetUnwindData(m_dp.Process, pdm->ImageBase + prtf->UnwindData, ui);
	ULONG uProlOff = (ULONG)(pCtrlPc - (uImgBase + prtf->BeginAddress));

	if (ui.FrameRegister == 0)
	{
		pEstFrame = (PBYTE)pctx->Rsp;
	}
	else if ((uProlOff >= ui.SizeOfProlog) || ((ui.Flags & UNW_FLAG_CHAININFO) != 0))
	{
		pEstFrame = (PBYTE)(&pctx->Rax)[ui.FrameRegister];
		pEstFrame -= ui.FrameOffset * 16;
	}
	else
	{
		ULONG uIdx = 0;
		for (; uIdx < ui.CountOfCodes; uIdx++)
		{
			if (pucs[uIdx].UnwindOp == UWOP_SET_FPREG)
				break;
		}

		if (uProlOff >= pucs[uIdx].CodeOffset)
		{
			pEstFrame = (PBYTE)(&pctx->Rax)[ui.FrameRegister];
			pEstFrame -= ui.FrameOffset * 16;
		}
		else
		{
			pEstFrame = (PBYTE)pctx->Rsp;
		}
	}

	PBYTE pCodeBase = NULL;
	DWORD dwCtlRva = (DWORD)(pCtrlPc - (ULONG64)pdm->ImageBase);
	for (int i = 0; i < (int)pdm->CSecCnt; i++)
	{
		PIMAGE_SECTION_HEADER psh = pdm->CodeSecs[i].CodeHdr;
		if (dwCtlRva >= psh->VirtualAddress &&
			dwCtlRva < psh->VirtualAddress + psh->Misc.VirtualSize)
		{
			pCodeBase = pdm->CodeSecs[i].CodeBase;
			break;
		}
	}
	_ASSERT((pCodeBase != NULL));

	PULONG64 pIntRegs = &pctx->Rax;
	PUCHAR	 NextByte = pCodeBase + dwCtlRva;
	ULONG	 uFrmReg  = 0;
	if ((NextByte[0] == SIZE64_PREFIX) &&
		(NextByte[1] == ADD_IMM8_OP) &&
		(NextByte[2] == 0xc4))
	{
		NextByte += 4;
	}
	else if ((NextByte[0] == SIZE64_PREFIX) &&
		(NextByte[1] == ADD_IMM32_OP) &&
		(NextByte[2] == 0xc4))
	{
		NextByte += 7;
	}
	else if (((NextByte[0] & 0xfe) == SIZE64_PREFIX) && (NextByte[1] == LEA_OP))
	{
		uFrmReg = ((NextByte[0] & 0x1) << 3) | (NextByte[2] & 0x7);
		if ((uFrmReg != 0) && (uFrmReg == ui.FrameRegister))
		{
			if ((NextByte[2] & 0xf8) == 0x60)
			{
				NextByte += 4;
			}
			else if ((NextByte[2] & 0xf8) == 0xa0)
			{
				NextByte += 7;
			}
		}
	}

	while (true)
	{
		if ((NextByte[0] & 0xf8) == POP_OP)
			NextByte += 1;
		else if (IS_REX_PREFIX(NextByte[0]) && ((NextByte[1] & 0xf8) == POP_OP))
			NextByte += 2;
		else
			break;
	}

	bool bInEpilogue = false;
	if ((NextByte[0] == RET_OP) || (NextByte[0] == RET_OP_2) ||
		((NextByte[0] == REP_PREFIX) && (NextByte[1] == RET_OP)))
	{
		bInEpilogue = true;
	}
	else if ((NextByte[0] == JMP_IMM8_OP) || (NextByte[0] == JMP_IMM32_OP))
	{
		ULONG64 uJmpTrg = (ULONG64)NextByte - uImgBase;
		if (NextByte[0] == JMP_IMM8_OP)
			uJmpTrg += 2 + (CHAR)NextByte[1];
		else
			uJmpTrg += 5 + *((LONG UNALIGNED *)&NextByte[1]);

		if (uJmpTrg < prtf->BeginAddress || uJmpTrg >= prtf->EndAddress)
		{
			PRUNTIME_FUNCTION prtf2 = SameFunction(prtf, (PBYTE)uImgBase, uJmpTrg + uImgBase);
			if ((prtf2 == NULL) || (uJmpTrg == prtf2->BeginAddress))
				bInEpilogue = true;
		}
		else if ((uJmpTrg == prtf->BeginAddress) &&
			((ui.Flags & UNW_FLAG_CHAININFO) == 0))
		{
			bInEpilogue = true;
		}
	}
	else if ((NextByte[0] == JMP_IND_OP) && (NextByte[1] == 0x25))
	{
		bInEpilogue = true;
	}
	else if (((NextByte[0] & 0xf8) == SIZE64_PREFIX) &&
		(NextByte[1] == 0xff) && (NextByte[2] & 0x38) == 0x20)
	{
		bInEpilogue = true;
	}

	if (!bInEpilogue)
	{
		UnwindPrologue(pCtrlPc, (ULONG64)pEstFrame, prtf, ui, pucs, uImgBase, pctx);
		VirtualFree(pucs, 0, MEM_RELEASE);
		return pEstFrame;
	}

	LONG lDisp = 0;
	NextByte = pCodeBase + dwCtlRva;
	if ((NextByte[0] & 0xf8) == SIZE64_PREFIX)
	{
		if (NextByte[1] == ADD_IMM8_OP)
		{
			pctx->Rsp += (CHAR)NextByte[3];
			NextByte += 4;
		}
		else if (NextByte[1] == ADD_IMM32_OP)
		{
			lDisp = NextByte[3] | (NextByte[4] << 8);
			lDisp |= (NextByte[5] << 16) | (NextByte[6] << 24);
			pctx->Rsp += lDisp;
			NextByte += 7;

		}
		else if (NextByte[1] == LEA_OP)
		{
			if ((NextByte[2] & 0xf8) == 0x60)
			{
				pctx->Rsp = pIntRegs[uFrmReg];
				pctx->Rsp += (CHAR)NextByte[3];
				NextByte += 4;
			}
			else if ((NextByte[2] & 0xf8) == 0xa0)
			{
				lDisp = NextByte[3] | (NextByte[4] << 8);
				lDisp |= (NextByte[5] << 16) | (NextByte[6] << 24);
				pctx->Rsp = pIntRegs[uFrmReg];
				pctx->Rsp += lDisp;
				NextByte += 7;
			}
		}
	}

	for (;;)
	{
		ULONG64 uIntAddr = 0;
		if ((NextByte[0] & 0xf8) == POP_OP)
		{
			ULONG uRegNum = NextByte[0] & 0x7;
			PULONG64 pIntAddr = (PULONG64)pctx->Rsp;
			ReadProcessMemory(m_dp.Process, pIntAddr, &uIntAddr, sizeof(ULONG64), NULL);
			pIntRegs[uRegNum] = uIntAddr;

			pctx->Rsp += 8;
			NextByte += 1;
		}
		else if (IS_REX_PREFIX(NextByte[0]) && ((NextByte[1] & 0xf8) == POP_OP))
		{
			ULONG uRegNum = ((NextByte[0] & 1) << 3) | (NextByte[1] & 0x7);
			PULONG64 pIntAddr = (PULONG64)pctx->Rsp;
			ReadProcessMemory(m_dp.Process, pIntAddr, &uIntAddr, sizeof(ULONG64), NULL);
			pIntRegs[uRegNum] = uIntAddr;

			pctx->Rsp += 8;
			NextByte += 2;
		}
		else
			break;
	}
	ReadProcessMemory(m_dp.Process, (PBYTE)pctx->Rsp, &pctx->Rip, sizeof(ULONG64), NULL);
	pctx->Rsp += 8;

	VirtualFree(pucs, 0, MEM_RELEASE);
	return pEstFrame;
}

PRUNTIME_FUNCTION CPEDoc::UnwindPrologue(ULONG64 pCtrlPc, ULONG64 ulFrmBase, PRUNTIME_FUNCTION prtf, 
								UNWIND_INFO& ui, PUNWIND_CODE pucs, ULONG64 ulImgBase, PCONTEXT pctx)
{
	bool		 bMachFrm = false;
	PM128A		 pFltRegs = &pctx->Xmm0;
	PULONG64	 pIntRegs = &pctx->Rax;
	ULONG		 uProlOff = (ULONG)(pCtrlPc - (prtf->BeginAddress + ulImgBase));

	ULONG uIdx = 0;
	while (uIdx < ui.CountOfCodes)
	{
		ULONG uUwdOp = pucs[uIdx].UnwindOp;
		ULONG uOpInf = pucs[uIdx].OpInfo;
		ULONG64 uIntAddr = 0;
		if (uProlOff >= pucs[uIdx].CodeOffset)
		{
			switch (uUwdOp)
			{
				case UWOP_PUSH_NONVOL:
				{
					PULONG64 pIntAddr = (PULONG64)(pctx->Rsp);
					ReadProcessMemory(m_dp.Process, pIntAddr, &uIntAddr, sizeof(ULONG64), NULL);
					pIntRegs[uOpInf] = uIntAddr;
					pctx->Rsp += 8;
				}
				break;

				case UWOP_ALLOC_LARGE:
				{
					uIdx += 1;
					ULONG uFrmOff = pucs[uIdx].FrameOffset;
					if (uOpInf != 0)
					{
						uIdx += 1;
						uFrmOff += (pucs[uIdx].FrameOffset << 16);
					}
					else
						uFrmOff *= 8;
					pctx->Rsp += uFrmOff;
				}
				break;

				case UWOP_ALLOC_SMALL:
					pctx->Rsp += (uOpInf * 8) + 8;
				break;

				case UWOP_SET_FPREG:
					pctx->Rsp = pIntRegs[ui.FrameRegister];
					pctx->Rsp -= ui.FrameOffset * 16;
				break;

				case UWOP_SAVE_NONVOL:
				{
					uIdx += 1;
					ULONG uFrmOff = pucs[uIdx].FrameOffset * 8;
					PULONG64 pIntAddr = (PULONG64)(ulFrmBase + uFrmOff);
					ReadProcessMemory(m_dp.Process, pIntAddr, &uIntAddr, sizeof(ULONG64), NULL);
					pIntRegs[uOpInf] = uIntAddr;
				}
				break;

				case UWOP_SAVE_NONVOL_FAR:
				{
					uIdx += 2;
					ULONG uFrmOff = pucs[uIdx - 1].FrameOffset;
					uFrmOff += (pucs[uIdx].FrameOffset << 16);
					PULONG64 pIntAddr = (PULONG64)(ulFrmBase + uFrmOff);
					ReadProcessMemory(m_dp.Process, pIntAddr, &uIntAddr, sizeof(ULONG64), NULL);
					pIntRegs[uOpInf] = uIntAddr;
				}
				break;

				case UWOP_SAVE_XMM128:
				{
					uIdx += 1;
					ULONG uFrmOff = pucs[uIdx].FrameOffset * 16;
					PM128A pFltAddr = (PM128A)(ulFrmBase + uFrmOff);
					M128A FltAddr;
					ReadProcessMemory(m_dp.Process, pFltAddr, &FltAddr, sizeof(M128A), NULL);
					pFltRegs[uOpInf].Low  = FltAddr.Low;
					pFltRegs[uOpInf].High = FltAddr.High;
				}
				break;

				case UWOP_SAVE_XMM128_FAR:
				{
					uIdx += 2;
					ULONG uFrmOff = pucs[uIdx - 1].FrameOffset;
					uFrmOff += (pucs[uIdx].FrameOffset << 16);
					PM128A pFltAddr = (PM128A)(ulFrmBase + uFrmOff);
					M128A FltAddr;
					ReadProcessMemory(m_dp.Process, pFltAddr, &FltAddr, sizeof(M128A), NULL);
					pFltRegs[uOpInf].Low  = FltAddr.Low;
					pFltRegs[uOpInf].High = FltAddr.High;
				}
				break;

				case UWOP_PUSH_MACHFRAME:
				{
					bMachFrm = true;
					PULONG64 pRetAddr = (PULONG64)(pctx->Rsp);
					PULONG64 pStkAddr = (PULONG64)(pctx->Rsp + (3 * 8));
					if (uOpInf != 0)
					{
						pRetAddr++;
						pStkAddr++;
					}
					ReadProcessMemory(m_dp.Process, pRetAddr, &uIntAddr, sizeof(ULONG64), NULL);
					pctx->Rip = uIntAddr;
					ReadProcessMemory(m_dp.Process, pStkAddr, &uIntAddr, sizeof(ULONG64), NULL);
					pctx->Rsp = uIntAddr;
				}
				break;

				default:
					_ASSERT(FALSE);
				break;
			}
			uIdx += 1;
		}
		else
		{
			uIdx += G_UWND_OOP_SLOTS[uUwdOp];
			if (uUwdOp == UWOP_ALLOC_LARGE && uOpInf != 0)
				uIdx += 1;
		}
	}

	PRUNTIME_FUNCTION prfRet = NULL;
	if ((ui.Flags & UNW_FLAG_CHAININFO) != 0)
	{
		uIdx = ui.CountOfCodes;
		if ((uIdx & 1) != 0)
			uIdx += 1;
		prtf = (PRUNTIME_FUNCTION)(&pucs[uIdx]);
		pucs = GetUnwindData(m_dp.Process, (PBYTE)ulImgBase + prtf->UnwindData, ui);
		prfRet = UnwindPrologue(pCtrlPc, ulFrmBase, prtf, ui, pucs, ulImgBase, pctx);
		VirtualFree(pucs, 0, MEM_RELEASE);
	}
	else
	{
		if (!bMachFrm)
		{
			ReadProcessMemory(m_dp.Process, (PBYTE)pctx->Rsp, &pctx->Rip, sizeof(ULONG64), NULL);
			pctx->Rsp += 8;
		}
		prfRet = prtf;
	}
	return prfRet;
}

PBYTE CPEDoc::GetReturnAddr64(PDBG_MODULE pdm, PBYTE pCtrlPC, HANDLE hThread)
{
	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_FULL;
	GetThreadContext(hThread, &ctx);

	PRUNTIME_FUNCTION prf = LookupRTF(pCtrlPC, pdm);
	if (prf == NULL || (prf->UnwindInfoAddress & RUNTIME_FUNCTION_INDIRECT) != 0)
	{
		ReadProcessMemory(m_dp.Process, (PVOID)ctx.Rsp, &pCtrlPC, sizeof(PBYTE), NULL);
		return pCtrlPC;
	}
	VirtualUnwind(pdm, (ULONG64)pCtrlPC, prf, &ctx);
	return (PBYTE)ctx.Rip;
}
#endif

PBYTE CPEDoc::GetReturnAddr32(PDBG_MODULE pdm, PBYTE pCtrlPC, HANDLE hThread)
{
	DWORD pBasePtr = 0;
#ifdef _X86_
	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_CONTROL;
	GetThreadContext(hThread, &ctx);
	pBasePtr = ctx.Ebp;
#else
	WOW64_CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_CONTROL;
	Wow64GetThreadContext(hThread, &ctx);
	pBasePtr = ctx.Ebp;
#endif
	pBasePtr -= sizeof(DWORD);

	DWORD uRetAddr = 0;
	ReadProcessMemory(m_dp.Process, (PBYTE)pBasePtr, &uRetAddr, sizeof(DWORD), NULL);
	return (PBYTE)uRetAddr;
}


PBYTE CPEDoc::GetPEHeader(HANDLE hProcess, PDBG_MODULE pdm)
{
	BYTE peHdr[PAGE_SIZE];
	ReadProcessMemory(hProcess, pdm->ImageBase, peHdr, PAGE_SIZE, NULL);

	DWORD_PTR pImgBase = NULL;
	DWORD dwEntPoint, dwImgSize, dwHdrSize;

	pdm->Is32Bit = PEPlus::Is32bitPE(peHdr);
	if (pdm->Is32Bit)
	{
		PIMAGE_OPTIONAL_HEADER32 poh = PEPlus::GetOptHdr32(peHdr);
		pImgBase	= (DWORD_PTR)poh->ImageBase;
		dwEntPoint	= poh->AddressOfEntryPoint;
		dwImgSize	= poh->SizeOfImage;
		dwHdrSize	= poh->SizeOfHeaders;
	}
	else
	{
		PIMAGE_OPTIONAL_HEADER64 poh = PEPlus::GetOptHdr64(peHdr);
		pImgBase	= (DWORD_PTR)poh->ImageBase;
		dwEntPoint	= poh->AddressOfEntryPoint;
		dwImgSize	= poh->SizeOfImage;
		dwHdrSize	= poh->SizeOfHeaders;
	}
	//if (pImgBase != (DWORD_PTR)pdm->ImageBase)
	//	throw HRESULT_FROM_WIN32(ERROR_CORE_RESOURCE);
	if (pdm->EntryPoint == 0)
		pdm->EntryPoint = dwEntPoint;
	else
	{
		if (dwEntPoint != pdm->EntryPoint)
			throw HRESULT_FROM_WIN32(ERROR_CORE_RESOURCE);
	}
	pdm->ImageSize = dwImgSize;

	PBYTE pHdr = new BYTE[dwHdrSize];
	memcpy(pHdr, peHdr, dwHdrSize);
	return pHdr;
}

void CPEDoc::BuildFunctoin(HANDLE hProcess, PDBG_MODULE pdm)
{
	if (pdm->CodeSecs == 0)
		GetCodeSections(hProcess, pdm);

	PIMAGE_SECTION_HEADER pshs = PEPlus::GetSectHdrs(pdm->Header);
	for (DBG_BREAK_MAP::iterator ib = m_mapBrks.begin(); ib != m_mapBrks.end(); ib++)
	{
		if (ib->second->Module != pdm)
			continue;
		DWORD dwRva = ib->second->HaltRVA;
		for (int i = 0; i < (int)pdm->CSecCnt; i++)
		{
			PIMAGE_SECTION_HEADER psh = &pshs[i];
			if (dwRva >= psh->VirtualAddress &&
				dwRva < psh->VirtualAddress + psh->Misc.VirtualSize)
			{
				dwRva -= psh->VirtualAddress;
				if (pdm->CodeSecs[i].CodeBase[dwRva] == OP_INT3)
					pdm->CodeSecs[i].CodeBase[dwRva] = ib->second->OrgCode;
				break;
			}
		}
	}

	DBG_CODES_MAP codes;
	if (pdm->Is32Bit)
	{
		if (pdm->DiaRef != NULL)
			ParseFuncPDB(hProcess, pdm, codes);
		else
		{
			ParseFuncDirect(hProcess, pdm, codes);
			return;
		}
	}
	else
	{
#ifdef _WIN64
		PIMAGE_DATA_DIRECTORY pdd = PEPlus::GetDataDir
			(pdm->Header, IMAGE_DIRECTORY_ENTRY_EXCEPTION);
		if (pdd == NULL)
			throw E_INVALIDARG;

		PBYTE pExptAddr = pdm->ImageBase + pdd->VirtualAddress;
		pdm->ExptData = (PBYTE)VirtualAlloc(NULL, pdd->Size, MEM_COMMIT, PAGE_READWRITE);
		ReadProcessMemory(hProcess, pExptAddr, pdm->ExptData, pdd->Size, NULL);

		ParsePData(hProcess, pdm, pdd, codes);
#endif
	}

	DBG_CODES_LIST dummies;
	DBG_CODES_MAP::iterator it = codes.begin();
	for (BYTE nSecIdx = 0; nSecIdx < pdm->CSecCnt; nSecIdx++)
	{
		PIMAGE_SECTION_HEADER psh = pdm->CodeSecs[nSecIdx].CodeHdr;
		DWORD dwCurRva = psh->VirtualAddress;
		for (; it != codes.end(); it++)
		{
			PDBG_CBASE pdc = it->second;
			if (pdc->SectIdx != nSecIdx)
				break;

			if (dwCurRva < pdc->BeginRVA)
			{
				DWORD dwDummySize = pdc->BeginRVA - dwCurRva;
				DWORD dwDmyBegin = 0;

				PBYTE pCodes = pdm->CodeSecs[nSecIdx].CodeBase;
				DWORD dwOffset = dwCurRva - psh->VirtualAddress;
				int nBuffSize = (DWORD)dwDummySize;
				DECODED_INS dis[DISASM_UNIT];
				while (nBuffSize > 0)
				{
					int ndiCnt = DISASM_UNIT;
					int nReadBytes = PEDisAsm::DisAssemble(dis, ndiCnt,
										pCodes, dwOffset, nBuffSize, pdm->Is32Bit);
					nBuffSize -= nReadBytes;
					dwOffset += nReadBytes;
					for (int i = 0; i < ndiCnt; i++)
					{
						BOOL bImp = -1;
						DECODED_INS& di = dis[i];
						if (di._opCode == OP_IID::OI_JMP)
						{
							if (pCodes[di._offset] == OP_JMP_IMP && pCodes[di._offset + 1] == 0x25)
								bImp = TRUE;
							else if (pCodes[di._offset] == OP_JMP_IN)
								bImp = FALSE;
						}
						if (bImp >= 0)
						{
							if (dwDmyBegin > 0)
							{
								PDBG_DUMMY pdd = new DBG_DUMMY(dwDmyBegin, 
									dwCurRva - dwDmyBegin, nSecIdx, pdm);
								dummies.push_back(pdd);
								dwDmyBegin = 0;
							}
							PDBG_THUNK pdt = new DBG_THUNK(bImp, dwCurRva, di._count, nSecIdx, pdm);
							PDBG_DASM pda = new DBG_DASM();
							di._offset += psh->VirtualAddress;
							pda->Code	 = di;
							pda->SecIdx  = nSecIdx;
							pdt->AsmCode = pda;
							pdm->ThunkMap.insert(std::make_pair(dwCurRva, pdt));

							PBYTE pChkAddr = pdm->ImageBase + di._offset;
							if (m_mapBrks.find(pChkAddr) != m_mapBrks.end())
								pda->HasBPoint = 1;
							if (m_mapMarks.find(pChkAddr) != m_mapMarks.end())
								pda->HasBMark = 1;
							bImp = -1;
						}
						else
						{
							if (dwDmyBegin == 0)
								dwDmyBegin = dwCurRva;
						}
						dwCurRva += di._count;
					}
				}
				if (dwDmyBegin > 0)
				{
					PDBG_DUMMY pdd = new DBG_DUMMY(dwDmyBegin,
						dwCurRva - dwDmyBegin, nSecIdx, pdm);
					dummies.push_back(pdd);
					dwDmyBegin = 0;
				}
			}
			dwCurRva = pdc->BeginRVA + pdc->CodeSize;
		}
	}

	for (DBG_THUNK_MAP::iterator im = pdm->ThunkMap.begin(); im != pdm->ThunkMap.end(); im++)
		codes.insert(std::make_pair(im->first, im->second));
	for (DBG_CODES_LIST::iterator im = dummies.begin(); im != dummies.end(); im++)
		codes.insert(std::make_pair((*im)->BeginRVA, (*im)));

	pdm->CBaseCnt = (int)codes.size();
	if (pdm->CBaseCnt == 0)
		throw E_INVALIDARG;
	pdm->CBaseList = new PDBG_CBASE[pdm->CBaseCnt];
	int nRIdx = 0;
	for (it = codes.begin(); it != codes.end(); it++)
	{
		pdm->CBaseList[nRIdx++] = it->second;
	}
}

void CPEDoc::DisassembleDummy(PDBG_DUMMY pdd)
{
	PDBG_MODULE pdm = pdd->Module;
	PIMAGE_SECTION_HEADER psh = &(PEPlus::GetSectHdrs(pdm->Header))[pdd->SectIdx];
	PBYTE pCodes = pdm->CodeSecs[pdd->SectIdx].CodeBase;
	DWORD dwOffset = pdd->BeginRVA - psh->VirtualAddress;
	int nBuffSize = (DWORD)pdd->CodeSize;

	DASM_LIST codes;
	DECODED_INS dis[DISASM_UNIT];
	while (nBuffSize > 0)
	{
		int ndiCnt = DISASM_UNIT;
		int nReadBytes = PEDisAsm::DisAssemble
			(dis, ndiCnt, pCodes, dwOffset, nBuffSize, pdm->Is32Bit);
		nBuffSize -= nReadBytes;
		dwOffset += nReadBytes;
		for (int i = 0; i < ndiCnt; i++)
		{
			DECODED_INS& di = dis[i];
			PDBG_DASM pda = new DBG_DASM();
			di._offset += psh->VirtualAddress;
			pda->Code	= di;
			pda->SecIdx = pdd->SectIdx;
			pda->Parent = pdd;
			codes.push_back(pda);

			if (m_mapBrks.find(pdm->ImageBase + di._offset) != m_mapBrks.end())
				pda->HasBPoint = 1;
			if (m_mapMarks.find(pdm->ImageBase + di._offset) != m_mapMarks.end())
				pda->HasBMark = 1;
		}
	}

	pdd->CodeCount = (int)codes.size();
	pdd->AsmCodes = new PDBG_DASM[pdd->CodeCount];
	for (int i = 0; i < pdd->CodeCount; i++)
	{
		pdd->AsmCodes[i] = codes.at(i);
	}
}

#ifdef _WIN64
typedef std::map<int, DWORD> SORT_TMP_MAP;
typedef std::set<int> UWI_LIST;
PBYTE CPEDoc::ParsePData(HANDLE hProcess, PDBG_MODULE pdm, PIMAGE_DATA_DIRECTORY pdd, DBG_CODES_MAP& codes)
{
	PBYTE pExptData = pdm->ExptData;
	PBYTE pUnwindData = NULL;
	PIMAGE_SECTION_HEADER pshUnwind = NULL;
	PRUNTIME_FUNCTION prfs = (PRUNTIME_FUNCTION)pExptData;
	int nItemCnt = pdd->Size / sizeof(RUNTIME_FUNCTION);

	UWI_LIST uwList;
	SORT_TMP_MAP uwMap;
	BYTE nSecIdx = 0;
	for (int i = 0; i < nItemCnt; i++)
	{
		RUNTIME_FUNCTION& rf = prfs[i];
		if (rf.BeginAddress == 0)
			continue;

		PIMAGE_SECTION_HEADER psh = pdm->CodeSecs[nSecIdx].CodeHdr;
		if (rf.BeginAddress >= psh->VirtualAddress + psh->Misc.VirtualSize)
			nSecIdx++;

		DWORD dwCodeSize = rf.EndAddress - rf.BeginAddress;
		bool bChained = false;
		if (rf.UnwindInfoAddress & RUNTIME_FUNCTION_INDIRECT)
		{
			uwList.insert(i);
			bChained = true;
		}
		else
		{
			if (pshUnwind == NULL)
			{
				pshUnwind = PEPlus::FindSectHdr(pdm->Header, rf.UnwindInfoAddress);
				pUnwindData = (PBYTE)VirtualAlloc(NULL,
					pshUnwind->Misc.VirtualSize, MEM_COMMIT, PAGE_READWRITE);
				ReadProcessMemory(hProcess, pdm->ImageBase +
					pshUnwind->VirtualAddress, pUnwindData, pshUnwind->Misc.VirtualSize, NULL);
			}

			DWORD dwUwiOff = rf.UnwindInfoAddress - pshUnwind->VirtualAddress;
			PBYTE pIter = pUnwindData + dwUwiOff;
			PUNWIND_INFO pui = (PUNWIND_INFO)pIter;
			if ((pui->Flags & UNW_FLAG_CHAININFO) != 0)
			{
				pIter += sizeof(UNWIND_INFO) + sizeof(UNWIND_CODE) * (((pui->CountOfCodes + 1) & ~1) - 1);
				PRUNTIME_FUNCTION puf = (PRUNTIME_FUNCTION)pIter;

				if (puf->EndAddress <= prfs[i].BeginAddress)
				{
					PDBG_BLOCK pdb = NULL;
					int j = i - 1, k = i;
					int upidx = -1;
					while (prfs[j].EndAddress == prfs[k].BeginAddress)
					{
						if (memcmp(&prfs[j], puf, sizeof(RUNTIME_FUNCTION)) == 0)
						{
							upidx = j;
							SORT_TMP_MAP::iterator it = uwMap.find(j);
							if (it == uwMap.end())
								break;

							pIter = pUnwindData + it->second;
							pui = (PUNWIND_INFO)pIter;
							pIter += sizeof(UNWIND_INFO) + sizeof(UNWIND_CODE) * (((pui->CountOfCodes + 1) & ~1) - 1);
							puf = (PRUNTIME_FUNCTION)pIter;
						}
						k = j; j--;
					}
					if (upidx >= 0)
					{
						pdb = (PDBG_BLOCK)codes.find(prfs[upidx].BeginAddress)->second;
						pdb->CodeSize += dwCodeSize;
					}
					else
						bChained = true;
				}
				else
					bChained = true;
				uwMap.insert(std::make_pair(i, dwUwiOff));
				if (!bChained)
					continue;
			}
		}
		if (bChained)
		{
			PDBG_BLOCK pdb = new DBG_BLOCK(rf.BeginAddress, dwCodeSize, nSecIdx, pdm);
			codes.insert(std::make_pair(rf.BeginAddress, pdb));
			continue;
		}

		PDBG_FUNC pdf = new DBG_FUNC(rf.BeginAddress, dwCodeSize, nSecIdx, pdm);
		if (pdm->DiaRef != NULL)
		{
			CComPtr<IDiaSymbol>	pIFunc;
			if (pdm->DiaRef->SESSION->findSymbolByRVA
				(rf.BeginAddress, SymTagNull, &pIFunc) == S_OK)
			{
				CComBSTR bszName;
				pIFunc->get_name(&bszName);
				pdf->FuncName = bszName;
				pIFunc = 0;
			}
		}
		if (pdf->FuncName.IsEmpty())
			pdf->FuncName.Format(L"FUNC_%08X", rf.BeginAddress);
		pdm->FuncMap.insert(std::make_pair(rf.BeginAddress, pdf));
		codes.insert(std::make_pair(rf.BeginAddress, pdf));
	}

	for (SORT_TMP_MAP::iterator it = uwMap.begin(); it != uwMap.end(); it++)
	{
		RUNTIME_FUNCTION& rf = prfs[it->first];
		PBYTE pIter = pUnwindData + it->second;
		PUNWIND_INFO pui = (PUNWIND_INFO)pIter;
		pIter += sizeof(UNWIND_INFO) - sizeof(UNWIND_CODE);
		pIter += sizeof(UNWIND_CODE) * ((pui->CountOfCodes + 1) & ~1);

		DBG_CODES_MAP::iterator ic = codes.find(rf.BeginAddress);
		if (ic == codes.end())
			continue;

		PDBG_BLOCK pdb = (PDBG_BLOCK)ic->second;
		PRUNTIME_FUNCTION puf = (PRUNTIME_FUNCTION)pIter;
		ic = codes.find(puf->BeginAddress);
		if (ic == codes.end())
		{
			continue;	//TODO:
		}
		PDBG_BLOCK pbp = (PDBG_BLOCK)ic->second;
		pdb->Parent = pbp;
		if (pbp->Childs == NULL)
			pbp->Childs = new BLOCK_LIST();
		pbp->Childs->push_back(pdb);
	}

	for (UWI_LIST::iterator it = uwList.begin(); it != uwList.end(); it++)
	{
		RUNTIME_FUNCTION& rf = prfs[*it];
		DBG_CODES_MAP::iterator ic = codes.find(rf.BeginAddress);
		PDBG_BLOCK pbp = (PDBG_BLOCK)ic->second;

		DWORD dwRtfOff = (rf.UnwindInfoAddress & (~1)) - pdd->VirtualAddress;
		PRUNTIME_FUNCTION prf = PRUNTIME_FUNCTION(pExptData + dwRtfOff);

		ic = codes.find(prf->BeginAddress);
		if (ic == codes.end())
			throw L"체인 정보에 예외 또는 종료 핸들러가 설정되었습니다.";
		PDBG_BLOCK pbc = (PDBG_BLOCK)ic->second;
		pbc->Parent = pbp;
		if (pbp->Childs == NULL)
			pbp->Childs = new BLOCK_LIST();
		pbp->Childs->push_back(pbc);
	}
	if (pUnwindData != NULL)
		VirtualFree(pUnwindData, 0, MEM_RELEASE);
	return pExptData;
}
#endif

void CPEDoc::ParseFuncPDB(HANDLE hProcess, PDBG_MODULE pdm, DBG_CODES_MAP& codes)
{
	int nFncCnt = 0;
	CString sz; USES_CONVERSION;

	CComPtr<IDiaEnumSymbols> pIEnumCpls;
	if (pdm->DiaRef->PESCOPE->findChildren(SymTagCompiland,
		L"*.obj", nsCaseInRegularExpression, &pIEnumCpls) != S_OK)
		return;

	ULONG ulCelt = 0;
	CComPtr<IDiaSymbol> pICpld;
	while (SUCCEEDED(pIEnumCpls->Next(1, &pICpld, &ulCelt)) && (ulCelt == 1))
	{
		DIA_SYMTAG arTags[2] = { SymTagFunction, SymTagBlock };
		for (int i = 0; i < 2; i++)
		{
			CComPtr<IDiaEnumSymbols> pIEnumSyms;
			if (pICpld->findChildren(arTags[i], NULL, nsNone, &pIEnumSyms) != S_OK)
				continue;

			CComPtr<IDiaSymbol> pIFunc;
			while (SUCCEEDED(pIEnumSyms->Next(1, &pIFunc, &ulCelt)) && ulCelt == 1)
			{
				DWORD dwRVA = 0;
				pIFunc->get_relativeVirtualAddress(&dwRVA);
				ULONGLONG ulSize = 0;
				pIFunc->get_length(&ulSize);
				if (dwRVA == 0 || ulSize == 0)
				{
					pIFunc = 0;
					continue;
				}

				int nSectIdx = 0;
				for (; nSectIdx < (int)pdm->CSecCnt; nSectIdx++)
				{
					PIMAGE_SECTION_HEADER psh = pdm->CodeSecs[nSectIdx].CodeHdr;
					if (dwRVA >= psh->VirtualAddress &&
						dwRVA < psh->VirtualAddress + psh->Misc.VirtualSize)
						break;
				}
				if (nSectIdx == pdm->CSecCnt)
				{
					pIFunc = 0;
					continue;
				}

				if (arTags[i] == SymTagBlock)
				{
					CComPtr<IDiaSymbol> pIUp;
					if (pIFunc->get_lexicalParent(&pIUp) != S_OK)
					{
						pIFunc = 0;
						continue;
					}

					DIA_SYMTAG tag = DIA_SYMTAG::SymTagNull;
					if (pIUp->get_symTag((PDWORD)&tag) == S_OK && tag == SymTagFunction)
					{
						DWORD dwSymId = 0;
						pIUp->get_symIndexId(&dwSymId);
						DBG_FUNC_MAP::iterator it = pdm->FuncMap.find(dwRVA);
						if (it != pdm->FuncMap.end())
						{
							PDBG_BLOCK pdb = new DBG_BLOCK(dwRVA, (DWORD)ulSize, nSectIdx, pdm);
							pdm->FuncMap.insert(std::make_pair(dwRVA, pdb));
							codes.insert(std::make_pair(dwRVA, pdb));

							PDBG_BLOCK pbf = (PDBG_BLOCK)it->second;
							pdb->Parent = pbf;
							if (pbf->Childs == NULL)
								pbf->Childs = new BLOCK_LIST();
							pbf->Childs->push_back(pdb);
						}
						pIUp = 0;
					}
				}
				else
				{
					CComBSTR bszName;
					if (pIFunc->get_name(&bszName) == S_OK)
						sz = bszName;
					else
						sz.Format(L"FUNC_%08X", dwRVA);

					PDBG_FUNC pdf = new DBG_FUNC(dwRVA, (DWORD)ulSize, nSectIdx, pdm);
					pdf->FuncName = bszName;
					pdm->FuncMap.insert(std::make_pair(dwRVA, pdf));
					codes.insert(std::make_pair(dwRVA, pdf));
				}
				pIFunc = 0;
			}
			pIEnumSyms = 0;
		}
		pICpld = 0;
	}
	pIEnumCpls = 0;
}

void CPEDoc::ParseFuncDirect(HANDLE hProcess, PDBG_MODULE pdm, DBG_CODES_MAP& codes)
{
	CString	sz; USES_CONVERSION;
	for (int nSecIdx = 0; nSecIdx < (int)pdm->CSecCnt; nSecIdx++)
	{
		PIMAGE_SECTION_HEADER psh = pdm->CodeSecs[nSecIdx].CodeHdr;
		if (psh->SizeOfRawData == 0)
			continue;
		PDBG_DUMMY pdd = new DBG_DUMMY(psh->VirtualAddress, psh->Misc.VirtualSize, nSecIdx, pdm);
		codes.insert(std::make_pair(psh->VirtualAddress, pdd));
	}
	pdm->CBaseCnt = (int)codes.size();
	if (pdm->CBaseCnt == 0)
		throw E_INVALIDARG;
	pdm->CBaseList = new PDBG_CBASE[pdm->CBaseCnt];
	int nRIdx = 0;
	for (DBG_CODES_MAP::iterator it = codes.begin(); it != codes.end(); it++)
	{
		pdm->CBaseList[nRIdx++] = it->second;
	}
}


PDBG_MODULE CPEDoc::GetHaltDebugInfo(PBYTE pCtlPtr, PDBG_THREAD pdt, bool bIsBreak)
{
	BOOL bIsOK = TRUE;
	PBYTE pStkPtr = NULL;
#ifdef _X86_
	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_FULL;
	GetThreadContext(pdt->Thread, &ctx);
	if (bIsBreak)
		ctx.Eip--;

	pdt->StackPtr = (PBYTE)ctx.Esp;
	pdt->InstPtr = pCtlPtr;

	PDBG_MODULE pdm = FindModule(pCtlPtr);
	if (pdm->CSecCnt == 0)
		GetCodeSections(m_dp.Process, pdm);
	return pdm;
#else
	if (m_dp.Is32Bit)
	{
		WOW64_CONTEXT wctx;
		wctx.ContextFlags = CONTEXT_FULL;
		Wow64GetThreadContext(pdt->Thread, &wctx);
		if (bIsBreak)
			wctx.Eip--;
		pdt->StackPtr = (PBYTE)wctx.Esp;
		pdt->InstPtr = pCtlPtr;

		PDBG_MODULE pdm = FindModule(pCtlPtr);
		if (pdm->CSecCnt == 0)
			GetCodeSections(m_dp.Process, pdm);
		return pdm;
	}

	CONTEXT ctx;
	ctx.ContextFlags = CONTEXT_FULL;
	GetThreadContext(pdt->Thread, &ctx);
	if (bIsBreak)
		ctx.Rip--;

	pdt->StackPtr = (PBYTE)ctx.Rsp;
	pdt->InstPtr  = pCtlPtr;

	DBG_CALL_STACK callStk;
	DWORD dwFrmSize = 0;
	PDBG_MODULE pdmThis = NULL;
	while (pCtlPtr > 0)
	{
		PDBG_MODULE pdm = FindModule(pCtlPtr);
		if (pdm == NULL)
			break;
		if (pdmThis == NULL)
			pdmThis = pdm;

		if (pdm->ExptData == NULL)
		{
			PIMAGE_DATA_DIRECTORY pdd = PEPlus::GetDataDir(pdm->Header, IMAGE_DIRECTORY_ENTRY_EXCEPTION);
			if (pdd == NULL)
				break;
			pdm->ExptData = (PBYTE)VirtualAlloc(NULL, pdd->Size, MEM_COMMIT, PAGE_READWRITE);
			ReadProcessMemory(m_dp.Process, 
				pdm->ImageBase + pdd->VirtualAddress, pdm->ExptData, pdd->Size, NULL);
		}
		if (pdm->CSecCnt == 0)
			GetCodeSections(m_dp.Process, pdm);

		PRUNTIME_FUNCTION prf = LookupRTF(pCtlPtr, pdm);
		if (prf == NULL || (prf->UnwindInfoAddress & RUNTIME_FUNCTION_INDIRECT) != 0)
		{
			ReadProcessMemory(m_dp.Process, pStkPtr, &pCtlPtr, sizeof(PBYTE), NULL);
			pStkPtr += sizeof(PBYTE);
			continue;
		}

		CONTEXT vctx;
		memcpy(&vctx, &ctx, sizeof(ctx));
		PBYTE pEstFrm = VirtualUnwind(pdm, (ULONG64)pCtlPtr, prf, &vctx);

		PDBG_CALLSITE pcs = NULL;
		while (true)
		{
			DBG_CALL_STACK::reverse_iterator it = pdt->CallStack.rbegin();
			if (it == pdt->CallStack.rend())
				break;

			PDBG_CALLSITE pitr = *it;
			if (pEstFrm <= pitr->EstFrame)
			{
				pcs = pitr;
				break;
			}
			pdt->CallStack.pop_back();
			delete pitr;
		}
		if (pcs != NULL)
		{
			if (pcs->ExecAddr == pdm->ImageBase + prf->BeginAddress)
			{
				memcpy(&pcs->Context, &ctx, sizeof(ctx));
				pcs->EstFrame = pEstFrm;
				break;
			}
		}

		CString szName;
		DWORD dwRva = (DWORD)(pCtlPtr - pdm->ImageBase);
		if (pdm->DiaRef != NULL)
		{
			CComPtr<IDiaSymbol> pISym;
			if (pdm->DiaRef->SESSION->findSymbolByRVA(dwRva, SymTagNull, &pISym) == S_OK)
			{
				DIA_SYMTAG tag;
				pISym->get_symTag((PDWORD)&tag);
				if (tag == SymTagBlock)
				{
					CComPtr<IDiaSymbol> pIUpSym;
					pISym->get_lexicalParent(&pIUpSym);
					pISym = pIUpSym;
					pIUpSym = 0;
				}

				CComBSTR bszName;
				if(pISym->get_name(&bszName) == S_OK)
					szName.Format(L"%s!%s", pdm->ModuleName, bszName);
				pISym = 0;
			}
		}
		if (szName.IsEmpty())
			szName.Format(L"Func_%08X", dwRva);
		pcs = new DBG_CALLSITE(pdm->ImageBase + prf->BeginAddress, pEstFrm, &ctx, szName);
		callStk.push_back(pcs);

		pCtlPtr = (PBYTE)vctx.Rip;
		memcpy(&ctx, &vctx, sizeof(vctx));
	}

	for (DBG_CALL_STACK::reverse_iterator it = callStk.rbegin(); it != callStk.rend(); it++)
	{
		pdt->CallStack.push_back(*it);
	}
	return pdmThis;
#endif
}
