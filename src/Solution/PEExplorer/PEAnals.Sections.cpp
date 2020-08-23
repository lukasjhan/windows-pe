#include "stdafx.h"
#include "PEAnals.h"
#include "PEView.h"

bool PEAnals::ParseDirEntryExport(PPE_NODE pnUp, PIMAGE_DATA_DIRECTORY pdd)
{
	PIMAGE_SECTION_HEADER psh = &m_pshs[pnUp->Index];
	DWORD dwOffset = RVA_TO_OFFSET(psh, pdd->VirtualAddress);
	PIMAGE_EXPORT_DIRECTORY ped = PIMAGE_EXPORT_DIRECTORY(m_pImgView + dwOffset);
	CString sz; USES_CONVERSION;

	PPE_NODE pnED = InsertStructNode(pnUp->Node, 
		pnUp->Index, dwOffset, L"ExportDir", L"IMAGE_EXPORT_DIRECTORY");
	AppendStructMembers(pnED);

	PPE_NODE pnSub = FindNode(pnED->Node, L"Name");
	DWORD dwFieldOff = RVA_TO_OFFSET(psh, ped->Name);
	CString szName = A2CT((PSTR)m_pImgView + dwFieldOff);
	UpdateNodeText(pnSub->Node, szName, COL_IDX_INFO, true);
	InsertStrNode(pnSub->Node, pnUp->Index, dwFieldOff, L"Dll Name", szName.GetLength());

	PDWORD pNameRvas = (PDWORD)(m_pImgView + RVA_TO_OFFSET(psh, ped->AddressOfNames));
	PWORD  pOrdinals = (PWORD) (m_pImgView + RVA_TO_OFFSET(psh, ped->AddressOfNameOrdinals));
	PDWORD pFuncRvas = (PDWORD)(m_pImgView + RVA_TO_OFFSET(psh, ped->AddressOfFunctions));
	PINT pIdxMap = new int[ped->AddressOfFunctions];
	memset(pIdxMap, 0xFF, ped->AddressOfFunctions * sizeof(int));

	dwFieldOff = RVA_TO_OFFSET(psh, ped->AddressOfNameOrdinals);
	pnSub = FindNode(pnED->Node, L"AddressOfNameOrdinals");
	sz.Format(L"%d Ordinals", ped->NumberOfNames);
	UpdateNodeText(pnSub->Node, sz, COL_IDX_INFO, true);
	for (DWORD i = 0; i < ped->NumberOfNames; i++)
	{
		PSTR pFncName = (PSTR)m_pImgView + RVA_TO_OFFSET(psh, pNameRvas[i]);
		sz.Format(L"[%d]%s", i, A2CT(pFncName));
		PPE_NODE pn = InsertFieldNode(pnSub->Node, pnUp->Index, dwFieldOff, sz, PE_TYPE::UInt16);

		WORD wOrd = pOrdinals[i];
		pIdxMap[wOrd] = (int)i;
		sz.Format(L"Ordinal : %d(%d)", ped->Base + wOrd, wOrd);
		UpdateNodeText(pn->Node, sz, COL_IDX_INFO);
		dwFieldOff += pn->Size;
	}

	dwFieldOff = RVA_TO_OFFSET(psh, ped->AddressOfNames);
	pnSub = FindNode(pnED->Node, L"AddressOfNames");
	sz.Format(L"%d Function Names", ped->NumberOfNames);
	UpdateNodeText(pnSub->Node, sz, COL_IDX_INFO, true);
	for (DWORD i = 0; i < ped->NumberOfNames; i++)
	{
		DWORD dwNameOff = RVA_TO_OFFSET(psh, pNameRvas[i]);
		PSTR pFncName = (PSTR)m_pImgView + dwNameOff;
		sz.Format(L"[%d]%s", i, A2CT(pFncName));

		PPE_NODE pn = InsertRVANode(pnSub->Node, pnUp->Index, dwFieldOff, sz);
		UpdateNodeText(pn->Node, A2CT(pFncName), COL_IDX_INFO, true);
		InsertStrNode(pn->Node, pnUp->Index, dwNameOff, L"Func Name", (int)strlen(pFncName));
		dwFieldOff += pn->Size;
	}

	dwFieldOff = RVA_TO_OFFSET(psh, ped->AddressOfFunctions);
	pnSub = FindNode(pnED->Node, L"AddressOfFunctions");
	sz.Format(L"%d Function Pointers", ped->NumberOfFunctions);
	UpdateNodeText(pnSub->Node, sz, COL_IDX_INFO, true);
	for (DWORD i = 0; i < ped->NumberOfFunctions; i++)
	{
		PPE_NODE pn = NULL;
		if (pFuncRvas[i] == 0)
		{
			sz.Format(L"[%d]NULL", i);
			pn = InsertFieldNode(pnSub->Node, pnUp->Index, 
				dwFieldOff, sz, PE_TYPE::UInt32, 0, IMG_IDX_LFNO);
			dwFieldOff += pn->Size;
			continue;
		}

		int nIdx = pIdxMap[i];
		if (nIdx < 0)
			szName = L"NO_NAME";
		else
			szName = A2CT((PSTR)m_pImgView + RVA_TO_OFFSET(psh, pNameRvas[nIdx]));
		sz.Format(L"[%d]%s", i, szName);
		pn = InsertRVANode(pnSub->Node, pnUp->Index, dwFieldOff, sz);

		PIMAGE_SECTION_HEADER pcsh = PEPlus::FindSectHdr(m_pImgView, pFuncRvas[i]);
		DWORD dwCodeOff = RVA_TO_OFFSET(pcsh, pFuncRvas[i]);
		if ((pcsh->Characteristics & IMAGE_SCN_CNT_CODE) == 0)
		{
			if (pFuncRvas[i] >= pdd->VirtualAddress &&
				pFuncRvas[i] < pdd->VirtualAddress + pdd->Size)
			{
				PCWSTR pszFwd = A2CT((PSTR)m_pImgView + dwCodeOff);
				sz.Format(L"Forwarded to %s", pszFwd);
				UpdateNodeText(pn->Node, sz, COL_IDX_INFO, true);
				InsertStrNode(pn->Node, pnUp->Index, 
					dwCodeOff, L"Forwarded Name", (int)_tcslen(pszFwd));
			}
			else
			{
				sz = L"Exported Data";
				if (dwCodeOff >= pcsh->PointerToRawData + pcsh->SizeOfRawData)
					sz.Append(L": Not part of PE");
				UpdateNodeText(pn->Node, sz, COL_IDX_INFO, true);
			}
			SetNodeTextColor(pn->Node, RGB(255, 0, 0));
		}
		else
		{
			DECODED_INS di; int ndiCnt = 1;
			PEDisAsm::DisAssemble(&di, ndiCnt, m_pImgView, 
				dwCodeOff, pcsh->Misc.VirtualSize, m_bIs32Bit);

			sz.Format(L"Code: %s", PE_SCHEMA::Bin2Str
				(m_pImgView + dwCodeOff, PE_TYPE::UInt8, di._count));
			UpdateNodeText(pn->Node, sz, COL_IDX_INFO, true);

			PPE_NODE pnCode = InsertCodeNode(pn->Node, 
				pnUp->Index, dwCodeOff, L"Code", di._count);
			DECODED dec;
			PEDisAsm::DecodedFormat(&di, &dec);	
			sz = A2T(dec.Mnemonic);
			if (dec.Operands != NULL)
				sz.AppendFormat(L" %s", A2CT(dec.Operands));
			UpdateNodeText(pnCode->Node, sz, COL_IDX_INFO);
		}
		dwFieldOff += pn->Size;
	}
	delete[] pIdxMap;

	return false;
}

