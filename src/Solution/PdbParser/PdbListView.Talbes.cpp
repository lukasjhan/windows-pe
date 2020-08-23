#include "stdafx.h"
#include "PdbDoc.h"
#include "PdbListView.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif

PCWSTR COLUMN_SYMBOLS[] =
{
	L"SymID", L"SymTag", L"Name", L"Location"
};
PCWSTR COLUMN_SEC_CONBS[] =
{
	L"Compliand", L"RVA", L"Seg:Offset", L"Length", L"SymbolTag", L"SymbolName"
};
PCWSTR COLUMN_SRC_FILES[] =
{
	L"SrcID", L"FileName", L"FilePath", L"Compliands", L"CheckSumType", L"CheckSum"
};
PCWSTR COLUMN_LINE_NUMS[] =
{
	L"LineNum", L"RVA", L"Seg::Offset", L"Length", L"SrcFile", L"SrcPath"
};
PCWSTR COLUMN_SEGMENTS[] =
{
	L"Name", L"SegID", L"RVA", L"Sect:Offset", L"Size", L"Attributes"
};
PCWSTR COLUMN_FRAME_DATA[] =
{
	L"RVA", L"Sect:Offset", L"Function", L"BlockLen", L"LocalLen", L"ParamLen", 
	L"MaxStack", L"PrologLen", L"SavedRegLen", L"Attr", L"Progname"
};

PCWSTR* CPdbListView::PSZ_COLUMS[6]
{
	COLUMN_SYMBOLS,
	COLUMN_SEC_CONBS,
	COLUMN_SRC_FILES,
	COLUMN_LINE_NUMS,
	COLUMN_SEGMENTS,
	COLUMN_FRAME_DATA
};

INT CPdbListView::PCNT_COLUMS[6] = 
{
	sizeof(COLUMN_SYMBOLS)	  / sizeof(PCWSTR),
	sizeof(COLUMN_SEC_CONBS)  / sizeof(PCWSTR),
	sizeof(COLUMN_SRC_FILES)  / sizeof(PCWSTR),
	sizeof(COLUMN_LINE_NUMS)  / sizeof(PCWSTR),
	sizeof(COLUMN_SEGMENTS)	  / sizeof(PCWSTR),
	sizeof(COLUMN_FRAME_DATA) / sizeof(PCWSTR)
};


void CPdbListView::IterateTableSymbols(IDiaTable* pITable)
{
	CPdbListView* pList = (CPdbListView*)&GetListCtrl();
	IDiaSession* pISess = pList->GetDocument()->DIA_SESSION;

	int nItemIdx = 0;
	ULONG ulCelt = 0;
	CComPtr<IDiaEnumSymbols> pIEnums;
	if (SUCCEEDED(pITable->QueryInterface(_uuidof(IDiaEnumSymbols), (PVOID*)&pIEnums)))
	{
		CComPtr<IDiaSymbol> pIRec;
		while (SUCCEEDED(pIEnums->Next(1, &pIRec, &ulCelt)) && ulCelt == 1)
		{
			DWORD tag;
			CComBSTR bszName;
			if (pIRec->get_symTag(&tag) == S_OK && pIRec->get_name(&bszName) == S_OK)
			{
				DWORD dwId;
				pIRec->get_symIndexId(&dwId);
				CString sz;
				sz.Format(L"%d", dwId);

				LocationType eLocType = LocationType::LocIsNull;
				pIRec->get_locationType((PDWORD)&eLocType);

				int nLvItem = pList->GetListCtrl().InsertItem(nItemIdx, sz);
				pList->GetListCtrl().SetItem(nLvItem, 1, TVIF_TEXT, PEPdb::GSZ_SYM_TAGS[tag], 0, 0, 0, 0);
				pList->GetListCtrl().SetItem(nLvItem, 2, TVIF_TEXT, bszName, 0, 0, 0, 0);
				pList->GetListCtrl().SetItem(nLvItem, 3, TVIF_TEXT, PEPdb::GSZ_LOC_TYPES[eLocType], 0, 0, 0, 0);
				pList->GetListCtrl().SetItemData(nLvItem,
					DWORD_PTR(new DIA_NODE(NODE_SYMBOL, (WORD)tag, pIRec)));
				nItemIdx++;
			}
			pIRec = 0;
		}
		pIEnums = 0;
	}
}

