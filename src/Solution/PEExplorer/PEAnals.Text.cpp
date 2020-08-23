#include "stdafx.h"
#include "PEAnals.h"
#include "PEView.h"


void PEAnals::ParseTextSection(int nNumOfSec)
{
	if (DIA_PDB.SESSION != NULL)
	{
		BuildThunkFromPDB(true);
		if (m_bIs32Bit)
			BuildFunctionFromPDB();
	}
	else
	{
		for (int i = 0; i < nNumOfSec; i++)
		{
			PIMAGE_SECTION_HEADER psh = &m_pshs[i];
			if ((psh->Characteristics & IMAGE_SCN_CNT_CODE) == 0 || psh->SizeOfRawData == 0)
				continue;

			if (m_bIs32Bit)
			{
				BuildFunctionDirect(psh, m_pnSects.at(i));
			}
			else
			{
				BuildThunkDirect(psh, m_pnSects.at(i));
			}
		}
	}
}

int PEAnals::BuildFunctionDirect(PIMAGE_SECTION_HEADER psh, PPE_NODE pnUp)
{
	PIMAGE_DATA_DIRECTORY pIAT = PEPlus::GetDataDir(m_pImgView, IMAGE_DIRECTORY_ENTRY_IAT);
	DWORD	dwCodeOff = psh->PointerToRawData;
	int		nCodeLen = (int)psh->Misc.VirtualSize;

	CString	sz; USES_CONVERSION;
	DWORD	dwFuncOff = 0;
	int		nBebunILT = -1;
	int		nItemIdx = 0;
	while (nCodeLen > 0)
	{
		DECODED_INS dis[128]; int ndiCnt = 128;
		int nDecSize = PEDisAsm::DisAssemble(dis, ndiCnt, m_pImgView, 
			dwCodeOff, psh->Misc.VirtualSize, m_bIs32Bit);
		dwCodeOff += nDecSize;
		nCodeLen  -= nDecSize;

		for (int i = 0; i < ndiCnt; i++)
		{
			DECODED_INS& di = dis[i];
			DWORD dwNxtCode = di._offset + di._count;

			switch (di._opCode)
			{
			case OP_IID::OI_INT_3:
			{
				if (dwFuncOff == 0)
					continue;
			}
			case OP_IID::OI_RET:
			{
				if (dwFuncOff == 0)
				{
					dwFuncOff = di._offset;
					if (nBebunILT <= 0)
						nBebunILT = 1;
				}
				sz.Format(L"[%d]FUNC_%08X", nItemIdx, OFFSET_TO_RVA(psh, dwFuncOff));
				PPE_NODE pnf = InsertFieldNode(pnUp->Node, pnUp->Index,
					dwFuncOff, sz, PE_TYPE::UInt8, (int)(dwNxtCode - dwFuncOff), IMG_IDX_FUNC);
				pnf->Kind = NK_FUNC;
				pnf->SubT |= PE_KIND_CODE;
				nItemIdx++;
				dwFuncOff = 0;
			}
			break;
			case OP_IID::OI_JMP:
			{
				if (dwFuncOff > 0)
					continue;

				int nImgIdx = -1;
				DWORD dwRVA = OFFSET_TO_RVA(psh, dwNxtCode);
				DWORD dwTargetRVA = 0;
				if (((m_bIs32Bit && di._oprs[0]._type == O_DISP) ||
					(!m_bIs32Bit && di._oprs[0]._type == O_SMEM && di._oprs[0]._regid == REG_IID::R_RIP)) &&
					(*(m_pImgView + di._offset) == 0xFF && *(m_pImgView + di._offset + 1) == 0x25))
				{
					if (di._oprs[0]._type == O_DISP)
						dwTargetRVA = (DWORD)(di._dispV - PEPlus::GetImageBase(m_pImgView));
					else
						dwTargetRVA = dwRVA + (int)di._dispV;
					if (RVA_IN_DIR(pIAT, dwTargetRVA))
					{
						bool bBound = false;
						CString szDllName = PEPlus::GetModNameFromIAT(m_pImgView, dwTargetRVA, &bBound);
						if (bBound)
							sz.Format(L"IMP:%s!Bounded:0x%08X", szDllName, dwTargetRVA);
						else
							sz.Format(L"IMP:%s!%s", szDllName, PEPlus::GetFuncNameFromIAT(m_pImgView, dwTargetRVA));
						nImgIdx = IMG_IDX_IMPORT;
					}
					if (nBebunILT <= 0)
						nBebunILT = 1;
				}
				else if (di._oprs[0]._type == O_PC && *(m_pImgView + di._offset) == 0xE9 && nBebunILT <= 0)
				{
					dwTargetRVA = dwRVA + (int)di._imm.sqword;
					sz.Format(L"ILT:0x%08X", OFFSET_TO_RVA(psh, di._offset));
					nImgIdx = IMG_IDX_THUNK;
					if (nBebunILT < 0)
						nBebunILT = 0;
				}
				if (nImgIdx >= 0)
				{
					PPE_NODE pnTh = InsertCodeNode(pnUp->Node, pnUp->Index, di._offset, sz, di._count, nImgIdx);
					DECODED dec;
					PEDisAsm::DecodedFormat(&di, &dec);
					sz = A2T(dec.Mnemonic);
					if (dec.Operands != NULL)
						sz.AppendFormat(L" %s", A2CT(dec.Operands));
					sz.AppendFormat(L"  ;Target: 0x%08X", dwTargetRVA);
					UpdateNodeText(pnTh->Node, sz, COL_IDX_INFO);
					SetNodeTextColor(pnTh->Node, (nImgIdx == IMG_IDX_THUNK) ? RGB(255, 0, 0) : RGB(0, 0, 255), 0);
				}
				else
				{
					dwFuncOff = di._offset;
					if (nBebunILT <= 0)
						nBebunILT = 1;
				}
			}
			break;
			case OP_IID::OI_NOP:
			case OP_IID::OI_UNKNOWN:
				break;
			default:
			{
				if (dwFuncOff == 0)
				{
					dwFuncOff = di._offset;
				}
				if (nBebunILT <= 0)
					nBebunILT = 1;
			}
			break;
			}
		}
	}
	return nItemIdx;
}