bool PEAnals::ParseDirEntryDebug(PPE_NODE pnUp, PIMAGE_DATA_DIRECTORY pdd)
{
	PIMAGE_SECTION_HEADER psh = &m_pshs[pnUp->Index];
	DWORD dwOffset = RVA_TO_OFFSET(psh, pdd->VirtualAddress);

	PIMAGE_DEBUG_DIRECTORY pdbgs = PIMAGE_DEBUG_DIRECTORY(m_pImgView + dwOffset);
	bool bIs32 = PEPlus::Is32bitPE(m_pImgView);
	CString sz; USES_CONVERSION;

	PPE_ENUM pDbgType = SCHEMA.EnumMap()->find(L"IMAGE_DEBUG_TYPE")->second;
	int nItemCnt = pdd->Size / sizeof(IMAGE_DEBUG_DIRECTORY);
	for (int i = 0; i < nItemCnt; i++)
	{
		PIMAGE_DEBUG_DIRECTORY pdbg = &pdbgs[i];

		CString szType = pDbgType->Find(pdbg->Type);
		if (szType.IsEmpty()) szType = L"Unknown";
		sz.Format(L"[%d]%s", i, szType);
		PPE_NODE pn = InsertStructNode(pnUp->Node, pnUp->Index, dwOffset, sz, L"IMAGE_DEBUG_DIRECTORY");
		AppendStructMembers(pn);
		dwOffset += sizeof(IMAGE_DEBUG_DIRECTORY);

		PPE_NODE pnSub = FindNode(pn->Node, L"PointerToRawData");
		switch (pdbg->Type)
		{
		case IMAGE_DEBUG_TYPE_CODEVIEW:
		{
			PPE_NODE pnd = InsertStructNode(pnSub->Node, pnUp->Index, pdbg->PointerToRawData,
				L"PDBInfo", L"IMAGE_PDB_INFO", 0, IMG_IDX_RVADIR);
			AppendStructMembers(pnd);

			PPE_NODE pndSub = FindNode(pnd->Node, L"PdbFileName");
			UpdateNodeText(pndSub->Node, PDB_INFO, COL_IDX_INFO);
		}
		break;

		case IMAGE_DEBUG_TYPE_FEATURE:
		{
			PPE_NODE pnd = InsertStructNode(pnSub->Node, pnUp->Index, pdbg->PointerToRawData,
				L"Feature", L"IMAGE_FEATURE_INFO", 0, IMG_IDX_RVADIR);
			AppendStructMembers(pnd);
		}
		break;

		case IMAGE_DEBUG_TYPE_POGO:
		{
			PPE_NODE pnd = InsertStructNode(pnSub->Node, pnUp->Index, pdbg->PointerToRawData,
				L"POGO", L"IMAGE_POGO_INFO", 0, IMG_IDX_RVADIR);
			AppendStructMembers(pnd);

			int nIdx = 0;
			DWORD dwPgoOff = pdbg->PointerToRawData + sizeof(IMAGE_POGO_INFO);
			while (true)
			{
				PIMAGE_PGO_ITEM pipi = (PIMAGE_PGO_ITEM)(m_pImgView + dwPgoOff);
				if (pipi->PGORva == 0)
					break;

				sz.Format(L"[%d]PGO", nIdx);
				PPE_NODE pnpgo = InsertStructNode(pnd->Node, pnUp->Index, dwPgoOff, sz, L"IMAGE_PGO_ITEM");
				AppendStructMembers(pnpgo);

				dwPgoOff += sizeof(DWORD64);
				int nPhsLen = (int)strlen((PCSTR)pipi->PGOName) + 1;
				dwPgoOff += nPhsLen;
				if (dwPgoOff & 3)
					dwPgoOff += (4 - (dwPgoOff & 3));
				nIdx++;
			}
			pnd->Size = dwPgoOff - pdbg->PointerToRawData;
			sz.Format(L"0x%X(%d)", pnd->Size, pnd->Size);
			UpdateNodeText(pnd->Node, sz, COL_IDX_SIZE);
		}
		break;
		}
	}
	return false;
}

