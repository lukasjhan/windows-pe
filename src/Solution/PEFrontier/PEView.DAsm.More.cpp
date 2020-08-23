#include "stdafx.h"
#ifndef SHARED_HANDLERS
#	include "PEApp.h"
#endif
#include "PEDoc.h"
#include "PEView.DAsm.h"
#include "PEFrame.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif

PCWSTR GSZ_RGN_TYPES[] =
{
	L"", L"썽크", L"더미", L"블록", L"함수"
};


int CDAsmView::InsertItem(int nIndex, bool bIsAsm, PVOID pTag)
{
	LV_ITEM		lvi;
	lvi.mask	 = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
	lvi.iItem	 = nIndex;
	lvi.iSubItem = 0;
	lvi.iImage	 = I_IMAGECALLBACK;
	lvi.pszText	 = LPSTR_TEXTCALLBACK;
	lvi.lParam	 = (LPARAM)new LINE_INFO(bIsAsm, pTag);
	return GetListCtrl().InsertItem(&lvi);
}

CString CDAsmView::GetMnemonic(DECODED_INS& di)
{
	if (di._flags == FLAG_NOT_DECODABLE)
		return L"DB";

	USES_CONVERSION;
	CString sz;
	if (di._flags & OPAF_PRE_LOCK)
		sz = L"LOCK ";
	else if (di._flags & OPAF_PRE_REP)
	{
		if ((di._opCode == OI_CMPS) || (di._opCode == OI_SCAS))
			sz = L"REPZ ";
		else
			sz = L"REP ";
	}
	else if (di._flags & OPAF_PRE_REPNZ)
		sz = L"REPNZ ";
	if (di._opCode == OI_UNKNOWN)
		sz.Append(L"DB");
	else
		sz.Append(A2T(PEDisAsm::G_MNEMONICS[di._opCode]));
	return sz;
	//if ((META_GET_ISC(di._meta) == ISC_INTEGER) &&
	//	((di._opCode == OI_MOVS) || (di._opCode == OI_CMPS) ||
	//	(di._opCode == OI_STOS) || (di._opCode == OI_LODS) ||
	//	(di._opCode == OI_SCAS)))
	//{
	//	/*
	//	* No operands are needed if the address size is the default one,
	//	* and no segment is overridden, so add the suffix letter,
	//	* to indicate size of operation and continue to next instruction.
	//	*/
	//	//if ((FLAG_GET_ADDRSIZE(di._flags) == m_bits) && (SEGR_IS_DEFAULT(di._seg)))
	//	if (SEGR_IS_DEFAULT(di._seg))
	//	{
	//		switch (di._oprs[0]._size)
	//		{
	//		case OP_SIZE::OPR_SZ8: strcat(szMNemo, "B"); break;
	//		case OP_SIZE::OPR_SZ16: strcat(szMNemo, "W"); break;
	//		case OP_SIZE::OPR_SZ32: strcat(szMNemo, "D"); break;
	//		case OP_SIZE::OPR_SZ64: strcat(szMNemo, "Q"); break;
	//		}
	//		goto $LABEL_ENDPROC;
	//	}
	//}
}