void CPdbListView::IterateTableSectionContribs(IDiaTable* pITable)
{
	CPdbListView* pList = (CPdbListView*)&GetListCtrl();
	IDiaSession* pISess = pList->GetDocument()->DIA_SESSION;

	int nItemIdx = 0;
	ULONG ulCelt = 0;
	CComPtr<IDiaEnumSectionContribs> pIEnums;
	if (SUCCEEDED(pITable->QueryInterface(_uuidof(IDiaEnumSectionContribs), (PVOID*)&pIEnums)))
	{
		CComPtr<IDiaSectionContrib> pIRec;
		while (SUCCEEDED(pIEnums->Next(1, &pIRec, &ulCelt)) && ulCelt == 1)
		{
			DWORD dwRVA, dwSect, dwOffset, dwLen;
			CComPtr<IDiaSymbol> pICompiland;
			CComBSTR bszName;
			if (pIRec->get_relativeVirtualAddress(&dwRVA) == S_OK	&&
				pIRec->get_addressSection(&dwSect) == S_OK &&
				pIRec->get_addressOffset(&dwOffset) == S_OK &&
				pIRec->get_length(&dwLen) == S_OK &&
				pIRec->get_compiland(&pICompiland) == S_OK &&
				pICompiland->get_name(&bszName) == S_OK)
			{
				CString sz;
				sz = SplitFilePath(bszName);
				int nLvItem = pList->GetListCtrl().InsertItem(nItemIdx, sz);

				sz.Format(L"%08X", dwRVA);
				pList->GetListCtrl().SetItem(nLvItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
				sz.Format(L"%04X:%08X", dwSect, dwOffset);
				pList->GetListCtrl().SetItem(nLvItem, 2, TVIF_TEXT, sz, 0, 0, 0, 0);
				sz.Format(L"0x%X (%d)", dwLen, dwLen);
				pList->GetListCtrl().SetItem(nLvItem, 3, TVIF_TEXT, sz, 0, 0, 0, 0);

				CComPtr<IDiaSymbol> pISym;	
				if (pISess->findSymbolByRVA(dwRVA, SymTagNull, &pISym) == S_OK)
				{
					DWORD tag;
					if (pISym->get_symTag(&tag) == S_OK && pISym->get_name(&bszName) == S_OK)
					{
						pList->GetListCtrl().SetItem(nLvItem, 4, TVIF_TEXT, PEPdb::GSZ_SYM_TAGS[tag], 0, 0, 0, 0);
						pList->GetListCtrl().SetItem(nLvItem, 5, TVIF_TEXT, bszName, 0, 0, 0, 0);
					}
					pISym = 0;
				}
				nItemIdx++;
				pICompiland = 0;
			}
			pIRec = 0;
		}
		pIEnums = 0;
	}
}

void CPdbListView::IterateTableSourceFiles(IDiaTable* pITable)
{
	CPdbListView* pList = (CPdbListView*)&GetListCtrl();
	IDiaSession* pISess = pList->GetDocument()->DIA_SESSION;

	int nItemIdx = 0;
	ULONG ulCelt = 0;
	CComPtr<IDiaEnumSourceFiles> pIEnums;
	if (SUCCEEDED(pITable->QueryInterface(_uuidof(IDiaEnumSourceFiles), (PVOID*)&pIEnums)))
	{
		CComPtr<IDiaSourceFile> pIRec;
		while (SUCCEEDED(pIEnums->Next(1, &pIRec, &ulCelt)) && ulCelt == 1)
		{
			CComBSTR bszSrcName;
			if (pIRec->get_fileName(&bszSrcName) == S_OK)
			{
				DWORD dwKey = 0;
				pIRec->get_uniqueId(&dwKey);
				CString sz; sz.Format(L"%d", dwKey);
				int nLvItem = pList->GetListCtrl().InsertItem(nItemIdx, sz);

				CString szName = SplitFilePath(bszSrcName, &sz);
				pList->GetListCtrl().SetItem(nLvItem, 1, TVIF_TEXT, (PCWSTR)szName, 0, 0, 0, 0);
				pList->GetListCtrl().SetItem(nLvItem, 2, TVIF_TEXT, sz, 0, 0, 0, 0);

				sz.Empty();
				CComPtr<IDiaEnumSymbols> pIEnumCpls;
				if (pIRec->get_compilands(&pIEnumCpls) == S_OK)
				{
					CComPtr<IDiaSymbol> pICompliand;
					while (SUCCEEDED(pIEnumCpls->Next(1, &pICompliand, &ulCelt)) && ulCelt == 1)
					{
						if (pICompliand->get_name(&bszSrcName) == S_OK)
						{
							if (!sz.IsEmpty()) sz.Append(L",");
							szName = SplitFilePath(bszSrcName);
							sz.Append(szName);
						}
						pICompliand = 0;
					}
					pIEnumCpls = 0;
					pList->GetListCtrl().SetItem(nLvItem, 3, TVIF_TEXT, sz, 0, 0, 0, 0);
				}

				BYTE checksum[256];
				DWORD cbChecksum = sizeof(checksum);
				if (pIRec->get_checksum(cbChecksum, &cbChecksum, checksum) == S_OK)
				{
					DWORD checksumType;
					if (pIRec->get_checksumType(&checksumType) == S_OK)
					{
						switch (checksumType)
						{
							case CHKSUM_TYPE_NONE: sz = L"None"; break;
							case CHKSUM_TYPE_MD5 : sz = L"MD5" ; break;
							case CHKSUM_TYPE_SHA1: sz = L"SHA1"; break;
							default	: sz.Format(L"0x%X", checksumType); break;
						}
						pList->GetListCtrl().SetItem(nLvItem, 4, TVIF_TEXT, sz, 0, 0, 0, 0);
					}

					sz.Empty();
					for (DWORD ib = 0; ib < cbChecksum; ib++)
						sz.AppendFormat(L"%02X ", checksum[ib]);
					pList->GetListCtrl().SetItem(nLvItem, 5, TVIF_TEXT, sz, 0, 0, 0, 0);
				}
				nItemIdx++;
			}
			pIRec = 0;
		}
		pIEnums = 0;
	}
}

void CPdbListView::IterateTableLineNumbers(IDiaTable* pITable)
{
	CPdbListView* pList = (CPdbListView*)&GetListCtrl();
	IDiaSession* pISess = pList->GetDocument()->DIA_SESSION;
	int nItemIdx = 0;

	ULONG ulCelt = 0;
	CComPtr<IDiaEnumLineNumbers> pIEnums;
	if (SUCCEEDED(pITable->QueryInterface(_uuidof(IDiaEnumLineNumbers), (PVOID*)&pIEnums)))
	{
		CComPtr<IDiaLineNumber> pIRec;
		while (SUCCEEDED(pIEnums->Next(1, &pIRec, &ulCelt)) && ulCelt == 1)
		{
			DWORD dwRVA, dwSeg, dwOffset;
			DWORD dwLinenum, dwSrcId, dwLength;
			if (pIRec->get_relativeVirtualAddress(&dwRVA) == S_OK &&
				pIRec->get_addressSection(&dwSeg) == S_OK &&
				pIRec->get_addressOffset(&dwOffset) == S_OK &&
				pIRec->get_lineNumber(&dwLinenum) == S_OK &&
				pIRec->get_sourceFileId(&dwSrcId) == S_OK &&
				pIRec->get_length(&dwLength) == S_OK)
			{
				CString sz;
				sz.Format(L"%u", dwLinenum);
				int nLvItem = pList->GetListCtrl().InsertItem(nItemIdx, sz);
				sz.Format(L"%08X", dwRVA);
				pList->GetListCtrl().SetItem(nLvItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
				sz.Format(L"%04X:%08X", dwSeg, dwOffset);
				pList->GetListCtrl().SetItem(nLvItem, 2, TVIF_TEXT, sz, 0, 0, 0, 0);
				sz.Format(L"0x%X (%d)", dwLength, dwLength);
				pList->GetListCtrl().SetItem(nLvItem, 3, TVIF_TEXT, sz, 0, 0, 0, 0);

				CComPtr<IDiaSourceFile> pISource;
				if (pIRec->get_sourceFile(&pISource) == S_OK)
				{
					CComBSTR bszSrcName;
					if (pISource->get_fileName(&bszSrcName) == S_OK)
					{
						CString szPath;
						sz = SplitFilePath(bszSrcName, &szPath);
						pList->GetListCtrl().SetItem(nLvItem, 4, TVIF_TEXT, sz, 0, 0, 0, 0);
						pList->GetListCtrl().SetItem(nLvItem, 5, TVIF_TEXT, szPath, 0, 0, 0, 0);
					}
					pISource = 0;
				}
				nItemIdx++;
			}
			pIRec = 0;
		}
		pIEnums = 0;
	}
}

void CPdbListView::IterateTableSegments(IDiaTable* pITable)
{
	CPdbListView* pList = (CPdbListView*)&GetListCtrl();
	IDiaSession* pISess = pList->GetDocument()->DIA_SESSION;
	int nItemIdx = 0;

	CComPtr<IDiaEnumDebugStreams> pIEnumStrms;
	if (FAILED(pISess->getEnumDebugStreams(&pIEnumStrms)))
		return;

	BYTE	data[1024];
	PBYTE	pIter = data;
	LONG	nSecCnt = 0;
	ULONG	ulCelt = 0;
	CComPtr<IDiaEnumDebugStreamData> pIStrm;
	while (SUCCEEDED(pIEnumStrms->Next(1, &pIStrm, &ulCelt)) && ulCelt == 1)
	{
		CComBSTR bszName;
		if (pIStrm->get_name(&bszName) == S_OK && wcsicmp(bszName, L"SECTIONHEADERS") == 0)
		{
			pIStrm->get_Count(&nSecCnt);

			DWORD cbData, cbTotal = 0;
			while (pIStrm->Next(1, sizeof(data), &cbData, pIter, &ulCelt) == S_OK)
			{
				pIter += cbData;
				cbTotal += cbData;
				cbData -= cbData;
			}
		}
		pIStrm = 0;
	}
	pIEnumStrms = 0;

	PIMAGE_SECTION_HEADER pshs = PIMAGE_SECTION_HEADER(data);
	CComPtr<IDiaEnumSegments> pIEnums;
	if (SUCCEEDED(pITable->QueryInterface(_uuidof(IDiaEnumSegments), (PVOID*)&pIEnums)))
	{
		CComPtr<IDiaSegment> pIRec;
		while (SUCCEEDED(pIEnums->Next(1, &pIRec, &ulCelt)) && ulCelt == 1)
		{
			DWORD dwRVA, seg, dwFrame, dwOffset, dwSize;
			pIRec->get_length(&dwSize);
			pIRec->get_offset(&dwOffset);
			pIRec->get_frame(&dwFrame);
			pIRec->get_addressSection(&seg);
			if (pIRec->get_relativeVirtualAddress(&dwRVA) == S_OK)
			{
				char szSecName[IMAGE_SIZEOF_SHORT_NAME + 1] = { 0, };
				for (int i = 0; i < nSecCnt; i++)
				{
					if (pshs[i].VirtualAddress == dwRVA)
					{
						memcpy(szSecName, pshs[i].Name, IMAGE_SIZEOF_SHORT_NAME);
						szSecName[IMAGE_SIZEOF_SHORT_NAME] = 0;
						break;
					}
				}
				USES_CONVERSION;
				int nLvItem = pList->GetListCtrl().InsertItem(nItemIdx, A2CW(szSecName));
				CString sz;
				sz.Format(L"%d", dwFrame);
				pList->GetListCtrl().SetItem(nLvItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
				sz.Format(L"0x%08X", dwRVA);
				pList->GetListCtrl().SetItem(nLvItem, 2, TVIF_TEXT, sz, 0, 0, 0, 0);
				sz.Format(L"%04X:%08X", seg, dwOffset);
				pList->GetListCtrl().SetItem(nLvItem, 3, TVIF_TEXT, sz, 0, 0, 0, 0);
				sz.Format(L"%d(0x%X)", dwSize, dwSize);
				pList->GetListCtrl().SetItem(nLvItem, 4, TVIF_TEXT, sz, 0, 0, 0, 0);

				sz = L"___";
				BOOL bOK;
				if (pIRec->get_read(&bOK) == S_OK && bOK)
					sz.SetAt(0, L'R');
				if (pIRec->get_write(&bOK) == S_OK && bOK)
					sz.SetAt(1, L'W');
				if (pIRec->get_execute(&bOK) == S_OK && bOK)
					sz.SetAt(2, L'E');
				pList->GetListCtrl().SetItem(nLvItem, 5, TVIF_TEXT, sz, 0, 0, 0, 0);

				pIRec = 0;
				nItemIdx++;
			}
		}
		pIEnums = 0;
	}
}

void CPdbListView::IterateTableFrameData(IDiaTable* pITable)
{
	CPdbListView* pList = (CPdbListView*)&GetListCtrl();
	IDiaSession* pISess = pList->GetDocument()->DIA_SESSION;
	int nItemIdx = 0;

	ULONG ulCelt = 0;
	CComPtr<IDiaEnumFrameData> pIEnums;
	if (SUCCEEDED(pITable->QueryInterface(_uuidof(IDiaEnumFrameData), (PVOID*)&pIEnums)))
	{
		CComPtr<IDiaFrameData> pIRec;
		while (SUCCEEDED(pIEnums->Next(1, &pIRec, &ulCelt)) && ulCelt == 1)
		{
			DWORD dwSect, dwOffset, dwRVA;
			DWORD cbBlock, cbLocals, cbParams;
			DWORD cbMaxStack, cbProlog, cbSavedRegs;
			BOOL bSEH, bEH, bStart;
			if (pIRec->get_relativeVirtualAddress(&dwRVA) == S_OK &&
				pIRec->get_addressSection(&dwSect) == S_OK &&
				pIRec->get_addressOffset(&dwOffset) == S_OK &&
				pIRec->get_lengthBlock(&cbBlock) == S_OK &&
				pIRec->get_lengthLocals(&cbLocals) == S_OK &&
				pIRec->get_lengthParams(&cbParams) == S_OK &&
				pIRec->get_maxStack(&cbMaxStack) == S_OK &&
				pIRec->get_lengthProlog(&cbProlog) == S_OK &&
				pIRec->get_lengthSavedRegisters(&cbSavedRegs) == S_OK &&
				pIRec->get_systemExceptionHandling(&bSEH) == S_OK &&
				pIRec->get_cplusplusExceptionHandling(&bEH) == S_OK &&
				pIRec->get_functionStart(&bStart) == S_OK)
			{
				CString sz;
				sz.Format(L"0x%08X", dwRVA);
				int nLvItem = pList->GetListCtrl().InsertItem(nItemIdx, sz);
				sz.Format(L"%04X:%08X", dwSect, dwOffset);
				pList->GetListCtrl().SetItem(nLvItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);

				CComPtr<IDiaSymbol> pISym;
				if (pISess->findSymbolByRVA(dwRVA, SymTagFunction, &pISym) == S_OK)
				{
					CComBSTR bszName;
					if (pISym->get_name(&bszName) == S_OK)
						pList->GetListCtrl().SetItem(nLvItem, 2, TVIF_TEXT, bszName, 0, 0, 0, 0);
					pISym = 0;
				}

				sz.Format(L"%d", cbBlock);
				pList->GetListCtrl().SetItem(nLvItem, 3, TVIF_TEXT, sz, 0, 0, 0, 0);
				sz.Format(L"%d", cbLocals);
				pList->GetListCtrl().SetItem(nLvItem, 4, TVIF_TEXT, sz, 0, 0, 0, 0);
				sz.Format(L"%d", cbParams);
				pList->GetListCtrl().SetItem(nLvItem, 5, TVIF_TEXT, sz, 0, 0, 0, 0);
				sz.Format(L"%d", cbMaxStack);
				pList->GetListCtrl().SetItem(nLvItem, 6, TVIF_TEXT, sz, 0, 0, 0, 0);
				sz.Format(L"%d", cbProlog);
				pList->GetListCtrl().SetItem(nLvItem, 7, TVIF_TEXT, sz, 0, 0, 0, 0);
				sz.Format(L"%d", cbSavedRegs);
				pList->GetListCtrl().SetItem(nLvItem, 8, TVIF_TEXT, sz, 0, 0, 0, 0);

				sz.Empty();
				if (bSEH)	sz.Append(L"|SEH");
				if (bSEH)	sz.Append(L"|EH");
				if (bSEH)	sz.Append(L"|FuncStart");
				if (!sz.IsEmpty())
					sz = sz.Mid(1);
				pList->GetListCtrl().SetItem(nLvItem, 9, TVIF_TEXT, sz, 0, 0, 0, 0);

				CComBSTR bszProg;
				if (pIRec->get_program(&bszProg) == S_OK)
					pList->GetListCtrl().SetItem(nLvItem, 10, TVIF_TEXT, bszProg, 0, 0, 0, 0);

				nItemIdx++;
			}
			pIRec = 0;
		}
		pIEnums = 0;
	}
}