bool PEAnals::ParseDirEntryBaseReloc(PPE_NODE pnUp, PIMAGE_DATA_DIRECTORY pdd)
{
	PIMAGE_SECTION_HEADER psh = &m_pshs[pnUp->Index];
	DWORD dwOffset = RVA_TO_OFFSET(psh, pdd->VirtualAddress);
	bool bIs32 = PEPlus::Is32bitPE(m_pImgView);
	CString sz; USES_CONVERSION;

	int nSize = 0, i = 0;
	PBYTE pbrs = m_pImgView + dwOffset;
	for (; nSize < (int)pdd->Size; i++)
	{
		PIMAGE_BASE_RELOCATION pbr = (PIMAGE_BASE_RELOCATION)pbrs;
		if (pbr->SizeOfBlock == 0)
		{
			pbrs += sizeof(IMAGE_BASE_RELOCATION);
			nSize += sizeof(IMAGE_BASE_RELOCATION);
			continue;
		}

		PPE_NODE pn = InsertStructNode(pnUp->Node, pnUp->Index, dwOffset, L"", L"IMAGE_BASE_RELOCATION");
		AppendStructMembers(pn);
		dwOffset += sizeof(IMAGE_BASE_RELOCATION);

		DWORD64 llImgBase = PEPlus::GetImageBase(m_pImgView);
		PIMAGE_SECTION_HEADER psh2 = PEPlus::FindSectHdr(m_pImgView, pbr->VirtualAddress);
		if (psh2 == NULL)
		{
			pbrs += sizeof(IMAGE_BASE_RELOCATION);
			nSize += sizeof(IMAGE_BASE_RELOCATION);
			continue;
		}
		sz.Format(_T("[%d]%08X:%s"), i, pbr->VirtualAddress, PEPlus::GetSectionName(psh2));
		UpdateNodeText(pn->Node, sz, COL_IDX_NAME);

		int nRelocCnt = (int)((pbr->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD));
		sz.Format(L"%d Relocations", nRelocCnt);
		UpdateNodeText(pn->Node, sz, COL_IDX_INFO);

		PWORD pwRelcs = (PWORD)(m_pImgView + dwOffset);
		PPE_NODE pnar = InsertFieldNode(pn->Node, pnUp->Index, dwOffset, L"TypeOffsets", PE_TYPE::UInt16, nRelocCnt);
		for (int j = 0; j < nRelocCnt; j++)
		{
			WORD dwVal = pwRelcs[j];
			WORD wType = (dwVal & 0xF000) >> 12;
			WORD wOffs = dwVal & 0x0FFF;

			PPE_NODE pnto = InsertFieldNode(pnar->Node, pnUp->Index, dwOffset, L"", PE_TYPE::UInt16);
			if (wType != IMAGE_REL_BASED_HIGHLOW && wType != IMAGE_REL_BASED_DIR64)
			{
				sz.Format(_T("[%d]ABSOLUTE"), j);
				UpdateNodeText(pnto->Node, sz, COL_IDX_NAME);
				m_pView->SetItemImage(pnto->Node, IMG_IDX_LFNO, IMG_IDX_LFNO);
				dwOffset += sizeof(WORD);
				continue;
			}

			DWORD dwRlcRva = pbr->VirtualAddress + wOffs;
			sz.Format(L"0x%02X:%06X(0x%08X)", wType, wOffs, dwRlcRva);
			UpdateNodeText(pnto->Node, sz, COL_IDX_VALUE);

			DWORD dwRlcOff = RVA_TO_OFFSET(psh2, dwRlcRva);
			sz.Format(L"Off:0x%08X => Val=", dwRlcOff);
			DWORD64 uv = (wType == IMAGE_REL_BASED_HIGHLOW) ? 
				*((PDWORD)(m_pImgView + dwRlcOff)) : *((PDWORD64)(m_pImgView + dwRlcOff));
			DWORD dwVarRva = (DWORD)(uv - llImgBase);
			PIMAGE_SECTION_HEADER psh3 = PEPlus::FindSectHdr(m_pImgView, dwVarRva);
			if (psh3 == NULL)
				sz.AppendFormat(L"0x%08X:[HEADER  ]0x00000000", dwVarRva);
			else
				sz.AppendFormat(L"0x%08X:[%-8s]0x%08X", dwVarRva,
				PEPlus::GetSectionName(psh3), RVA_TO_OFFSET(psh3, dwVarRva));

			CString szLoc, szTrg;
			if (DIA_PDB.SESSION)
			{
				szLoc = GetSymbolName(DIA_PDB.SESSION, dwRlcRva);
				szTrg = GetSymbolName(DIA_PDB.SESSION, dwVarRva);
			}
			if (!szTrg.IsEmpty())
				sz.AppendFormat(L" -> %s", szTrg);
			UpdateNodeText(pnto->Node, sz, COL_IDX_INFO);

			sz.Format(L"[%d]%08X", j, dwRlcRva);
			if (!szLoc.IsEmpty())
				sz.AppendFormat(L":%s", szLoc);
			UpdateNodeText(pnto->Node, sz, COL_IDX_NAME);
			dwOffset += sizeof(WORD);
		}
		nSize += pbr->SizeOfBlock;
		pbrs  += pbr->SizeOfBlock;
	}
	return false;
}