CString CDAsmView::GetOperand(DECODED_INS& di)
{
	UINT isDefault;
	INT64 tmpDisp64;
	UINT64 addrMask = (UINT64)-1;
	UINT8 segment;

	USES_CONVERSION;
	CString sz;
	for (UINT i = 0; ((i < MAX_OPRNDS_COUNT) && (di._oprs[i]._type != O_NONE)); i++)
	{
		if (i > 0)
			sz.Append(L", ");

		switch (di._oprs[i]._type)
		{
			case O_REG:
				sz.Append(A2T(PEDisAsm::G_REGISTERS[di._oprs[i]._regid]));
			break;
			case O_IMM:
				if ((di._opCode == OI_PUSH) && (di._oprs[i]._size != OP_SIZE::OPR_SZ8))
				{
					//FormatSize(szOpers, pdi, i);
					sz.Append(A2T(PEDisAsm::OP_SIZETOSTR[di._oprs[i]._size]));
					sz.Append(L" ");
				}
				if ((di._flags & FLAG_IMM_SIGNED) && (di._oprs[i]._size == OP_SIZE::OPR_SZ8))
				{
					if (di._imm.sbyte < 0)
					{
						sz.AppendFormat(L"-%xh", (UINT8)-di._imm.sbyte);
						break;
					}
				}
				if (di._oprs[i]._size == OP_SIZE::OPR_SZ64)
					sz.AppendFormat(L"%I64xh", di._imm.qword);
				else
					sz.AppendFormat(L"%xh", di._imm.dword);
			break;
			case O_DISP:
				//FormatSize(szOpers, pdi, i);
				sz.Append(A2T(PEDisAsm::OP_SIZETOSTR[di._oprs[i]._size]));
				if (di._oprs[i]._size != OP_SIZE::OPR_SZ0)
					sz.Append(L" PTR ");
				sz.Append(L"[");
				if ((SEGR_GET(di._seg) != REGI_NONE) && !SEGR_IS_DEFAULT(di._seg))
				{
					sz.Append(A2T(PEDisAsm::G_REGISTERS[SEGR_GET(di._seg)]));
					sz.Append(L":");
				}
				tmpDisp64 = di._dispV & addrMask;
				sz.AppendFormat(L"%I64xh]", tmpDisp64);
			break;
			case O_SMEM:
				//FormatSize(szOpers, pdi, i);
				sz.Append(A2T(PEDisAsm::OP_SIZETOSTR[di._oprs[i]._size]));
				if (di._oprs[i]._size != OP_SIZE::OPR_SZ0)
					sz.Append(L" PTR ");
				sz.Append(L"[");
				segment = SEGR_GET(di._seg);
				isDefault = SEGR_IS_DEFAULT(di._seg);
				switch (di._opCode)
				{
					case OI_MOVS:
						isDefault = FALSE;
						if (i == 0)
							segment = R_ES;
						break;
					case OI_CMPS:
						isDefault = FALSE;
						if (i == 1)
							segment = R_ES;
						break;
					case OI_INS:
					case OI_LODS:
					case OI_STOS:
					case OI_SCAS: isDefault = FALSE; break;
				}
				if (!isDefault && (segment != REGI_NONE))
				{
					sz.Append(A2T(PEDisAsm::G_REGISTERS[segment]));
					sz.Append(L":");
				}
				sz.Append(A2T(PEDisAsm::G_REGISTERS[di._oprs[i]._regid]));
				if (di._dispS > OP_SIZE::OPR_SZ0)
				{
					sz.Append(((INT64)di._dispV < 0) ? L"-" : L"+");
					INT64 llTmp = 0LL;
					if ((INT64)di._dispV < 0)
						llTmp = -(INT64)di._dispV;
					else
						llTmp = di._dispV;
					sz.AppendFormat(L"%I64xh", llTmp);
				}
				sz.Append(L"]");
			break;
			case O_MEM:
				//FormatSize(szOpers, pdi, i);
				sz.Append(A2T(PEDisAsm::OP_SIZETOSTR[di._oprs[i]._size]));
				if (di._oprs[i]._size != OP_SIZE::OPR_SZ0)
					sz.Append(L" PTR ");
				sz.Append(L"[");
				if ((SEGR_GET(di._seg) != REGI_NONE) && !SEGR_IS_DEFAULT(di._seg))
				{
					sz.Append(A2T(PEDisAsm::G_REGISTERS[SEGR_GET(di._seg)]));
					sz.Append(L":");
				}
				if (di._index != REGI_NONE)
				{
					sz.Append(A2T(PEDisAsm::G_REGISTERS[di._index]));
					sz.Append(L"+");
				}
				sz.Append(A2T(PEDisAsm::G_REGISTERS[di._oprs[i]._regid]));
				if (di._scale > 0)
				{
					sz.Append(L"*");
					if (di._scale == 1) sz.Append(L"2");
					else if (di._scale == 2) sz.Append(L"4");
					else /* if (di._scale == 8) */ sz.Append(L"8");
				}
				if (di._dispS > OP_SIZE::OPR_SZ0)
				{
					sz.Append(((INT64)di._dispV < 0) ? L"-" : L"+");
					INT64 llTmp = 0LL;
					if ((INT64)di._dispV < 0)
						llTmp = -(INT64)di._dispV;
					else
						llTmp = di._dispV;
					sz.AppendFormat(L"%I64xh", llTmp);
				}
				sz.Append(L"]");
			break;
			case O_PC:
			{
				PBYTE pCtrlPC =  m_pdm->ImageBase + di._imm.sqword + di._offset + di._count;
				sz.Format(L"%sh", PEPlus::GetAddrForm(m_pdm->Is32Bit, pCtrlPC));
			}
			break;
			case O_PTR:
				sz.AppendFormat(L"%xh:%xh", di._imm.ptr.seg, di._imm.ptr.off);
			break;
		}
	}
	return sz;
}

