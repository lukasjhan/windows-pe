#include "stdafx.h"
#include "PEAnals.h"
#include "PEView.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
PE_NODE::PE_NODE(PEAnals* ppe) : PE_MEMBER()
{
	Type	= PE_TYPE::PEFormat;
	Offset	= 0;
	Size	= GetFileSize(ppe->GetImgFile(), NULL);
	Tag		= (LPARAM)ppe;
	Index	= INVALID_SECT_IDX;
	Node	= NULL;
	Kind = NK_LEAF, SubT = 0;

	CString sz = ppe->GetPEPath();
	int nPos = sz.ReverseFind(L'\\');
	Name	= sz.Mid(nPos + 1);
	HasInfo = 1;
	sz = sz.Left(nPos);
	Ref.Info = new TCHAR[nPos + 1];
	wcsncpy_s(Ref.Info, nPos + 1, (PCWSTR)sz, nPos);
}

PE_NODE::PE_NODE(short nSectIdx, DWORD dwOffset, PCTSTR pszName, PCTSTR pszType, int nArrLen)
	: PE_MEMBER(pszName, PE_TYPE::Struct)
{
	Ref.Struct = PEAnals::SCHEMA.FindType(pszType);
	Offset = dwOffset;
	Index = nSectIdx;
	if (nArrLen > 0)
	{
		IsArr = true;
		More.ArrLen = nArrLen;
	}
	Size = GetSize();
	Node = NULL, Tag = 0;
	Kind = NK_LEAF, SubT = 0;
}

PE_NODE::PE_NODE(short nSectIdx, DWORD dwOffset, PCTSTR pszName, PE_TYPE dt, int nArrLen)
	: PE_MEMBER(pszName, dt)
{
	ASSERT(dt < PE_TYPE::Struct);
	Offset = dwOffset;
	Index = nSectIdx;
	if (nArrLen > 0)
	{
		IsArr = true;
		More.ArrLen = nArrLen;
	}
	Size = GetSize();
	Node = NULL, Tag = 0;
	Kind = NK_LEAF, SubT = 0;
}

PE_NODE::PE_NODE(short nSectIdx, DWORD dwOffset, PPE_MEMBER pm) : PE_MEMBER(*pm)
{
	Offset = dwOffset;
	Index = nSectIdx;
	Size = GetSize();
	Node = NULL, Tag = 0;
	Kind = NK_LEAF, SubT = 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////
#define GSZ_APP_TMP_CACHE	_T("%s\\Temp\\SymbolCache")
PE_SCHEMA PEAnals::SCHEMA;
TCHAR PEAnals::SYM_PATH[MAX_PATH];

PEAnals::PEAnals()
{
	m_hImgFile = INVALID_HANDLE_VALUE;
	m_hImgMap  = NULL;
	m_pImgView = NULL;
}

PEAnals::PEAnals(PCTSTR pszPEFile, CTreeListCtrl* pView) : PEAnals()
{
	memset(m_pfnddrs, 0, sizeof(PFN_PARSE_DATADIR) * (IMAGE_NUMBEROF_DIRECTORY_ENTRIES - 1));
	m_pfnddrs[ 0] = &PEAnals::ParseDirEntryExport;
	m_pfnddrs[ 1] = &PEAnals::ParseDirEntryImport;
	m_pfnddrs[ 2] = &PEAnals::ParseDirEntryResource;
#ifdef _WIN64
	m_pfnddrs[ 3] = &PEAnals::ParseDirEntryException;
#endif
	m_pfnddrs[ 4] = &PEAnals::ParseDirEntrySecurity;
	m_pfnddrs[ 5] = &PEAnals::ParseDirEntryBaseReloc;
	m_pfnddrs[ 6] = &PEAnals::ParseDirEntryDebug;
	m_pfnddrs[ 9] = &PEAnals::ParseDirEntryTLS;
	m_pfnddrs[10] = &PEAnals::ParseDirEntryLoadConfig;
	m_pfnddrs[11] = &PEAnals::ParseDirEntryBoundImport;
	m_pfnddrs[12] = &PEAnals::ParseDirEntryIAT;
	m_pfnddrs[13] = &PEAnals::ParseDirEntryDelayImport;
	m_pfnddrs[14] = &PEAnals::ParseDirEntryComDescript;

	m_pshs		= NULL;
	m_pdds		= NULL;
	m_pView		= pView;
	m_pnRoot	= NULL;
	m_bIs32Bit	= false;

	m_szPEPath = pszPEFile;
	m_hImgFile = CreateFile(pszPEFile, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, 0, NULL);
	if (m_hImgFile == INVALID_HANDLE_VALUE)
		throw HRESULT_FROM_WIN32(GetLastError());

	m_hImgMap = CreateFileMapping(m_hImgFile, NULL,
		PAGE_READONLY, 0, 0, NULL);
	if (m_hImgMap == NULL)
		throw HRESULT_FROM_WIN32(GetLastError());

	m_pImgView = (PBYTE)MapViewOfFile(m_hImgMap, FILE_MAP_READ, 0, 0, 0);
	if (m_pImgView == NULL)
		throw HRESULT_FROM_WIN32(GetLastError());

	if (SYM_PATH[0] == 0)
	{
		PWSTR pszAppData = NULL;
		SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &pszAppData);
		wsprintf(SYM_PATH, GSZ_APP_TMP_CACHE, pszAppData);
	}
}