bool PEAnals::ParseDirEntryTLS(PPE_NODE pnUp, PIMAGE_DATA_DIRECTORY pdd)
{
	PIMAGE_SECTION_HEADER psh = &m_pshs[pnUp->Index];
	DWORD dwOffset = RVA_TO_OFFSET(psh, pdd->VirtualAddress);
	CString sz; USES_CONVERSION;

	PCWSTR pszType = (m_bIs32Bit) ? L"IMAGE_TLS_DIRECTORY32" : L"IMAGE_TLS_DIRECTORY64";
	PPE_NODE pnTls = InsertStructNode(pnUp->Node, pnUp->Index, dwOffset, L"TLSDir", pszType);
	AppendStructMembers(pnTls);

	if (DIA_PDB.SESSION)
	{
		PCWSTR pszCols[] =
		{
			L"StartAddressOfRawData", L"EndAddressOfRawData",
			L"AddressOfIndex", L"AddressOfCallBacks"
		};
		DWORD64 ulImgBase = PEPlus::GetImageBase(m_pImgView);
		for (int i = 0; i < 4; i++)
		{
			PPE_NODE pnSub = FindNode(pnTls->Node, pszCols[i]);
			DWORD64 ulVal = PE_SCHEMA::GetValue(m_pImgView + pnSub->Offset,
				(m_bIs32Bit) ? PE_TYPE::UInt32 : PE_TYPE::UInt64);
			DWORD dwRVA = (DWORD)(ulVal - ulImgBase);
			if (DIA_PDB.SESSION)
			{
				sz = GetSymbolName(DIA_PDB.SESSION, dwRVA);
				if (!sz.IsEmpty())
					UpdateNodeText(pnSub->Node, sz, COL_IDX_INFO, true);
			}
		}
	}
	return false;
}