void CDAsmView::UpdateBreakPoint(DWORD dwBpCode, DWORD dwHaltRva, DWORD dwThreadId)
{
	ASSERT(dwHaltRva < m_pdm->ImageSize);
	m_dwDbgThId = dwThreadId;
	ScrollDAsmLine(dwHaltRva, (dwThreadId) ? DASM_BRKPTR : DASM_SETPOS);
}

void CDAsmView::UpdateDAsmItem(DWORD dwBrkRva)
{
	PLINE_INFO pli = NULL;
	int nItem = FindItem(dwBrkRva, pli);
	if (nItem >= 0)
		GetListCtrl().Update(nItem);
}

void CDAsmView::ScrollDAsmLine(DWORD dwWantRva, DASM_POS eDasmPos)
{
	PLINE_INFO pli = NULL;
	int nItem = FindItem(dwWantRva, pli);
	if (nItem < 0)
	{
		AfxMessageBox(L"코드 번지가 이상합니다.");
		return;
	}
	PINT pnItem = NULL;
	if (eDasmPos == DASM_BRKPTR)
		pnItem = &m_nBrkItem;
	else if (eDasmPos == DASM_SETPOS)
		pnItem = &m_nCurItem;
	if (pnItem != NULL && *pnItem >= 0)
	{
		int nOldItem = *pnItem;
		*pnItem = -1;
		GetListCtrl().Update(nOldItem);
	}

	PDBG_DUMMY pdd = NULL;
	if (!pli->IsAsm && ((PDBG_CBASE)pli->Tag)->CodeType > CT_THUNK)
	{
		pdd = (PDBG_DUMMY)pli->Tag;
		if (pdd->CodeCount == 0)
			GetDocument()->SendCommand(TM_DISASSEMBLE, DASM_RANGE, (LPARAM)pdd);

		if (!pli->Expanded)
		{
			for (int i = 0; i < pdd->CodeCount; i++)
				InsertItem(nItem + i + 1, true, pdd->AsmCodes[i]);
			nItem = FindCodeItem(dwWantRva, nItem + 1, pdd->CodeCount);
			if (m_nCurItem > nItem)
				m_nCurItem += pdd->CodeCount;
			if (m_nBrkItem > nItem)
				m_nBrkItem += pdd->CodeCount;
			if (pnItem)
				*pnItem = nItem;
			pli->Expanded = true;
		}
		else
		{
			if (pnItem)
				*pnItem = nItem + 1;
		}
	}
	else
	{
		if (pnItem)
			*pnItem = nItem;
	}

	int nEndItem = -1;
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = (SIF_PAGE | SIF_POS);
	GetListCtrl().GetScrollInfo(SB_VERT, &si);

	if (nItem <= si.nPos)
		nEndItem = nItem;
	else
	{
		if (pdd != NULL)
		{
			if (pdd->CodeCount + 1 <= (int)si.nPage)
				nEndItem = nItem + pdd->CodeCount - 1;
			else
				nEndItem = nItem + (int)si.nPage - 1;
		}
		else
		{
			nEndItem = nItem;
		}
	}
	GetListCtrl().EnsureVisible(nEndItem, FALSE);
	if (pnItem == NULL)
	{
		POSITION pos = GetListCtrl().GetFirstSelectedItemPosition();
		while (pos)
		{
			int nPrvItem = GetListCtrl().GetNextSelectedItem(pos);
			GetListCtrl().SetItemState(nPrvItem, 0, LVIS_SELECTED);
		}
		GetListCtrl().SetItemState(nItem, LVIS_SELECTED, LVIS_SELECTED);
	}
	else
		GetListCtrl().Update(*pnItem);
}

int CDAsmView::GetImageIndex(PLINE_INFO pci, int nItem)
{
	int nImgIdx = -1;
	if (pci->IsAsm || ((PDBG_CBASE)pci->Tag)->CodeType == CT_THUNK)
	{
		PDBG_DASM pasm = (pci->IsAsm) ? (PDBG_DASM)pci->Tag : ((PDBG_THUNK)pci->Tag)->AsmCode;
		bool bIsEntPos = (pasm->Code._offset == m_pdm->EntryPoint);
		bool bIsRrlPos = (pasm->Code._offset + m_pdm->ImageBase == GetDocument()->GetRtlStartUp());
		if (pasm->HasBPoint)
		{
			PDBG_BRKPNT pdb = GetDocument()->GetBreakMap()->
				find(m_pdm->ImageBase + pasm->Code._offset)->second;
			if (pdb->Module == NULL)
				nImgIdx = BPI_DISBP;
			else
			{
				if (pdb->IsTemp)
				{
					if (bIsEntPos || bIsRrlPos)
						nImgIdx = BPI_EP;

				}
				else
				{
					if (pdb->Active)
					{
						if (pdb->HaltIf.IsEmpty())
							nImgIdx = BPI_BP;
						else
							nImgIdx = BPI_CONBP;
					}
					else
						nImgIdx = BPI_DISBP;
				}
			}
		}
		else
		{
			if (pasm->HasBMark)
				nImgIdx = BPI_BMARK;
			else
				nImgIdx = (bIsEntPos || bIsRrlPos) ? BPI_EP : BPI_NOBP;
		}
		if (nItem == m_nBrkItem)
			nImgIdx++;
		else if (nItem == m_nCurItem)
			nImgIdx += 2;
	}
	else
	{
		nImgIdx = (pci->Expanded) ? BPI_CODES_MINUS : BPI_CODES_PLUS;
	}
	return nImgIdx;
}