PEAnals::~PEAnals()
{
	for (RVA_FNC_MAP::iterator it = RAV_FUNCS.begin(); it != RAV_FUNCS.end(); it++)
		delete it->second;
	for (RVA_THK_MAP::iterator it = RAV_THNKS.begin(); it != RAV_THNKS.end(); it++)
		delete it->second;

	if (m_pImgView != NULL)
		UnmapViewOfFile(m_pImgView);
	if (m_hImgMap != NULL)
		CloseHandle(m_hImgMap);
	if (m_hImgFile != INVALID_HANDLE_VALUE)
		CloseHandle(m_hImgFile);
}

bool PEAnals::SectionHasData(int nSectIdx)
{
	PIMAGE_SECTION_HEADER psh = &m_pshs[nSectIdx];
	CComPtr<IDiaEnumSymbolsByAddr> pIEnumByAddr;
	if (FAILED(DIA_PDB.SESSION->getSymbolsByAddr(&pIEnumByAddr)))
		return false;

	CComPtr<IDiaSymbol> pISymbol;
	if (FAILED(pIEnumByAddr->symbolByAddr(nSectIdx + 1, 0, &pISymbol)))
	{
		pIEnumByAddr = 0;
		return false;
	}

	bool bHasData = true;
	DWORD dwRvaLast = 0;
	if (pISymbol->get_relativeVirtualAddress(&dwRvaLast) == S_OK)
	{
		pISymbol = 0;

		int nItemIdx = 0;
		if (pIEnumByAddr->symbolByRVA(dwRvaLast, &pISymbol) == S_OK)
		{
			DWORD dwSectId = 0;
			if ((pISymbol->get_addressSection(&dwSectId) != S_OK) ||
				(dwSectId != nSectIdx + 1))
				bHasData = false;
			pISymbol = 0;
		}
		else
			bHasData = false;
	}
	pIEnumByAddr = 0;
	return bHasData;
}

int PEAnals::GetImageIndex(PPE_MEMBER pm)
{
	int nImgIdx = IMG_IDX_LEAF;
	if (pm->IsArr)
	{
		if (pm->IsStr)
			nImgIdx = IMG_IDX_STRING;
		else
		{
			if (pm->Type == PE_TYPE::Struct)
				nImgIdx = IMG_IDX_ARRAY;
			else
				nImgIdx = IMG_IDX_LFARR;
		}
	}
	else
	{
		if (pm->Type >= PE_TYPE::Struct)
		{
			if (pm->Type == PE_TYPE::Struct)
				nImgIdx = IMG_IDX_NODE;
			else
				nImgIdx = IMG_IDX_PE;
		}
		else
		{
			if (pm->IsEnum)
				nImgIdx = IMG_IDX_ENUM;
			else if (pm->IsRva)
				nImgIdx = IMG_IDX_RVA; 
			else if (pm->IsVa)
				nImgIdx = IMG_IDX_VA;
		}
	}
	return nImgIdx;
}