int PEAnals::BuildThunkDirect(PIMAGE_SECTION_HEADER psh, PPE_NODE pnUp)
{
	SYMN_MAP fncMap;
	HTREEITEM hIter = m_pView->GetChildItem(pnUp->Node);
	while (hIter)
	{
		PPE_NODE pn = (PPE_NODE)m_pView->GetItemData(hIter);
		fncMap.insert(std::make_pair(pn->Offset, pn));
		HTREEITEM hSub = m_pView->GetChildItem(hIter);
		while (hSub)
		{
			pn = (PPE_NODE)m_pView->GetItemData(hSub);
			fncMap.insert(std::make_pair(pn->Offset, pn));
			hSub = m_pView->GetNextSiblingItem(hSub);
		}
		hIter = m_pView->GetNextSiblingItem(hIter);
	}

	PIMAGE_DATA_DIRECTORY pIAT = PEPlus::GetDataDir(m_pImgView, IMAGE_DIRECTORY_ENTRY_IAT);
	DWORD dwCodeOff = psh->PointerToRawData;

	CString	sz; USES_CONVERSION;
	int		nItemIdx = 0;
	bool	bIsfirst = true;
	DECODED_INS dis[128]; int ndiCnt = 128;
	for (SYMN_MAP::iterator it = fncMap.begin(); it != fncMap.end(); it++)
	{
		int nSize = (int)(it->second->Offset - dwCodeOff);
		int	nCodeLen = nSize;
		while (nCodeLen > 0)
		{
			int nDecSize = PEDisAsm::DisAssemble(dis, ndiCnt,
				m_pImgView, dwCodeOff, nSize, m_bIs32Bit);
			dwCodeOff += nDecSize;
			nCodeLen -= nDecSize;

			for (int i = 0; i < ndiCnt; i++)
			{
				DECODED_INS& di = dis[i];
				DWORD dwNxtCode = di._offset + di._count;

				if (di._opCode != OP_IID::OI_JMP)
					continue;

				int nImgIdx = -1;
				DWORD dwRVA = OFFSET_TO_RVA(psh, dwNxtCode);
				DWORD dwTargetRVA = 0;
				if (((m_bIs32Bit && di._oprs[0]._type == O_DISP) ||
					(!m_bIs32Bit && di._oprs[0]._type == O_SMEM && di._oprs[0]._regid == REG_IID::R_RIP)) &&
					(*(m_pImgView + di._offset) == 0xFF && *(m_pImgView + di._offset + 1) == 0x25))
				{
					if (di._oprs[0]._type == O_DISP)
						dwTargetRVA = (DWORD)(di._dispV - PEPlus::GetImageBase(m_pImgView));
					else
						dwTargetRVA = dwRVA + (int)di._dispV;
					if (RVA_IN_DIR(pIAT, dwTargetRVA))
					{
						bool bBound = false;
						CString szDllName = PEPlus::GetModNameFromIAT(m_pImgView, dwTargetRVA, &bBound);
						if (bBound)
							sz.Format(L"IMP:%s!Bounded:0x%08X", szDllName, dwTargetRVA);
						else
							sz.Format(L"IMP:%s!%s", szDllName, PEPlus::GetFuncNameFromIAT(m_pImgView, dwTargetRVA));
						nImgIdx = IMG_IDX_IMPORT;
					}
				}
				else if (di._oprs[0]._type == O_PC && *(m_pImgView + di._offset) == 0xE9 && bIsfirst)
				{
					dwTargetRVA = dwRVA + (int)di._imm.sqword;
					sz.Format(L"ILT:0x%08X", OFFSET_TO_RVA(psh, di._offset));
					nImgIdx = IMG_IDX_THUNK;
				}
				if (nImgIdx >= 0)
				{
					PPE_NODE pnTh = InsertCodeNode(pnUp->Node, pnUp->Index, di._offset, sz, di._count, nImgIdx);
					DECODED dec;
					PEDisAsm::DecodedFormat(&di, &dec);
					sz = A2T(dec.Mnemonic);
					if (dec.Operands != NULL)
						sz.AppendFormat(L" %s", A2CT(dec.Operands));
					sz.AppendFormat(L"  ;Target: 0x%08X", dwTargetRVA);
					UpdateNodeText(pnTh->Node, sz, COL_IDX_INFO);
					SetNodeTextColor(pnTh->Node, (nImgIdx == IMG_IDX_THUNK) ? RGB(255, 0, 0) : RGB(0, 0, 255), 0);
					nItemIdx++;
				}
			}
		}
		bIsfirst = false;
	}
	return nItemIdx;
}