void CDAsmView::GetDispDAsmStr(PDBG_DASM pasm, int nSubIstm, CString& sz)
{
	DECODED_INS& di = pasm->Code;
	switch (nSubIstm)
	{
		case 0:	//주소
		{
			PBYTE pVAddr = m_pdm->ImageBase + pasm->Code._offset;
			sz = PEPlus::GetAddrForm(m_pdm->Is32Bit, pVAddr);
			if (m_bShowRva)
				sz.AppendFormat(L":%08X", pasm->Code._offset);
		}
		break;
		case 1:	//코드
		{
			for (int j = 0; j < pasm->Code._count; j++)
			{
				DWORD dwVirAddr = m_pdm->CodeSecs[pasm->SecIdx].CodeHdr->VirtualAddress;
				int nCodeOffset = (di._offset - dwVirAddr);
				sz.AppendFormat(L" %02X", *(m_pdm->CodeSecs[pasm->SecIdx].CodeBase + nCodeOffset + j));
			}
		}
		break;
		case 2:	//니모닉
			sz = GetMnemonic(di);
		break;
		case 3:	//오퍼랜드
			if (di._oprs[0]._type != O_NONE)
				sz = GetOperand(di);
		break;
		case 4:	//상세
			if (di._opCode == OP_IID::OI_CALL || di._opCode == OP_IID::OI_JMP ||
				(di._opCode >= OP_IID::OI_JO && di._opCode <= OP_IID::OI_JNLE))
			{
				INT64 ll = 0;
				if (di._oprs[0]._type == O_PC)
				{
					ll = di._imm.sqword;
				}
				else if (di._oprs[0]._type == O_SMEM && di._oprs[0]._regid == REG_IID::R_RIP)
				{
					ll = di._dispV;
				}
				if (ll != 0)
				{
					PBYTE pCtrlPC = m_pdm->ImageBase + di._offset + di._count + ll;
					PDBG_MODULE pdm = NULL;
					if (pCtrlPC > m_pdm->ImageBase && pCtrlPC < m_pdm->ImageBase + m_pdm->ImageSize)
						pdm = m_pdm;
					else
						pdm = GetDocument()->FindModule(pCtrlPC);
					DWORD dwCtrlPC = (DWORD)(pCtrlPC - pdm->ImageBase);
					if (pdm->DiaRef != NULL)
					{
						CComPtr<IDiaSymbol> pISym;
						if (pdm->DiaRef->SESSION->
							findSymbolByRVA(dwCtrlPC, SymTagNull, &pISym) == S_OK)
						{
							CComBSTR bszName;
							pISym->get_name(&bszName);
							sz.Format(L"%s!%s(%08Xh)", pdm->ModuleName, bszName, dwCtrlPC);
							pISym = 0;
						}
					}
				}
				//if (di._opCode == OP_IID::OI_CALL || di._opCode == OP_IID::OI_JMP ||
				//	di._opCode == OP_IID::OI_MOV || di._opCode == OP_IID::OI_LEA)
				//{
				//	if (di._opCode == OP_IID::OI_CALL || di._opCode == OP_IID::OI_JMP)
				//	{
				//		INT64 ll = 0;
				//		if (di._oprs[0]._type == O_PC)
				//			ll = di._imm.sqword;
				//		else if (di._oprs[0]._type == O_SMEM && di._oprs[0]._regi == REG_IID::R_RIP)
				//			ll = (INT64)di._dispV;
				//		if (ll != 0)
				//		{
				//			DWORD dwIp = OFFSET_TO_RVA(psh, (di._offset + di._count));
				//			ll += dwIp;
				//			sz.Format(L"%08X", (LONG)ll);
				//			GetListCtrl().SetItem(nCurIdx, 4, TVIF_TEXT, sz, 0, 0, 0, 0);
				//		}
				//	}
				//	else if (((di._opCode == OP_IID::OI_MOV || di._opCode == OP_IID::OI_LEA) &&
				//		((di._oprs[1]._type == O_SMEM || di._oprs[1]._type == O_MEM) &&
				//		di._oprs[1]._regi == REG_IID::R_RIP) || di._oprs[1]._type == O_PC))
				//	{
				//		INT64 ll = (INT64)di._dispV;
				//		DWORD dwIp = (di._offset + di._count) - psh->PointerToRawData + psh->VirtualAddress;
				//		ll += dwIp;
				//		sz.Format(L"%08X", (LONG)ll);
				//		GetListCtrl().SetItem(nCurIdx, 4, TVIF_TEXT, sz, 0, 0, 0, 0);
				//	}
				//}
			}
		break;
	}
}