CString PEAnals::GetTypeName(PPE_MEMBER pm)
{
	CString sz;
	if (pm->Type >= PE_TYPE::Struct)
		sz = pm->Ref.Struct->Name;
	else
		sz = PE_MEMBER::C_Names[pm->Type];
	if (pm->IsArr)
	{
		sz.AppendFormat(L"[%d]", pm->More.ArrLen);
		if (pm->IsStr)
			sz.Append(L", String");
		else if (pm->IsGuid)
			sz.Append(L", GUID");
	}
	else
	{
		if (pm->IsTime)
			sz.Append(L", Time");
		else if (pm->IsBits)
			sz.AppendFormat(L":%d", pm->More.BitFld.Mask);
		if (pm->IsEnum)
			sz.Append(L", Enum");
	}
	if (pm->IsRva)
		sz.Append(L", RVA");
	else if (pm->IsVa)
		sz.Append(L", VA");
	return sz;
}

CString PEAnals::GetSymbolName(IDiaSession* pISess, DWORD dwRVA)
{
	CString szName;
	CComPtr<IDiaSymbol> pISym;
	if (pISess->findSymbolByRVA(dwRVA, SymTagNull, &pISym) == S_OK)
	{
		DIA_SYMTAG tag = DIA_SYMTAG::SymTagNull;
		pISym->get_symTag((PDWORD)&tag);
		if (tag != SymTagNull)
		{
			CComBSTR bszName;
			if (pISym->get_name(&bszName) == S_OK)
				szName = bszName;

			DWORD dwRVA2 = 0;
			pISym->get_relativeVirtualAddress(&dwRVA2);
			if (dwRVA != dwRVA2)
			{
				if (tag == SymTagBlock)
				{
					CComPtr<IDiaSymbol> pISym2;
					if (pISym->get_lexicalParent(&pISym2) == S_OK)
					{
						CComBSTR bszName2;
						if (pISym2->get_name(&bszName2) == S_OK)
							szName = bszName2;
						pISym2 = 0;
					}
				}
				int nOffV = (int)(dwRVA - dwRVA2);
				if (nOffV < 0)
				{
					nOffV = -nOffV;
					szName.AppendFormat(L"-%xh", nOffV);
				}
				else
					szName.AppendFormat(L"+%xh", nOffV);
			}

		}
		pISym = 0;
	}
	return szName;
}