int PEAnals::BuildFunctionFromPDB()
{
	int nFncCnt = 0;
	CString sz; USES_CONVERSION;
	SYMN_MAP funcMap;

	CComPtr<IDiaEnumSymbols> pIEnumCpls;
	if (DIA_PDB.PESCOPE->findChildren(SymTagCompiland, 
		L"*.obj", nsCaseInRegularExpression, &pIEnumCpls) != S_OK)
		return nFncCnt;

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

			PIMAGE_SECTION_HEADER pshs = PEPlus::GetSectHdrs(m_pImgView);
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

				short nSectIdx = PEPlus::GetSectionIdx(m_pImgView, dwRVA);
				if (nSectIdx < 0)
				{
					pIFunc = 0;
					continue;
				}
				PIMAGE_SECTION_HEADER psh = &pshs[nSectIdx];
				DWORD dwCodeOff = RVA_TO_OFFSET(psh, dwRVA);
				PPE_NODE pni = m_pnSects.at(nSectIdx);

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
						SYMN_MAP::iterator it = funcMap.find(dwSymId);
						if (it != funcMap.end())
						{
							sz.Format(L"BLOCK_0x%08X", dwRVA);
							PPE_NODE pnb = InsertFieldNode(it->second->Node, nSectIdx,
								dwCodeOff, sz, PE_TYPE::UInt8, (int)ulSize, IMG_IDX_BLOCK);
							pnb->Kind  = NK_FUNC;
							pnb->SubT |= PE_KIND_CODE;
							pnb->Tag   = (LPARAM)(new CComPtr<IDiaSymbol>(pIFunc));
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
					PPE_NODE pnf = InsertFieldNode(pni->Node, nSectIdx,
						dwCodeOff, sz, PE_TYPE::UInt8, (int)ulSize, IMG_IDX_FUNC);
					pnf->Kind = NK_FUNC;
					pnf->SubT |= PE_KIND_CODE;
					pnf->Tag = (LPARAM)(new CComPtr<IDiaSymbol>(pIFunc));

					DWORD dwSymId = 0;
					pIFunc->get_symIndexId(&dwSymId);
					funcMap.insert(std::make_pair(dwSymId, pnf));

					if (m_ilts.find(dwRVA) != m_ilts.end())
					{
						m_pView->SetItemTextColor(pnf->Node, 0, RGB(255, 0, 255));
					}
					nFncCnt++;
				}
				pIFunc = 0;
			}
			pIEnumSyms = 0;
		}
		pICpld = 0;
	}
	pIEnumCpls = 0;
	return nFncCnt;
}