void CDAsmView::GetDispCodesStr(PDBG_CBASE pdc, int nSubIstm, CString& sz)
{
	switch (nSubIstm)
	{
		case 0:	//주소
		{
			PBYTE pVAddr = m_pdm->ImageBase + pdc->BeginRVA;
			sz = PEPlus::GetAddrForm(m_pdm->Is32Bit, pVAddr);
			if (m_bShowRva)
				sz.AppendFormat(L":%08X", pdc->BeginRVA);
		}
		break;
		case 1:	//코드
		break;
		case 2:	//니모닉
			sz = GSZ_RGN_TYPES[pdc->CodeType];
		break;
		case 3:	//오퍼랜드
			if (pdc->CodeType >= CT_BLOCK)
			{
				PDBG_FUNC pdf = NULL;
				if (pdc->CodeType == CT_BLOCK)
				{
					PDBG_BLOCK pitr = (PDBG_BLOCK)pdc;
					while (pitr->Parent) pitr = pitr->Parent;
					pdf = (PDBG_FUNC)pitr;
				}
				else
					pdf = (PDBG_FUNC)pdc;
				sz = pdf->FuncName;
			}
		break;
		case 4:	//상세
			sz.Format(L"Code Size: %d(0x%X) Bytes.", pdc->CodeSize, pdc->CodeSize);
		break;
	}
}

int CDAsmView::FindCodeItem(DWORD dwCodeRva, int base, int size, PLINE_INFO* ppInf)
{
	int low = base;
	int high = base + size - 1;

	while (low <= high)
	{
		int i = (low + high) >> 1;
		PLINE_INFO pli = (PLINE_INFO)GetListCtrl().GetItemData(i);
		ASSERT(pli->IsAsm);

		PDBG_DASM pv = (PDBG_DASM)pli->Tag;
		if (dwCodeRva >= pv->Code._offset && dwCodeRva < pv->Code._offset + pv->Code._count)
		{
			if (ppInf != NULL)
				*ppInf = pli;
			return i;
		}

		if (dwCodeRva < pv->Code._offset)
			high = i - 1;
		else
			low  = i + 1;
	}
	return -1;
}

int CDAsmView::FindItem(DWORD dwCodeRva, PLINE_INFO& pInf)
{
	int nItemCnt = GetListCtrl().GetItemCount();
	int low		 = 0;
	int high	 = nItemCnt - 1;

	while (low <= high)
	{
		int i = (low + high) >> 1;
		PLINE_INFO pli = (PLINE_INFO)GetListCtrl().GetItemData(i);
		DWORD dwFrom = 0, dwTo = 0;
		if (pli->IsAsm)
		{
			PDBG_DASM pv = (PDBG_DASM)pli->Tag;
			dwFrom = pv->Code._offset;
			dwTo = dwFrom + pv->Code._count;
		}
		else
		{
			PDBG_CBASE pv = (PDBG_CBASE)pli->Tag;
			dwFrom = pv->BeginRVA;
			dwTo = dwFrom + pv->CodeSize;
		}
		if (dwCodeRva >= dwFrom && dwCodeRva < dwTo)
		{
			if (!pli->IsAsm && pli->Expanded && ((PDBG_CBASE)pli->Tag)->CodeType > CT_THUNK)
			{
				return FindCodeItem(dwCodeRva, i + 1, ((PDBG_DUMMY)pli->Tag)->CodeCount, &pInf);
			}
			pInf = pli;
			return i;
		}

		if (dwCodeRva < dwFrom)
			high = i - 1;
		else
			low  = i + 1;
	}

	return -1;
}