void PEAnals::InsertNode(PPE_NODE pn, HTREEITEM hParent, int nImgIdx)
{
	if (nImgIdx < 0) nImgIdx = GetImageIndex(pn);
	pn->Node = m_pView->InsertItem(pn->Name, nImgIdx, nImgIdx, hParent);
	m_pView->SetItemData(pn->Node, (DWORD_PTR)pn);

	DWORD dwRVA = (pn->Index == INVALID_SECT_IDX) ? pn->Offset :
		OFFSET_TO_RVA(&m_pshs[pn->Index], pn->Offset);
	CString sz; sz.Format(_T("%08X:%08X"), pn->Offset, dwRVA);
	m_pView->SetItemText(pn->Node, sz, COL_IDX_OFFSET);

	sz.Empty();
	if (pn->IsArr)
	{
		if (pn->Type < PE_TYPE::Struct)
		{
			if (pn->IsStr)
			{
				sz = PE_SCHEMA::GetString(m_pImgView + pn->Offset, pn->Type, pn->More.ArrLen);
				if (pn->More.ArrLen == 0)
				{
					pn->More.ArrLen = sz.GetLength() + 1;
					pn->Size = pn->GetSize();
				}
			}
			else if (pn->IsGuid)
				sz = PEPlus::Bin2GuidStr(m_pImgView + pn->Offset);
			else
			{
				int nLen = pn->More.ArrLen;
				if (pn->Type == PE_TYPE::UInt8 || pn->Type == PE_TYPE::Int8)
				{
					if (pn->IsCode)
					{
						if (nLen > 15) nLen = 15;
					}
					else
					{
						if (nLen > 5) nLen = 5;
					}
				}
				else
				{
					if (nLen > 3) nLen = 3;
				}
				sz = PE_SCHEMA::Bin2Str(m_pImgView + pn->Offset, pn->Type, nLen);
				if (nLen != pn->More.ArrLen)
					sz.Append(L"...");
			}
			m_pView->SetItemText(pn->Node, sz, COL_IDX_VALUE);
		}
	}
	else
	{
		if (pn->Type < PE_TYPE::Struct)
		{
			UINT64 uv = PE_SCHEMA::GetValue(m_pImgView + pn->Offset, pn->Type);
			if (pn->IsBits)
			{
				UINT64 mask = 0;
				for (int i = 0; i < pn->More.BitFld.Mask; i++)
					mask |= (UINT64)(1 << i);
				uv = (uv >> pn->More.BitFld.Shift) & mask;
			}
			m_pView->SetItemText(pn->Node, PE_SCHEMA::Val2Str(uv, pn->Type), COL_IDX_VALUE);
			if (pn->IsEnum)
			{
				if (pn->Ref.Enum->Flags)
					sz = PE_SCHEMA::GetEnumFlags(uv, pn->Ref.Enum);
				else
					sz = pn->Ref.Enum->Find(uv);
			}
			else if (pn->IsTime)
			{
				if (uv != 0)
					sz = PEPlus::Int2TimeStr((DWORD)uv);
			}
			else if (pn->HasInfo)
				sz = pn->Ref.Info;
			else if (pn->IsRva)
			{
				if (uv == 0)
					m_pView->SetItemImage(pn->Node, IMG_IDX_LFNO, IMG_IDX_LFNO);
				else
				{
					PIMAGE_SECTION_HEADER psh = PEPlus::FindSectHdr(m_pImgView, (DWORD)uv);
					if (psh != NULL)
					{
						sz.Format(L"[%-8s]0x%08X", PEPlus::GetSectionName(psh),
							RVA_TO_OFFSET(psh, (DWORD)uv));
					}
				}
			}
			else if (pn->IsVa)
			{
				if (uv == 0)
					m_pView->SetItemImage(pn->Node, IMG_IDX_LFNO, IMG_IDX_LFNO);
				else
				{
					DWORD dwRVA2 = (DWORD)(uv - PEPlus::GetImageBase(m_pImgView));
					if (dwRVA2 > 0)
					{
						PIMAGE_SECTION_HEADER psh = PEPlus::FindSectHdr(m_pImgView, dwRVA2);
						if (psh != NULL)
							sz.Format(L"R:0x%08X:[%-8s]0x%08X", dwRVA,
							PEPlus::GetSectionName(psh), RVA_TO_OFFSET(psh, dwRVA2));
						else
							sz.Format(L"R:0x%08X", dwRVA2);
					}
					else
						sz.Format(L"R:0x%08X", dwRVA2);
				}
			}
			else if (pn->IsSize)
			{
				if (uv > 1023)
					sz.Format(L"%s(%s)", PEPlus::Val2CommaStr(uv), PEPlus::Size2Units(uv));
				else
					sz.Format(L"%sB", PEPlus::Val2CommaStr(uv));
			}
			if (!sz.IsEmpty())
				m_pView->SetItemText(pn->Node, sz, COL_IDX_INFO);
		}
		else if (pn->Type == PE_TYPE::PEFormat)
		{
			m_pView->SetItemText(pn->Node, pn->Ref.Info, COL_IDX_INFO);
		}
	}
	if (pn->Type == PE_TYPE::PEFormat)
	{
		PCWSTR pszType = PathFindExtension(m_szPEPath);
		CString sz; sz.Format(L"%dbit %s PE", (m_bIs32Bit ? 32 : 64), pszType + 1);
		m_pView->SetItemText(pn->Node, sz, COL_IDX_TYPE);
	}
	else
	{
		m_pView->SetItemText(pn->Node, GetTypeName(pn), COL_IDX_TYPE);
	}

	sz.Format(_T("0x%X(%d)"), pn->Size, pn->Size);
	m_pView->SetItemText(pn->Node, sz, COL_IDX_SIZE);
}