int PEAnals::BuildThunkFromPDB(bool bThunkILT, PCWSTR pszDll)
{
	CString sz; USES_CONVERSION;
	if (bThunkILT)
		sz = L"* Linker *";
	else
		sz.Format(L"Import:%s", pszDll);

	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if(DIA_PDB.PESCOPE->findChildren(SymTagCompiland, sz, nsfCaseInsensitive, &pIEnumSyms) != S_OK)
		return 0;

	int nSymCnt = 0;
	ULONG ulCelt = 0;
	CComPtr<IDiaSymbol> pICompiland;
	if (FAILED(pIEnumSyms->Next(1, &pICompiland, &ulCelt)) || (ulCelt == 0))
	{
		pIEnumSyms = 0;
		return nSymCnt;
	}

	PIMAGE_SECTION_HEADER pshs = PEPlus::GetSectHdrs(m_pImgView);
	CComPtr<IDiaEnumSymbols> pIEnumThunks;
	if(pICompiland->findChildren(SymTagThunk, NULL, nsNone, &pIEnumThunks) != S_OK)
	{
		pICompiland = 0;
		pIEnumSyms = 0;
		return nSymCnt;
	}

	CComPtr<IDiaSymbol> pIThunk;
	while (SUCCEEDED(pIEnumThunks->Next(1, &pIThunk, &ulCelt)) && ulCelt == 1)
	{
		DWORD dwRVA = 0;
		ULONGLONG ulSize = 0;
		THUNK_ORDINAL eto = THUNK_ORDINAL_TRAMP_BRANCHISLAND;

		if (pIThunk->get_relativeVirtualAddress(&dwRVA) == S_OK &&
			pIThunk->get_length(&ulSize) == S_OK &&
			pIThunk->get_thunkOrdinal((PDWORD)&eto) == S_OK)
		{
			if (dwRVA == 0 || ulSize == 0 ||
				((bThunkILT && eto != THUNK_ORDINAL_TRAMP_INCREMENTAL) &&
				(!bThunkILT && eto != THUNK_ORDINAL_NOTYPE)))
			{
				pIThunk = 0;
				continue;
			}

			short nSectIdx = PEPlus::GetSectionIdx(m_pImgView, dwRVA);
			if (nSectIdx < 0)
			{
				pIThunk = 0;
				continue;
			}
			PIMAGE_SECTION_HEADER psh = &pshs[nSectIdx];
			DWORD dwCodeOff = RVA_TO_OFFSET(psh, dwRVA);
			PPE_NODE pni = m_pnSects.at(nSectIdx);

			DECODED_INS di; int ndiCnt = 1;
			PEDisAsm::DisAssemble(&di, ndiCnt, m_pImgView, 
				dwCodeOff, psh->Misc.VirtualSize, m_bIs32Bit);
			if (ulSize != di._count || ulSize != di._count)
			{
				pIThunk = 0;
				continue;
			}

			CString szName;
			DWORD dwTargetRVA = 0;
			int nImgIdx = -1;
			if (bThunkILT)
			{
				//pIThunk->get_targetRelativeVirtualAddress(&dwTargetRVA);
				if (di._oprs[0]._type != OPR_TYPECAT::O_PC)
				{
					pIThunk = 0;
					continue;
				}
				dwTargetRVA = dwRVA + di._count + (int)di._imm.sqword;

				CComPtr<IDiaSymbol> pISym;
				if (DIA_PDB.SESSION->findSymbolByRVA(dwTargetRVA, SymTagNull, &pISym) == S_OK)
				{
					CComBSTR bszName;
					if (pISym->get_name(&bszName) == S_OK)
						szName = bszName;
					m_ilts.insert(dwTargetRVA);
					pISym = 0;
				}
				if (szName.IsEmpty())
					szName.Format(L"0x%08X", dwTargetRVA);
				nImgIdx = IMG_IDX_THUNK;
				sz.Format(L"ILT:%s", szName);
			}
			else
			{
				if ((di._oprs[0]._type != OPR_TYPECAT::O_DISP) &&
					(di._oprs[0]._type != OPR_TYPECAT::O_SMEM || di._oprs[0]._regid != R_RIP))
				{
					pIThunk = 0;
					continue;
				}
				dwTargetRVA = dwRVA + di._count + (int)di._dispV;

				CComBSTR bszName;
				if (pIThunk->get_name(&bszName) == S_OK)
					szName = bszName;
				if (szName.IsEmpty())
					szName.Format(L"0x%08X", dwTargetRVA);
				nImgIdx = IMG_IDX_IMPORT;
				sz.Format(L"IMP:%s!%s", pszDll, szName);
			}

			PPE_NODE pnTh = InsertCodeNode(pni->Node, nSectIdx, dwCodeOff, sz, di._count, nImgIdx);
			DECODED dec;
			PEDisAsm::DecodedFormat(&di, &dec);
			sz = A2T(dec.Mnemonic);
			if (dec.Operands != NULL)
				sz.AppendFormat(L" %s", A2CT(dec.Operands));
			sz.AppendFormat(L"  ;Target: 0x%08X", dwTargetRVA);
			UpdateNodeText(pnTh->Node, sz, COL_IDX_INFO);
			SetNodeTextColor(pnTh->Node, (bThunkILT) ? RGB(255, 0, 0) : RGB(0, 0, 255), 0);
			nSymCnt++;
		}
		pIThunk = 0;
	}
	pIEnumThunks = 0;
	pICompiland = 0;
	pIEnumSyms = 0;

	return nSymCnt;
}
