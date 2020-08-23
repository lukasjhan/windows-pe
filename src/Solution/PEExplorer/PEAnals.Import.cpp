#include "stdafx.h"
#include "PEAnals.h"
#include "PEView.h"


bool PEAnals::ParseDirEntryImport(PPE_NODE pnUp, PIMAGE_DATA_DIRECTORY pdd)
{
	PIMAGE_SECTION_HEADER psh = &m_pshs[pnUp->Index];
	DWORD dwOffset = RVA_TO_OFFSET(psh, pdd->VirtualAddress);
	PIMAGE_IMPORT_DESCRIPTOR pids = PIMAGE_IMPORT_DESCRIPTOR(m_pImgView + dwOffset);
	CString sz; USES_CONVERSION;

	int nItemCnt = 0;
	for (;;)
	{
		PIMAGE_IMPORT_DESCRIPTOR pid = &pids[nItemCnt];
		if (pid->Name == 0)
		{
			sz.Format(L"[%d]NULL", nItemCnt);
			InsertStructNode(pnUp->Node, pnUp->Index, dwOffset, sz, L"IMAGE_IMPORT_DESCRIPTOR", 0, IMG_IDX_XBOX);
			dwOffset += sizeof(IMAGE_IMPORT_DESCRIPTOR);
			break;
		}

		DWORD dwFieldOff = RVA_TO_OFFSET(psh, pid->Name);
		CString szName = A2CT((PSTR)m_pImgView + dwFieldOff);
		sz.Format(L"[%d]%s", nItemCnt, szName);
		
		PPE_NODE pn = InsertStructNode(pnUp->Node, pnUp->Index, dwOffset, sz, L"IMAGE_IMPORT_DESCRIPTOR");
		AppendStructMembers(pn);
		PPE_NODE pnSub = FindNode(pn->Node, L"Name");
		UpdateNodeText(pnSub->Node, szName, COL_IDX_INFO, true);

		if (DIA_PDB.SESSION != 0)
		{
			BuildThunkFromPDB(false, szName);
		}

		pnSub = FindNode(pn->Node, L"OriginalFirstThunk");
		dwFieldOff = RVA_TO_OFFSET(psh, pid->OriginalFirstThunk);
		int nINTCnt = 0;
		for (;; nINTCnt++)
		{
			PPE_NODE pn2 = InsertImpRVARefNode(psh, nINTCnt, dwFieldOff, pnSub);
			if (PE_SCHEMA::GetValue(m_pImgView + pn2->Offset, pn2->Type) == 0)
				break;
			dwFieldOff += pn2->Size;
		}
		sz.Format(L"%d functions Imported", nINTCnt);
		UpdateNodeText(pn->Node, sz, COL_IDX_INFO);

		pnSub = FindNode(pn->Node, L"FirstThunk");
		PIMAGE_SECTION_HEADER psh2 = PEPlus::FindSectHdr(m_pImgView, pid->FirstThunk);
		dwFieldOff = RVA_TO_OFFSET(psh2, pid->FirstThunk);
		DWORD dwINTOff = ((int)pid->TimeDateStamp < 0) ?
			RVA_TO_OFFSET(psh, pid->OriginalFirstThunk) : 0;
		for (int i = 0; i <= nINTCnt; i++)
		{
			PPE_NODE pn2 = InsertImpRVARefNode(psh, i, dwFieldOff, pnSub, dwINTOff);
			dwFieldOff += pn2->Size;
			if (dwINTOff > 0)
				dwINTOff += pn2->Size;
		}

		dwOffset += sizeof(IMAGE_IMPORT_DESCRIPTOR);
		nItemCnt++;
	}
	return true;
}