DWORD PEAnals::AppendStructMembers(PPE_NODE pn, DWORD dwVarSize)
{
	DWORD dwOffset = pn->Offset, dwSize = 0;
	PE_TYPE dtBitFld = PE_TYPE::Struct;
	short nBFldPos = 0;

	PPE_STRUCT pst = pn->Ref.Struct;
	for (MBR_LIST::iterator it = pst->Members.begin(); it != pst->Members.end(); it++)
	{
		PPE_NODE ppn = new PE_NODE(pn->Index, dwOffset, *it);
		InsertNode(ppn, pn->Node);
		if (ppn->IsBits)
		{
			nBFldPos += ppn->More.BitFld.Mask;
			if (dtBitFld == PE_TYPE::Struct)
			{
				dtBitFld = ppn->Type;
			}
			else
			{
				if (nBFldPos == PE_MEMBER::C_Sizes[dtBitFld] * 8)
				{
					dtBitFld = PE_TYPE::Struct;
					nBFldPos = 0;

					dwOffset += ppn->Size;
					dwSize += ppn->Size;
				}
			}
		}
		else
		{
			dwOffset += ppn->Size;
			dwSize += ppn->Size;
		}
		if (dwVarSize > 0 && dwSize >= dwVarSize)
			break;
	}
	if (pn->Size != dwSize)
	{
		pn->Size = dwSize;
		CString sz; sz.Format(L"0x%X(%d)", dwSize, dwSize);
		UpdateNodeText(pn->Node, sz, COL_IDX_SIZE);
	}
	return dwSize;
}

PPE_NODE PEAnals::FindNode(HTREEITEM hNode, PCTSTR pszName, bool bSibling)
{
	HTREEITEM hIter = bSibling ? hNode : m_pView->GetChildItem(hNode);
	while (hIter != NULL)
	{
		PPE_NODE pn = (PPE_NODE)m_pView->GetItemData(hIter);
		if (pn->Name.Compare(pszName) == 0)
			return pn;
		hIter = m_pView->GetNextSiblingItem(hIter);
	}
	return NULL;
}

PPE_NODE PEAnals::FindNode(HTREEITEM hNode, DWORD dwOffset, bool bSibling)
{
	HTREEITEM hIter = bSibling ? hNode : m_pView->GetChildItem(hNode);
	while (hIter != NULL)
	{
		PPE_NODE pn = (PPE_NODE)m_pView->GetItemData(hIter);
		if (pn->Offset == dwOffset)
			return pn;
		hIter = m_pView->GetNextSiblingItem(hIter);
	}
	return NULL;
}

PPE_NODE PEAnals::FindRVANode(HTREEITEM hNode, DWORD dwRVA)
{
	HTREEITEM hIter = m_pView->GetChildItem(hNode);
	while (hIter != NULL)
	{
		PPE_NODE pn = (PPE_NODE)m_pView->GetItemData(hIter);
		if (pn->IsRva)
		{
			UINT64 uv = PE_SCHEMA::GetValue(m_pImgView + pn->Offset, (PE_TYPE)pn->Type);
			if (dwRVA == (DWORD)uv)
				return pn;
		}
		if (m_pView->ItemHasChildren(hIter))
		{
			pn = FindRVANode(hIter, dwRVA);
			if (pn != NULL)
				return pn;
		}
		hIter = m_pView->GetNextSiblingItem(hIter);
	}
	return NULL;
}

PPE_NODE PEAnals::UpdateRVANode(PPE_NODE pn, DWORD dwRVA, HTREEITEM hFind)
{
	PPE_NODE prv = FindRVANode
		((hFind == NULL) ? pn->Node : hFind, dwRVA);
	if (prv == NULL)
		return prv;

	prv->Ref.Rva = pn;
	CString sz;
	sz.Format(L"%s, %s:%s", m_pView->GetItemText
		(prv->Node, COL_IDX_INFO), pn->Name, GetTypeName(pn));
	m_pView->SetItemText(prv->Node, sz, COL_IDX_INFO);
	return prv;
}

