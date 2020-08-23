#include "stdafx.h"
#include "PEAnals.h"
#include "PEView.h"

#ifdef _WIN64

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

#define UWOP_PUSH_NONVOL		 0	// 1 node
#define UWOP_ALLOC_LARGE		 1	// 2 or 3 nodes
#define UWOP_ALLOC_SMALL		 2	// 1 node
#define UWOP_SET_FPREG			 3	// 1 node
#define UWOP_SAVE_NONVOL		 4	// 2 nodes
#define UWOP_SAVE_NONVOL_FAR	 5	// 3 nodes
#define UWOP_SAVE_XMM128		 8	// 2 nodes
#define UWOP_SAVE_XMM128_FAR	 9	// 3 nodes
#define UWOP_PUSH_MACHFRAME		10	// 1 node
PCTSTR GPSZ_UNW_OPS[] =
{
	_T("PUSH_NONVOL"), _T("ALLOC_LARGE"), _T("ALLOC_SMALL"), _T("SET_FPREG"),
	_T("SAVE_NONVOL"), _T("SAVE_NONVOL_FAR"), _T(""), _T(""),
	_T("SAVE_XMM128"), _T("SAVE_XMM128_FAR"), _T("PUSH_MACHFRAME"), _T("")
};

PCTSTR GPSZ_UNW_REGS[] =
{
	_T("RAX"), _T("RCX"), _T("RDX"), _T("RBX"), _T("RSP"), _T("RBP"), _T("RSI"), _T("RDI"),
	_T("R8"), _T("R9"), _T("R10"), _T("R11"), _T("R12"), _T("R13"), _T("R14"), _T("R15"),
	_T("XMM0"), _T("XMM1"), _T("XMM2"), _T("XMM3"), _T("XMM4"), _T("XMM5"), _T("XMM6"), _T("XMM7"),
	_T("XMM8"), _T("XMM9"), _T("XMM10"), _T("XMM11"), _T("XMM12"), _T("XMM13"), _T("XMM14"), _T("XMM15"),
};


#define GetUnwindCodeEntry(info, index) \
    ((info)->UnwindCode[index])
#define GetLanguageSpecificDataPtr(info) \
    ((PVOID)&GetUnwindCodeEntry((info),((info)->CountOfCodes + 1) & ~1))
#define GetExceptionHandler(base, info) \
    ((PEXCEPTION_HANDLER)((base) + *(PULONG)GetLanguageSpecificDataPtr(info)))
#define GetChainedFunctionEntry(base, info) \
    ((PRUNTIME_FUNCTION)((base) + *(PULONG)GetLanguageSpecificDataPtr(info)))