typedef std::map<DWORD, PIMAGE_IMPORT_DESCRIPTOR> IDESC_OFFS;
bool PEAnals::ParseDirEntryIAT(PPE_NODE pnUp, PIMAGE_DATA_DIRECTORY pdd)
{
	PIMAGE_SECTION_HEADER psh = &m_pshs[pnUp->Index];
	DWORD dwOffset = RVA_TO_OFFSET(psh, pdd->VirtualAddress);

	IDESC_OFFS idcMap;
	PIMAGE_DATA_DIRECTORY pdr = PEPlus::GetDataDir(m_pImgView, IMAGE_DIRECTORY_ENTRY_IMPORT);
	PIMAGE_SECTION_HEADER pish = PEPlus::FindSectHdr(m_pImgView, pdr->VirtualAddress);
	DWORD dwIIDOff = RVA_TO_OFFSET(pish, pdr->VirtualAddress);
	PIMAGE_IMPORT_DESCRIPTOR pids = PIMAGE_IMPORT_DESCRIPTOR(m_pImgView + dwIIDOff);
	for (int i = 0;; i++)
	{
		PIMAGE_IMPORT_DESCRIPTOR pid = &pids[i];
		if (pid->Name == 0)
			break;
		idcMap.insert(std::make_pair(RVA_TO_OFFSET(pish, pid->FirstThunk), pid));
	}
	IMAGE_IMPORT_DESCRIPTOR tid;
	tid.FirstThunk = UINT_MAX;
	idcMap.insert(std::make_pair(tid.FirstThunk, &tid));

	int nImpCnt = (int)idcMap.size();
	int nItemCnt = pdd->Size / ((m_bIs32Bit) ? sizeof(UINT32) : sizeof(UINT64));
	for (int i = 0; i < nItemCnt; i++)
	{
		PIMAGE_IMPORT_DESCRIPTOR pid = NULL;
		int nIdx = 0;
		DWORD dwINTOff = 0;
		IDESC_OFFS::iterator it = idcMap.begin();
		while (nIdx < nImpCnt - 1)
		{
			DWORD dwCur = it->first;
			pid = it->second;
			it++;
			DWORD dwNxt = it->first;
			if (dwOffset >= dwCur && dwOffset < dwNxt)
			{
				dwINTOff = dwOffset - dwCur;
				break;
			}
			nIdx++;
		}

		if ((int)pid->TimeDateStamp < 0)
			dwINTOff = RVA_TO_OFFSET(psh, pid->OriginalFirstThunk) + dwINTOff;
		else
			dwINTOff = 0;
		PPE_NODE pn = InsertImpRVARefNode(pish, i, dwOffset, pnUp, dwINTOff);
		dwOffset += pn->Size;
	}
	return false;
}

bool PEAnals::ParseDirEntryBoundImport(PPE_NODE pnUp, PIMAGE_DATA_DIRECTORY pdd)
{
	DWORD dwOffset = (pnUp->Index == INVALID_SECT_IDX) ? pdd->VirtualAddress :
		RVA_TO_OFFSET(&m_pshs[pnUp->Index], pdd->VirtualAddress);
	CString sz; USES_CONVERSION;

	int nItemIdx = 0;
	PBYTE pIter = m_pImgView + dwOffset;
	PBYTE pBegin = pIter;
	while (true)
	{
		PIMAGE_BOUND_IMPORT_DESCRIPTOR pbid = (PIMAGE_BOUND_IMPORT_DESCRIPTOR)pIter;
		if (*((PDWORD64)pbid) == 0)
		{
			sz.Format(L"[%d]NULL", nItemIdx);
			InsertStructNode(pnUp->Node, pnUp->Index, dwOffset, 
				sz, L"IMAGE_BOUND_IMPORT_DESCRIPTOR", 0, IMG_IDX_XBOX);
			break;
		}

		PCWSTR pszDllName = A2CT((PSTR)(pBegin + pbid->OffsetModuleName));
		sz.Format(L"[%d]%s", nItemIdx, pszDllName);
		PPE_NODE pn = InsertStructNode(pnUp->Node, pnUp->Index, dwOffset, sz, L"IMAGE_BOUND_IMPORT_DESCRIPTOR");
		AppendStructMembers(pn);
		if (pbid->NumberOfModuleForwarderRefs > 0)
			UpdateNodeText(pn->Node, L"Has Forwarding", COL_IDX_INFO);
		PPE_NODE pnom = FindNode(pn->Node, L"OffsetModuleName");
		UpdateNodeText(pnom->Node, pszDllName, COL_IDX_INFO);

		dwOffset += sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR);
		pIter += sizeof(IMAGE_BOUND_IMPORT_DESCRIPTOR);

		for (WORD i = 0; i < pbid->NumberOfModuleForwarderRefs; i++)
		{
			PIMAGE_BOUND_FORWARDER_REF pbfr = (PIMAGE_BOUND_FORWARDER_REF)pIter;
			pszDllName = A2CT((PSTR)(pBegin + pbfr->OffsetModuleName));
			sz.Format(L"[%d]%s", i, pszDllName);
			PPE_NODE pn2 = InsertStructNode(pn->Node, pnUp->Index, dwOffset, sz, L"IMAGE_BOUND_FORWARDER_REF");
			AppendStructMembers(pn2);
			pnom = FindNode(pn2->Node, L"OffsetModuleName");
			UpdateNodeText(pnom->Node, pszDllName, COL_IDX_INFO);

			dwOffset += sizeof(IMAGE_BOUND_FORWARDER_REF);
			pIter += sizeof(IMAGE_BOUND_FORWARDER_REF);
		}
		nItemIdx++;
	}
	return false;
}