int PEAnals::ParsePEHeaders()
{
	// IMAGE_DOS_HEADER
	DWORD dwOffset = 0;
	PIMAGE_DOS_HEADER pdh = PIMAGE_DOS_HEADER(m_pImgView);
	if (pdh->e_magic != IMAGE_DOS_SIGNATURE)
		throw _T("윈도우 실행 파일 포맷이 아닙니다");
	PPE_NODE pn = InsertStructNode(m_pnRoot->Node, 
		INVALID_SECT_IDX, dwOffset, L"DosHeader", L"IMAGE_DOS_HEADER");
	AppendStructMembers(pn);
	dwOffset = pdh->e_lfanew;

	// IMAGE_NT_HEADERS
	PBYTE pIter = m_pImgView + dwOffset;
	DWORD dwSig = *PDWORD(pIter); pIter += sizeof(DWORD);
	if (dwSig != IMAGE_NT_SIGNATURE)
		throw _T("PE 포맷을 가진 파일이 아닙니다");
	PCWSTR pszType = (m_bIs32Bit) ? L"IMAGE_NT_HEADERS32" : L"IMAGE_NT_HEADERS64";
	pn = InsertStructNode(m_pnRoot->Node, INVALID_SECT_IDX, dwOffset, L"NTHeaders", pszType);
	pn->Kind = NK_HDR;
	dwOffset += AppendStructMembers(pn);

	// IMAGE_FILE_HEADER
	PPE_NODE pn2 = FindNode(pn->Node, L"FileHeader");
	dwOffset += AppendStructMembers(pn2);

	// IMAGE_OPTIONAL_HEADER
	pn2 = FindNode(pn->Node, L"OptionalHeader");
	pn2->Kind = NK_HDR;
	dwOffset += AppendStructMembers(pn2);

	// IMAGE_DATA_DIRECTORY
	m_pdds = PEPlus::GetDataDirs(m_pImgView);
	PPE_ENUM pDirEnt = SCHEMA.EnumMap()->find(L"IMAGE_DIRECTORY_ENTRY")->second;
	PPE_NODE pn3 = FindNode(pn2->Node, L"DataDirectory");
	pn3->Kind = NK_HDR;
	dwOffset = pn3->Offset;
	for (int i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; i++)
	{
		PIMAGE_DATA_DIRECTORY pdd = &m_pdds[i];
		int nImgIdx = (pdd->VirtualAddress == 0) ? IMG_IDX_XBOX : -1;
		CString sz; sz.Format(L"[%2d]%s", i, pDirEnt->Find(i));
		PPE_NODE pn4 = InsertStructNode(pn3->Node, 
			pn3->Index, dwOffset, sz, L"IMAGE_DATA_DIRECTORY", 0, nImgIdx);
		if (pdd->VirtualAddress > 0)
		{
			AppendStructMembers(pn4);
			PIMAGE_SECTION_HEADER psh = PEPlus::FindSectHdr(m_pImgView, pdd->VirtualAddress);
			CString szOffset;
			DWORD dwDataOff = pdd->VirtualAddress;
			if (psh != NULL)
				szOffset.Format(L"%s:%X", PEPlus::GetSectionName(psh),
				RVA_TO_OFFSET(psh, pdd->VirtualAddress));
			else
				szOffset.Format(L"HDR:%X", pdd->VirtualAddress);
			sz.Format(L"0x%08X(%s)+%d", pdd->VirtualAddress, szOffset, pdd->Size);
			UpdateNodeText(pn4->Node, sz, COL_IDX_INFO);
		}
		dwOffset += sizeof(IMAGE_DATA_DIRECTORY);
	}

	// IMAGE_SECTION_HEADER
	m_pshs = PEPlus::GetSectHdrs(m_pImgView);
	PIMAGE_FILE_HEADER pfh = PIMAGE_FILE_HEADER(pIter);
	pn = InsertStructNode(m_pnRoot->Node, INVALID_SECT_IDX, dwOffset,
		L"SectionHeaders", L"IMAGE_SECTION_HEADER", pfh->NumberOfSections);
	for (int i = 0; i < pfh->NumberOfSections; i++)
	{
		PIMAGE_SECTION_HEADER psh = &m_pshs[i];
		CString sz; sz.Format(L"[%d]%s", i, PEPlus::GetSectionName(psh));
		PPE_NODE pn4 = InsertStructNode(pn->Node, 
			pn->Index, dwOffset, sz, L"IMAGE_SECTION_HEADER");
		pn4->Kind = NK_HDR;
		sz.Format(L"(R)0x%04X:(O)0x%04X +%d", 
			psh->VirtualAddress, psh->PointerToRawData, psh->Misc.VirtualSize);
		UpdateNodeText(pn4->Node, sz, COL_IDX_INFO);
		dwOffset += AppendStructMembers(pn4);
	}

	return (int)pfh->NumberOfSections;
}