#define GetExceptionDataPtr(info) \
    ((PVOID)((PULONG)GetLanguageSpecificData(info) + 1)



bool PEAnals::ParseDirEntryException(PPE_NODE pnUp, PIMAGE_DATA_DIRECTORY pdd)
{
	PIMAGE_SECTION_HEADER psh = &m_pshs[pnUp->Index];
	DWORD dwOffset = RVA_TO_OFFSET(psh, pdd->VirtualAddress);

	RTF_MAP rtfs;
	int nItemCnt = pdd->Size / sizeof(RUNTIME_FUNCTION);
	PRUNTIME_FUNCTION prfs = PRUNTIME_FUNCTION(m_pImgView + dwOffset);
	for (int i = 0; i < nItemCnt; i++)
	{
		PRUNTIME_FUNCTION prf = &prfs[i];
		if (prf->BeginAddress == 0)
			continue;

		PRTF_INFO pri = new RTF_INFO(i);
		if (prf->UnwindInfoAddress & RUNTIME_FUNCTION_INDIRECT)
		{
			DWORD dwRtfOff = RVA_TO_OFFSET(psh,
				(prf->UnwindInfoAddress & (~RUNTIME_FUNCTION_INDIRECT)));
			PRUNTIME_FUNCTION pci = PRUNTIME_FUNCTION(m_pImgView + dwRtfOff);
			pri->Parent = pci->BeginAddress;
		}
		else
		{
			short nUwSecIdx = PEPlus::GetSectionIdx(m_pImgView, prf->UnwindInfoAddress);
			ASSERT(nUwSecIdx != INVALID_SECT_IDX);
			PIMAGE_SECTION_HEADER pshu = &m_pshs[nUwSecIdx];

			DWORD dwUwiOff = RVA_TO_OFFSET(pshu, prf->UnwindInfoAddress);
			pri->Unwind = (PUNWIND_INFO)(m_pImgView + dwUwiOff);
			pri->UwSecIdx = nUwSecIdx;
			if (pri->Unwind->Flags & UNW_FLAG_CHAININFO)
			{
				DWORD dwChainOff = dwUwiOff + sizeof(UNWIND_INFO) - sizeof(UNWIND_CODE) +
					((pri->Unwind->CountOfCodes + 1) & ~1) * sizeof(UNWIND_CODE);
				PRUNTIME_FUNCTION pci = PRUNTIME_FUNCTION(m_pImgView + dwChainOff);
				pri->Parent = pci->BeginAddress;
			}
		}
		rtfs.insert(std::make_pair(prf->BeginAddress, pri));
	}

	for (RTF_MAP::iterator it = rtfs.begin(); it != rtfs.end(); it++)
	{
		PRTF_INFO pri = it->second;
		if (pri->RtfNode == NULL)
			InsertRTFNode(pri, pnUp->Node, pnUp->Index, prfs, psh, rtfs, true);
	}
	for (RTF_MAP::iterator it = rtfs.begin(); it != rtfs.end(); it++)
		delete it->second;

	return false;
}



void PEAnals::InsertRTFNode(PRTF_INFO pri, HTREEITEM hUp, short nSectIdx, PRUNTIME_FUNCTION prfs,
							PIMAGE_SECTION_HEADER psh, RTF_MAP& rtfs, bool bIsRoot)
{
	PPE_NODE pnfUp = NULL;
	PRTF_INFO priUp = NULL;
	if (pri->Parent > 0)
	{
		RTF_MAP::iterator iu = rtfs.find(pri->Parent);
		ASSERT(iu != rtfs.end());
		priUp = iu->second;
		if (priUp->RtfNode == NULL)
			InsertRTFNode(priUp, hUp, nSectIdx, prfs, psh, rtfs, false);
		hUp = iu->second->RtfNode->Node;
		pnfUp = iu->second->FncNode;
	}

	PRUNTIME_FUNCTION prf = &prfs[pri->Index];
	CString szName; USES_CONVERSION;
	CComPtr<IDiaSymbol> pISym;
	if (DIA_PDB.Loaded())
	{
		if (DIA_PDB.SESSION->findSymbolByRVA(prf->BeginAddress, SymTagNull, &pISym) == S_OK)
		{
			if (pri->Parent == 0)
			{
				CComBSTR bszName;
				if (pISym->get_name(&bszName) == S_OK)
					szName.Format(L"[%d]%s", pri->Index, bszName);
				else
					szName.Format(L"[%d]FUNC_%08X", pri->Index, prf->BeginAddress);
			}
			else
				szName.Format(L"[%d]BLK_%08X", pri->Index, prf->BeginAddress);
		}
	}
	else
		szName.Format(L"[%d]%s_%08X", pri->Index, 
			(pri->Parent == 0) ? L"FUNC" : L"BLK", prf->BeginAddress);

	DWORD dwOffset = psh->PointerToRawData + pri->Index * sizeof(RUNTIME_FUNCTION);
	pri->RtfNode = InsertStructNode(hUp, nSectIdx, dwOffset, szName, L"RUNTIME_FUNCTION");
	AppendStructMembers(pri->RtfNode);

	bool bInsFunc = true;
	if (priUp != NULL && pri->Index > priUp->Index)
	{
		if (pri->Index > 0 && pri->Unwind != NULL)
		{
			int nIdx = pri->Index;
			while (nIdx > 0 && (prfs[nIdx].BeginAddress == prfs[nIdx - 1].EndAddress))
			{
				if (pri->Parent == prfs[nIdx - 1].BeginAddress)
				{
					bInsFunc = false;
					break;
				}
				nIdx--;
			}
		}
	}
	if (bInsFunc)
	{
		short nCodeIdx = PEPlus::GetSectionIdx(m_pImgView, prf->BeginAddress);
		PIMAGE_SECTION_HEADER pshc = &m_pshs[nCodeIdx];
		DWORD dwCodeOff = RVA_TO_OFFSET(pshc, prf->BeginAddress);
		if (pnfUp == NULL)
			pnfUp = m_pnSects.at(nCodeIdx);
		int nFncLen = (int)(prf->EndAddress - prf->BeginAddress);
		int nImgIdx = (pri->Parent > 0) ? IMG_IDX_BLOCK : IMG_IDX_FUNC;
		pri->FncNode = InsertFieldNode(pnfUp->Node,
			nCodeIdx, dwCodeOff, szName, PE_TYPE::UInt8, nFncLen, nImgIdx);
		pri->FncNode->Kind = NK_FUNC;
		pri->FncNode->SubT |= PE_KIND_CODE;
		if (pISym != 0)
			pri->FncNode->Tag = (LPARAM)(new CComPtr<IDiaSymbol>(pISym));
	}
	else
	{
		while (priUp->FncNode == NULL)
		{
			RTF_MAP::iterator iu = rtfs.find(priUp->Parent);
			ASSERT(iu != rtfs.end());
			priUp = iu->second;
		}
		priUp->FncNode->Size += (prf->EndAddress - prf->BeginAddress);
	}
	if (pISym != 0) pISym = 0;


	if (pri->Unwind != NULL)
	{
		int nImgIdx = IMG_IDX_NODE;
		if (pri->Unwind->Flags & UNW_FLAG_CHAININFO)
		{
			if (pri->Unwind->Flags & (UNW_FLAG_EHANDLER | UNW_FLAG_UHANDLER))
				throw L"체인 정보에 예외 또는 종료 핸들러가 설정되었습니다.";
			nImgIdx = IMG_IDX_CS;
		}
		else
		{
			int nImgOff = pri->Unwind->Flags & (UNW_FLAG_EHANDLER | UNW_FLAG_UHANDLER);
			if (nImgOff > 0)
				nImgIdx = IMG_IDX_LFNO + nImgOff;
		}
		if (nImgIdx != IMG_IDX_NODE)
			m_pView->SetItemImage(pri->RtfNode->Node, nImgIdx, nImgIdx);

		DWORD dwUwiOff = (DWORD)((PBYTE)pri->Unwind - m_pImgView);
		PPE_NODE pnSub = FindNode(pri->RtfNode->Node, L"UnwindInfoAddress");

		PPE_NODE pn = InsertStructNode(pnSub->Node, pri->UwSecIdx, dwUwiOff, 
			L"UnwindInfo", L"UNWIND_INFO", 0, IMG_IDX_RVADIR);
		AppendStructMembers(pn);
		dwUwiOff += (sizeof(UNWIND_INFO) - sizeof(UNWIND_CODE));
		dwUwiOff += ParseUnwindCodes(pri->Unwind, pri->UwSecIdx, dwUwiOff, pn->Node);

		if (pri->Unwind->Flags == 0)
			return;
		if (pri->Unwind->Flags == UNW_FLAG_CHAININFO)
		{
			PPE_NODE pn2 = InsertStructNode(pn->Node, pri->UwSecIdx, dwUwiOff, L"Chained", L"RUNTIME_FUNCTION");
			AppendStructMembers(pn2);
		}
		else
		{
			PPE_NODE pnsub = InsertRVANode(pn->Node, pri->UwSecIdx, dwUwiOff, L"ExceptionHandler");
			DWORD dwHandler = *((PDWORD)(m_pImgView + dwUwiOff));
			dwUwiOff += sizeof(DWORD);
			szName.Empty();
			if (DIA_PDB.SESSION)
			{
				CComPtr<IDiaSymbol> pISym;
				if (DIA_PDB.SESSION->findSymbolByRVA(dwHandler, SymTagNull, &pISym) == S_OK)
				{
					CComBSTR bszName;
					if (pISym->get_name(&bszName) == S_OK)
						szName = bszName;
					pISym = 0;
				}
			}
			else
			{
				PIMAGE_SECTION_HEADER pshc = PEPlus::FindSectHdr(m_pImgView, dwHandler);
				if (pshc != NULL)
				{
					DWORD dwCodeOff = RVA_TO_OFFSET(pshc, dwHandler);
					PBYTE pCodes = m_pImgView + dwCodeOff;
					if (pCodes[0] == 0xFF && pCodes[1] == 0x25)
					{
						DWORD dwIatEnt = *((PDWORD)(pCodes + 2)) + dwHandler + 6;
						szName = PEPlus::GetFuncNameFromIAT(m_pImgView, dwIatEnt);
					}
				}
			}
			if (!szName.IsEmpty())
			{
				UpdateNodeText(pnsub->Node, szName, COL_IDX_INFO, true);
				ParseExceptionData(szName, pn, dwUwiOff);
			}
		}
	}
}

CString PEAnals::GetUnwindCode(PUNWIND_INFO pui, DWORD dwUwiOff, int nIdx, int& nCnt)
{
	PUNWIND_CODE puc = &pui->UnwindCode[nIdx];
	CString sz = GPSZ_UNW_OPS[puc->UnwindOp];

	nCnt = 0;
	switch (puc->UnwindOp)
	{
		case UWOP_PUSH_NONVOL:	// 0	// 1 node
			sz.AppendFormat(_T(": reg=%s -> PUSH %s"), 
				GPSZ_UNW_REGS[puc->OpInfo], GPSZ_UNW_REGS[puc->OpInfo]);
		break;

		case UWOP_ALLOC_LARGE:	// 1	// 2 or 3 nodes
		case UWOP_ALLOC_SMALL:	// 2	// 1 node
		{
			int size = 0;
			if (puc->UnwindOp == UWOP_ALLOC_SMALL)
				size = puc->OpInfo * 8 + 8;
			else
			{
				if (puc->OpInfo == 0)
				{
					size = (puc + 1)->FrameOffset * 8;
					nCnt++;
				}
				else
				{
					size = MAKELONG((puc + 1)->FrameOffset, (puc + 2)->FrameOffset);
					nCnt += 2;
				}
			}
			sz.AppendFormat(_T(": size=0x%X -> SUB RSP, %Xh"), size, size);
		}
		break;

		case UWOP_SAVE_NONVOL:			// 4	// 2 nodes
		case UWOP_SAVE_NONVOL_FAR:		// 5	// 3 nodes
		case UWOP_SAVE_XMM128:			// 8	// 2 nodes
		case UWOP_SAVE_XMM128_FAR:		// 9	// 3 nodes
		{
			int reg = puc->OpInfo;
			if (puc->UnwindOp > UWOP_SAVE_XMM128)
				reg += 16;

			int off = 0;
			if (puc->UnwindOp == UWOP_SAVE_NONVOL || puc->UnwindOp == UWOP_SAVE_XMM128)
			{
				off = (puc + 1)->FrameOffset;
				off *= (puc->UnwindOp == UWOP_SAVE_NONVOL) ? 8 : 16;
				nCnt++;
			}
			else
			{
				off = MAKELONG((puc + 1)->FrameOffset, (puc + 2)->FrameOffset);
				nCnt += 2;
			}
			sz.AppendFormat(_T(": reg=%s, offset=0x%X -> MOV [RSP + %Xh], %s"), 
				GPSZ_UNW_REGS[reg], off, off, GPSZ_UNW_REGS[reg]);
		}
		break;

		case UWOP_SET_FPREG:	// 3	// 1 node
			sz.AppendFormat(_T(": reg=%s, offset=0x%X -> LEA %s, [RSP + %Xh]"),
				GPSZ_UNW_REGS[pui->FrameRegister], pui->FrameOffset * 16,
				GPSZ_UNW_REGS[pui->FrameRegister], pui->FrameOffset * 16);
		break;

		case UWOP_PUSH_MACHFRAME:	//10	// 1 node
			sz.Append(_T(" : Push Machine Frame"));
		break;
	}
	return sz;
}

int PEAnals::ParseUnwindCodes(PUNWIND_INFO pui, short nSectIdx, DWORD dwUwiOff, HTREEITEM hUp)
{
	int nUiSize = 0;
	for (int i = 0; i < pui->CountOfCodes; i++)
	{
		int nCnt = 0;
		CString sz = GetUnwindCode(pui, dwUwiOff, i, nCnt);
		for (int j = 0; j < nCnt + 1; j++)
		{
			CString szName; szName.Format(L"[%d]UnwindCode", i + j);
			PPE_NODE pnuc = InsertStructNode(hUp, nSectIdx, 
				dwUwiOff + nUiSize, szName, L"UNWIND_CODE", 0, IMG_IDX_NODE);
			if (j == nCnt)
				UpdateNodeText(pnuc->Node, sz, COL_IDX_INFO);
			AppendStructMembers(pnuc);
			nUiSize += sizeof(UNWIND_CODE);
		}
		i += nCnt;
	}
	if ((pui->CountOfCodes & 1) > 0)
		nUiSize += sizeof(UNWIND_CODE);
	return nUiSize;
}

void PEAnals::ParseExceptionData(PCWSTR pszHdlrName, PPE_NODE pnui, DWORD dwUwiOff)
{
	int nHdlrType = HDLR_C_SPECIFIC_HANDLER;
	for (; nHdlrType < HDLR_MAX_COUNT; nHdlrType++)
	{
		if (_wcsicmp(pszHdlrName, GPSZ_STD_VC_HDLRS[nHdlrType]) == 0)
			break;
	}
	if (nHdlrType == HDLR_MAX_COUNT)
		return;

	CString sz;
	switch (nHdlrType)
	{
		case HDLR_C_SPECIFIC_HANDLER:
		case HDLR_GS_HANDLER_CHECK_SEH:
		{
			PPE_NODE pntbl = InsertStructNode(pnui->Node, pnui->Index, dwUwiOff, 
				L"ExceptionData: ScopeTable", L"C_SCOPE_TABLE");
			AppendStructMembers(pntbl);

			PC_SCOPE_TABLE pcst = PC_SCOPE_TABLE(m_pImgView + dwUwiOff);
			dwUwiOff += sizeof(DWORD);
			for (int j = 0; j < (int)pcst->Count; j++)
			{
				PC_SCOPE_TABLE_ENTRY pste = (PC_SCOPE_TABLE_ENTRY)(m_pImgView + dwUwiOff);
				sz.Format(L"[%d]Table_%c", j, (pste->JumpTarget == NULL) ? L'T' : L'E');
				PPE_NODE pnsi = InsertStructNode(pntbl->Node, pnui->Index, dwUwiOff, sz, L"C_SCOPE_TABLE_ENTRY");
				AppendStructMembers(pnsi);
				dwUwiOff += sizeof(C_SCOPE_TABLE_ENTRY);

				if (DIA_PDB.Loaded())
				{
					CString szFncName;
					CComPtr<IDiaSymbol> pISym;
					if (DIA_PDB.SESSION->findSymbolByRVA(pste->HandlerAddress, SymTagNull, &pISym) == S_OK)
					{
						CComBSTR bszName;
						if (pISym->get_name(&bszName) == S_OK)
							szFncName = bszName;
						pISym = 0;

						PPE_NODE pnHdlr = FindNode(pnsi->Node, L"HandlerAddress");
						UpdateNodeText(pnHdlr->Node, szFncName, COL_IDX_INFO, true);
					}
				}
			}
			if (nHdlrType == HDLR_C_SPECIFIC_HANDLER)
				break;
		}
		case HDLR_GS_HANDLER_CHECK:
		{
			PPE_NODE pntbl = InsertStructNode(pnui->Node, pnui->Index, dwUwiOff, L"HandlerData", L"GS_HANDLER_DATA");
			AppendStructMembers(pntbl);
		}
		break;

		case HDLR_CXX_FRAME_HANDLER3:
		case HDLR_GS_HANDLER_CHECK_EH:
		{
			PPE_NODE pntbl = InsertRVANode(pnui->Node, pnui->Index, dwUwiOff, L"ExceptionData");

			DWORD dwHdlRva = *PDWORD(m_pImgView + dwUwiOff);
			PIMAGE_SECTION_HEADER push = PEPlus::FindSectHdr(m_pImgView, dwHdlRva);
			DWORD dwFiOff = RVA_TO_OFFSET(push, dwHdlRva);
			PPE_NODE pn3 = InsertStructNode(pntbl->Node, pnui->Index, 
				dwFiOff, L"S_FuncInfo", L"S_FUNC_INFO", 0, IMG_IDX_RVADIR);
			AppendStructMembers(pn3);
			PS_FUNC_INFO pfi = (PS_FUNC_INFO)(m_pImgView + dwFiOff);

			PPE_NODE pn4 = NULL;
			if (pfi->DispUnwindMap > 0)
			{
				pn4 = FindNode(pn3->Node, L"DispUnwindMap");
				push = PEPlus::FindSectHdr(m_pImgView, pfi->DispUnwindMap);
				dwFiOff = RVA_TO_OFFSET(push, pfi->DispUnwindMap);
				for (int i = 0; i < pfi->MaxState; i++)
				{
					sz.Format(L"UnwindMap[%d]", i);
					PPE_NODE pn5 = InsertStructNode(pn4->Node, pn4->Index,
						dwFiOff, sz, L"UnwindMapEntry", 0, IMG_IDX_RVADIR);
					AppendStructMembers(pn5);

					PUnwindMapEntry pum = (PUnwindMapEntry)(m_pImgView + dwFiOff);
					if (pum->Action > 0 && DIA_PDB.SESSION)
					{
						CString szFncName;
						CComPtr<IDiaSymbol> pISym;
						if (DIA_PDB.SESSION->findSymbolByRVA(pum->Action, SymTagNull, &pISym) == S_OK)
						{
							CComBSTR bszName;
							if (pISym->get_name(&bszName) == S_OK)
								szFncName = bszName;
							pISym = 0;

							PPE_NODE pnHdlr = FindNode(pn5->Node, L"Action");
							UpdateNodeText(pnHdlr->Node, szFncName, COL_IDX_INFO, true);
						}
					}
					dwFiOff += sizeof(UnwindMapEntry);
				}
			}

			if (pfi->DispTryBlockMap > 0)
			{
				pn4 = FindNode(pn3->Node, L"DispTryBlockMap");
				push = PEPlus::FindSectHdr(m_pImgView, pfi->DispTryBlockMap);
				dwFiOff = RVA_TO_OFFSET(push, pfi->DispTryBlockMap);
				for (int i = 0; i < (int)pfi->TryBlocks; i++)
				{
					sz.Format(L"TryBlockMap[%d]", i);
					PPE_NODE pn5 = InsertStructNode(pn4->Node, pn4->Index,
						dwFiOff, sz, L"TryBlockMapEntry", 0, IMG_IDX_RVADIR);
					AppendStructMembers(pn5);
					PTryBlockMapEntry ptbm = (PTryBlockMapEntry)(m_pImgView + dwFiOff);

					PPE_NODE pn6 = FindNode(pn5->Node, L"DispHandlerArray");
					push = PEPlus::FindSectHdr(m_pImgView, ptbm->DispHandlerArray);
					DWORD dwDisp = RVA_TO_OFFSET(push, ptbm->DispHandlerArray);
					for (int j = 0; j < ptbm->CatchCnt; j++)
					{
						sz.Format(L"DispHandler[%d]", j);
						PPE_NODE pn7 = InsertStructNode(pn6->Node, pn6->Index,
							dwDisp, sz, L"HandlerType", 0, IMG_IDX_RVADIR);
						AppendStructMembers(pn7);

						PHandlerType pht = (PHandlerType)(m_pImgView + dwDisp);
						if (pht->DispOfHandler > 0 && DIA_PDB.SESSION)
						{
							CString szFncName;
							CComPtr<IDiaSymbol> pISym;
							if (DIA_PDB.SESSION->findSymbolByRVA(pht->DispOfHandler, SymTagNull, &pISym) == S_OK)
							{
								CComBSTR bszName;
								if (pISym->get_name(&bszName) == S_OK)
									szFncName = bszName;
								pISym = 0;

								PPE_NODE pnHdlr = FindNode(pn7->Node, L"DispOfHandler");
								UpdateNodeText(pnHdlr->Node, szFncName, COL_IDX_INFO, true);
							}
						}
						dwDisp += sizeof(HandlerType);
					}
					dwFiOff += sizeof(TryBlockMapEntry);
				}
			}

			if (pfi->DispIPtoStateMap > 0)
			{
				pn4 = FindNode(pn3->Node, L"DispIPtoStateMap");
				push = PEPlus::FindSectHdr(m_pImgView, pfi->DispIPtoStateMap);
				dwFiOff = RVA_TO_OFFSET(push, pfi->DispIPtoStateMap);
				for (int i = 0; i < (int)pfi->IPMapEntries; i++)
				{
					sz.Format(L"IptoStateMap[%d]", i);
					PPE_NODE pn5 = InsertStructNode(pn4->Node, pn4->Index,
						dwFiOff, sz, L"IptoStateMapEntry", 0, IMG_IDX_RVADIR);
					AppendStructMembers(pn5);

					PIptoStateMapEntry pis = (PIptoStateMapEntry)(m_pImgView + dwFiOff);
					if (pis->Ip > 0 && DIA_PDB.SESSION)
					{
						CString szFncName;
						LONG lDisp = 0;
						CComPtr<IDiaSymbol> pISym;
						if (DIA_PDB.SESSION->findSymbolByRVAEx(pis->Ip, SymTagNull, &pISym, &lDisp) == S_OK)
						{
							DIA_SYMTAG tag = SymTagNull;
							pISym->get_symTag((PDWORD)&tag);
							CComBSTR bszName;
							if (tag == SymTagBlock)
							{
								CComPtr<IDiaSymbol> pISym2;
								if (pISym->get_lexicalParent(&pISym2) == S_OK)
								{
									if (pISym2->get_name(&bszName) == S_OK)
										szFncName = bszName;
									pISym2 = 0;
								}
							}
							else
							{
								if (pISym->get_name(&bszName) == S_OK)
									szFncName = bszName;
							}
							if (lDisp != 0)
								szFncName.AppendFormat(L" + 0x%X", lDisp);
							pISym = 0;

							PPE_NODE pnHdlr = FindNode(pn5->Node, L"Ip");
							UpdateNodeText(pnHdlr->Node, szFncName, COL_IDX_INFO, true);
						}
					}
					dwFiOff += sizeof(IptoStateMapEntry);
				}
			}

			if (nHdlrType == HDLR_CXX_FRAME_HANDLER3)
				break;
			dwUwiOff += sizeof(DWORD);
			pntbl = InsertStructNode(pnui->Node, pnui->Index, dwUwiOff, L"HandlerData", L"GS_HANDLER_DATA");
			AppendStructMembers(pntbl);
		}
		break;
	}
}
#endif