#include <DelayImp.h>
bool PEAnals::ParseDirEntryDelayImport(PPE_NODE pnUp, PIMAGE_DATA_DIRECTORY pdd)
{
	PIMAGE_SECTION_HEADER psh = &m_pshs[pnUp->Index];
	DWORD dwOffset = RVA_TO_OFFSET(psh, pdd->VirtualAddress);
	CString sz; USES_CONVERSION;

	PBYTE pIter = m_pImgView + dwOffset;
	INT nItemCnt = 0;
	for (;;)
	{
		PImgDelayDescr pdi = (PImgDelayDescr)pIter;
		pIter += sizeof(ImgDelayDescr);
		if (pdi->grAttrs != dlattrRva || !pdi->rvaDLLName)
		{
			if (pdi->rvaDLLName == 0)
			{
				sz.Format(L"[%d]NULL", nItemCnt);
				InsertStructNode(pnUp->Node, pnUp->Index, 
					dwOffset, sz, L"ImgDelayDescr", 0, IMG_IDX_XBOX);
				break;
			}
			sz.Format(L"[%d]Invalid Descr", nItemCnt);
			PPE_NODE pnSub = InsertStructNode(pnUp->Node, pnUp->Index, dwOffset, sz, L"ImgDelayDescr");
			sz.Format(L"grAttrs = %d, rvaDLLName = 0x%08X", pdi->grAttrs, pdi->rvaDLLName);
			UpdateNodeText(pnSub->Node, sz, COL_IDX_INFO);
			dwOffset += sizeof(ImgDelayDescr);
			nItemCnt++;
			continue;
		}

		DWORD dwFieldOff = 0;
		if (RVA_IN_SECT(psh, pdi->rvaDLLName))
			dwFieldOff = RVA_TO_OFFSET(psh, pdi->rvaDLLName);
		else
		{
			PIMAGE_SECTION_HEADER psh2 = PEPlus::FindSectHdr(m_pImgView, pdi->rvaDLLName);
			dwFieldOff = RVA_TO_OFFSET(psh2, pdi->rvaDLLName);
		}
		CString szName = A2CT((PSTR)m_pImgView + dwFieldOff);
		sz.Format(L"[%d]%s", nItemCnt, szName);

		PPE_NODE pn = InsertStructNode(pnUp->Node,
			pnUp->Index, dwOffset, sz, L"ImgDelayDescr");
		AppendStructMembers(pn);
		PPE_NODE pnSub = FindNode(pn->Node, L"rvaDLLName");
		UpdateNodeText(pnSub->Node, szName, COL_IDX_INFO, true);

		pnSub = FindNode(pn->Node, L"rvaINT");
		if (RVA_IN_SECT(psh, pdi->rvaINT))
			dwFieldOff = RVA_TO_OFFSET(psh, pdi->rvaINT);
		else
		{
			PIMAGE_SECTION_HEADER psh2 = PEPlus::FindSectHdr(m_pImgView, pdi->rvaINT);
			dwFieldOff = RVA_TO_OFFSET(psh2, pdi->rvaINT);
		}
		DWORD dwINTOff = dwFieldOff;
		int nINTCnt = 0;
		for (;; nINTCnt++)
		{
			PPE_NODE pn2 = InsertImpRVARefNode(psh, nINTCnt, dwFieldOff, pnSub);
			if (PE_SCHEMA::GetValue(m_pImgView + pn2->Offset, pn2->Type) == 0)
				break;
			dwFieldOff += pn2->Size;
		}
		sz.Format(L"%d functions Imported", nINTCnt);
		UpdateNodeText(pn->Node, sz, COL_IDX_INFO);

		PCWSTR pszFlds[] = { L"rvaIAT", L"rvaBoundIAT", L"rvaUnloadIAT" };
		DWORD  dwRVAs[] = { pdi->rvaIAT, pdi->rvaBoundIAT, pdi->rvaUnloadIAT };
		for (int j = 0; j < 3; j++)
		{
			if (dwRVAs[j] == 0)
				continue;

			pnSub = FindNode(pn->Node, pszFlds[j]);
			PIMAGE_SECTION_HEADER psh2 = PEPlus::FindSectHdr(m_pImgView, dwRVAs[j]);
			dwFieldOff = RVA_TO_OFFSET(psh2, dwRVAs[j]);

			DWORD dwINTItr = dwINTOff;
			for (int i = 0; i <= nINTCnt; i++)
			{
				PPE_NODE pn2 = InsertImpRVARefNode(psh, i, dwFieldOff, pnSub, dwINTItr, true);
				dwFieldOff += pn2->Size;
				dwINTItr += pn2->Size;
			}
		}
		dwOffset += sizeof(ImgDelayDescr);
		nItemCnt++;
	}
	return false;
}