bool PEAnals::ParseDirEntryLoadConfig(PPE_NODE pnUp, PIMAGE_DATA_DIRECTORY pdd)
{
	PIMAGE_SECTION_HEADER psh = &m_pshs[pnUp->Index];
	DWORD dwOffset = RVA_TO_OFFSET(psh, pdd->VirtualAddress);
	CString sz; USES_CONVERSION;

	PCWSTR pszType = NULL;
	DWORD  dwSize = 0;
	if (m_bIs32Bit)
	{
		pszType = L"IMAGE_LOAD_CONFIG_DIRECTORY32";
		dwSize = ((PIMAGE_LOAD_CONFIG_DIRECTORY32)(m_pImgView + dwOffset))->Size;
	}
	else
	{
		pszType = L"IMAGE_LOAD_CONFIG_DIRECTORY64";
		dwSize = ((PIMAGE_LOAD_CONFIG_DIRECTORY64)(m_pImgView + dwOffset))->Size;
	}
	PPE_NODE pn = InsertStructNode(pnUp->Node, pnUp->Index, dwOffset, L"LoadConfig", pszType);
	AppendStructMembers(pn, dwSize);

	if (DIA_PDB.SESSION)
	{
		PCWSTR pszCols[] =
		{
			L"SecurityCookie", 
			L"SEHandlerTable",
			L"GuardCFCheckFunctionPointer", 
			L"GuardCFDispatchFunctionPointer",
			L"GuardCFFunctionTable"
		};
		DWORD64 ulImgBase = PEPlus::GetImageBase(m_pImgView);
		for (int i = 0; i < 5; i++)
		{
			PPE_NODE pnSub = FindNode(pn->Node, pszCols[i]);
			if (pnSub == NULL)
				break;

			DWORD64 ulVal = PE_SCHEMA::GetValue(m_pImgView + pnSub->Offset,
				(m_bIs32Bit) ? PE_TYPE::UInt32 : PE_TYPE::UInt64);
			if (ulVal == 0)
				continue;
			DWORD dwRVA = (DWORD)(ulVal - ulImgBase);
			sz = GetSymbolName(DIA_PDB.SESSION, dwRVA);
			if (!sz.IsEmpty())
				UpdateNodeText(pnSub->Node, sz, COL_IDX_INFO, true);
		}
	}
	return false;
}