PPE_NODE PEAnals::ParsePEImage(SORT_SET& sorts)
{
	m_bIs32Bit = PEPlus::Is32bitPE(m_pImgView);
	DWORD dwTimeStamp = 0;
	PIMAGE_PDB_INFO ppi = PEPlus::GetPdbInfo(m_pImgView, &dwTimeStamp);
	if (ppi != NULL)
		DIA_PDB.LoadPdbFile(ppi, dwTimeStamp, SYM_PATH);

	m_pnRoot = InsertPERootNode();
	int nNumOfSec = ParsePEHeaders();
	for (int i = 0; i < nNumOfSec; i++)
	{
		PIMAGE_SECTION_HEADER psh = &m_pshs[i];
		if (psh->SizeOfRawData == 0)
		{
			m_pnSects.push_back(NULL);
			continue;
		}

		DWORD dwOffset = psh->PointerToRawData;
		PPE_NODE pn = InsertFieldNode(m_pnRoot->Node, (short)i, dwOffset, 
			PEPlus::GetSectionName(psh), PE_TYPE::UInt8, psh->SizeOfRawData, IMG_IDX_SECT);
		pn->Kind = NK_SECT;
		if (psh->Characteristics & IMAGE_SCN_CNT_CODE)
			pn->SubT |= PE_KIND_CODE;
		if (psh->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)
		{
			if (DIA_PDB.Loaded() && SectionHasData(i))
				pn->SubT |= PE_KIND_DATA;
		}

		CString szAttrs = L"_______";
		CONST WCHAR SECT_ATTRS[7] = { L'W', L'R', L'E', L'S', L'P', L'K', L'D' };
		DWORD dwMasks = IMAGE_SCN_MEM_WRITE;
		for (int j = 0; j < 7; j++)
		{
			if (psh->Characteristics & dwMasks)
				szAttrs.SetAt(j, SECT_ATTRS[j]);
			dwMasks >>= 1;
		}
		UpdateNodeText(pn->Node, szAttrs, COL_IDX_INFO);
		m_pnSects.push_back(pn);
	}

	PPE_ENUM pDirEnt = SCHEMA.EnumMap()->find(L"IMAGE_DIRECTORY_ENTRY")->second;
	for (int i = 0; i < IMAGE_NUMBEROF_DIRECTORY_ENTRIES - 1; i++)
	{
		PIMAGE_DATA_DIRECTORY pdd = &m_pdds[i];
		if (pdd->VirtualAddress == 0)
			continue;

		DWORD dwOffset = 0;
		HTREEITEM hParent = NULL;
		short nSectIdx = INVALID_SECT_IDX;
		if (i == IMAGE_DIRECTORY_ENTRY_SECURITY)
		{
			dwOffset = pdd->VirtualAddress;
			hParent = m_pnRoot->Node;
		}
		else
		{
			nSectIdx = PEPlus::GetSectionIdx(m_pImgView, pdd->VirtualAddress);
			if (nSectIdx == INVALID_SECT_IDX)
			{
				if (i != IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT)
					continue;
				dwOffset = pdd->VirtualAddress;
				hParent = m_pnRoot->Node;
			}
			else
			{
				dwOffset = RVA_TO_OFFSET(&m_pshs[nSectIdx], pdd->VirtualAddress);
				hParent = m_pnSects.at(nSectIdx)->Node;
			}
		}

		PPE_NODE pn = InsertFieldNode(hParent, nSectIdx, dwOffset,
			pDirEnt->Find(i), PE_TYPE::UInt8, pdd->Size, IMG_IDX_DIRS);
		pn->Kind = NK_DDIR;
		pn->SubT = (BYTE)i;
		PFN_PARSE_DATADIR pfnParse = m_pfnddrs[i];
		(this->*pfnParse)(pn, pdd);
	}

	ParseTextSection(nNumOfSec);

	for (int i = 0; i < (int)m_pnSects.size(); i++)
	{
		if (m_pnSects.at(i) != NULL)
			sorts.insert(m_pnSects.at(i)->Node);
	}

	return m_pnRoot;
}