CString PEAnals::GetNameOrOrd(DWORD dwIntOff, PIMAGE_SECTION_HEADER psh)
{
	CString sz; USES_CONVERSION;
	int nOrdVal = -1;
	ULONG64 dwHanRva = 0;

	if (m_bIs32Bit)
	{
		DWORD v = *((PDWORD)(m_pImgView + dwIntOff));
		if (IMAGE_SNAP_BY_ORDINAL32(v))
			nOrdVal = (WORD)IMAGE_ORDINAL32(v);
		else
			dwHanRva = v;
	}
	else
	{
		ULONGLONG v = *((PULONGLONG)(m_pImgView + dwIntOff));
		if (IMAGE_SNAP_BY_ORDINAL64(v))
			nOrdVal = (WORD)IMAGE_ORDINAL64(v);
		else
			dwHanRva = v;
	}

	if (nOrdVal < 0)
	{
		PIMAGE_IMPORT_BY_NAME pin = (PIMAGE_IMPORT_BY_NAME)
			(m_pImgView + RVA_TO_OFFSET(psh, (DWORD)dwHanRva));
		return A2CT(pin->Name);
	}
	else
	{
		sz.Format(L"ORD_%d", nOrdVal);
		return sz;
	}
}

PIMAGE_IMPORT_BY_NAME PEAnals::BuildHintNameNodes(DWORD dwRvaVal, PPE_NODE pnUp)
{
	PIMAGE_IMPORT_BY_NAME pin = (PIMAGE_IMPORT_BY_NAME)(m_pImgView + dwRvaVal);
	PPE_NODE pn = InsertStructNode(pnUp->Node, pnUp->Index, dwRvaVal,
		L"HintAndName", L"IMAGE_IMPORT_BY_NAME", 0, IMG_IDX_RVADIR);
	AppendStructMembers(pn);

	CString sz; USES_CONVERSION;
	sz.Format(L"0x%X, %s", pin->Hint, A2CT(pin->Name));
	m_pView->SetItemText(pn->Node, sz, COL_IDX_INFO);
	return pin;
}

PPE_NODE PEAnals::InsertImpRVARefNode(PIMAGE_SECTION_HEADER psh,
	int nIdx, DWORD dwOffset, PPE_NODE pnUp, DWORD dwNameOff, bool bDelay)
{
	CString sz; USES_CONVERSION;
	int nOrdVal = -1;
	ULONG64 dwHanRva = 0;

	if (m_bIs32Bit)
	{
		DWORD v = *((PDWORD)(m_pImgView + dwOffset));
		if (IMAGE_SNAP_BY_ORDINAL32(v))
			nOrdVal = (WORD)IMAGE_ORDINAL32(v);
		else
			dwHanRva = v;
	}
	else
	{
		ULONGLONG v = *((PULONGLONG)(m_pImgView + dwOffset));
		if (IMAGE_SNAP_BY_ORDINAL64(v))
			nOrdVal = (WORD)IMAGE_ORDINAL64(v);
		else
			dwHanRva = v;
	}

	PE_TYPE dt = (m_bIs32Bit) ? PE_TYPE::UInt32 : PE_TYPE::UInt64;
	PPE_NODE pn = new PE_NODE(pnUp->Index, dwOffset, L"", dt);
	if (nOrdVal < 0 && dwHanRva > 0)
	{
		if (IS_VALID_RVA(dwHanRva) && dwNameOff == 0)
			pn->IsRva = true;
		if (bDelay)
			pn->IsVa = true;
	}
	InsertNode(pn, pnUp->Node);

	if (nOrdVal < 0)
	{
		sz.Format(L"[%d]", nIdx);
		if (dwHanRva > 0)
		{
			if (dwNameOff > 0)
			{
				if (bDelay)
					sz.Append(L"dimp_");
				else
					sz.Append(L"Bound:");
				sz.Append(GetNameOrOrd(dwNameOff, psh));
			}
			else
			{
				if (IS_VALID_RVA(dwHanRva))
				{
					PIMAGE_IMPORT_BY_NAME pin =
						BuildHintNameNodes(RVA_TO_OFFSET(psh, (DWORD)dwHanRva), pn);
					sz.Append(A2CT(pin->Name));
					CString szInf; szInf.Format(L"Hint:%d", pin->Hint);
					UpdateNodeText(pn->Node, szInf, COL_IDX_INFO, true);
				}
				else
					sz.AppendFormat(L"Unknown:0x%08X", dwHanRva);
			}
		}
		else
		{
			sz.Append(L"NULL");
			m_pView->SetItemImage(pn->Node, IMG_IDX_LFNO, IMG_IDX_LFNO);
		}
	}
	else
	{
		sz.Format(L"Ordianal : %d(0x%X)", nOrdVal, nOrdVal);
		UpdateNodeText(pn->Node, sz, COL_IDX_INFO);
		sz.Format(L"[%d]ORD_%d", nIdx, nOrdVal);
	}
	UpdateNodeText(pn->Node, sz, COL_IDX_NAME);
	return pn;
}