#include <WinTrust.h>
bool PEAnals::ParseDirEntrySecurity(PPE_NODE pnUp, PIMAGE_DATA_DIRECTORY pdd)
{
	CString sz; USES_CONVERSION;
	DWORD dwOffset = pdd->VirtualAddress;
	int nSize = (int)pdd->Size;
	int nIdx = 0;

	while (nSize > 0)
	{
		sz.Format(L"[%d]WinCertificate", nIdx);
		PPE_NODE pn = InsertStructNode(pnUp->Node, pnUp->Index, dwOffset, sz, L"WIN_CERTIFICATE");
		AppendStructMembers(pn);

		LPWIN_CERTIFICATE pCert = (LPWIN_CERTIFICATE)(m_pImgView + dwOffset);
		pn->Size = pCert->dwLength;
		sz.Format(L"%d(0x%X)", pn->Size, pn->Size);
		UpdateNodeText(pn->Node, sz, COL_IDX_SIZE);

		PBYTE pPKCS7 = pCert->bCertificate;
		DWORD dwPkcsSize = pCert->dwLength - 8;
		PPE_NODE pnSub = InsertFieldNode(pn->Node, pn->Index, 
			dwOffset + 8, L"PKCS7", PE_TYPE::UInt8, (int)dwPkcsSize);
		pnSub->Kind = IMAGE_DIRECTORY_ENTRY_SECURITY;

		int nEntSize = (int)ROUND_UP(pCert->dwLength, 8);
		dwOffset += nEntSize;
		nSize	 -= nEntSize;
		nIdx++;
	}
	return false;
}

bool PEAnals::ParseDirEntryComDescript(PPE_NODE pnUp, PIMAGE_DATA_DIRECTORY pdd)
{
	PIMAGE_SECTION_HEADER psh = &m_pshs[pnUp->Index];
	DWORD dwOffset = RVA_TO_OFFSET(psh, pdd->VirtualAddress);
	PPE_NODE pn = InsertStructNode(pnUp->Node, pnUp->Index, dwOffset, L".NET Header", L"IMAGE_COR20_HEADER");
	AppendStructMembers(pn);
	
	PCWSTR pszFldNames[7] = 
	{
		L"MetaData", L"Resources", L"StrongNameSignature", L"CodeManagerTable", 
		L"VTableFixups", L"ExportAddressTableJumps", L"ManagedNativeHeader", 
	};
	for (int i = 0; i < 7; i++)
	{
		PPE_NODE pnSub = FindNode(pn->Node, pszFldNames[i]);
		DWORD dwRVA = (DWORD)PE_SCHEMA::GetValue(m_pImgView + pnSub->Offset, pnSub->Type);
		if (dwRVA == 0)
			m_pView->SetItemImage(pnSub->Node, IMG_IDX_XBOX, IMG_IDX_XBOX);
		else
			AppendStructMembers(pnSub);
	}
	return false;
}
