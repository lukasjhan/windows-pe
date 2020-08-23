#include "stdafx.h"
#	ifndef SHARED_HANDLERS
#include "PdbParser.h"
#endif
#include "PdbDoc.h"
#include "PdbListView.h"
#include "PdbPropView.h"
#include "PdbTextView.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


PCWSTR COL_SYMB_UDT[] =
{
	L"Kind", L"Length", L"Parent", L"Attributes"	//4
};
PCWSTR COL_SYMB_ENUMDEF[] =
{
	L"Type", L"Length", L"Nested"	//3
};
PCWSTR COL_SYMB_FUNC[] =
{
	L"RVA", L"Seg:Offset", L"Length", L"Attributes", L"Type"	//5
};
PCWSTR COL_SYMB_DATA[] =
{
	L"Kind", L"Type", L"Length", L"Location", L"RVA", L"Seg:Offset"	//6
};
PCWSTR COL_SYMB_PUBSYMS[] =
{
	 L"Attributes", L"Length", L"Location", L"RVA", L"Seg:Offset"		//7
};
PCWSTR COL_SYMB_COMPLND[] =
{
	L"Path", L"Language", L"Length", L"Target"	// 4
};



// CPdbListView
IMPLEMENT_DYNCREATE(CPdbListView, CListView)
BEGIN_MESSAGE_MAP(CPdbListView, CListView)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CPdbListView::OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CPdbListView::OnLvnItemchanged)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, &CPdbListView::OnLvnDeleteitem)
END_MESSAGE_MAP()

// CPdbListView 생성/소멸
CPdbListView::CPdbListView()
{
	m_pfnItrTbls[0] = &CPdbListView::IterateTableSymbols;
	m_pfnItrTbls[1] = &CPdbListView::IterateTableSectionContribs;
	m_pfnItrTbls[2] = &CPdbListView::IterateTableSourceFiles;
	m_pfnItrTbls[3] = &CPdbListView::IterateTableLineNumbers;
	m_pfnItrTbls[4] = &CPdbListView::IterateTableSegments;
	m_pfnItrTbls[5] = &CPdbListView::IterateTableFrameData;

	m_pfnTypes.insert(std::make_pair(SymTagUDT,			 &CPdbListView::ParseUdtList));
	m_pfnTypes.insert(std::make_pair(SymTagEnum,		 &CPdbListView::ParseEnumList));
	m_pfnTypes.insert(std::make_pair(SymTagTypedef,		 &CPdbListView::ParseTypedDefList));
	m_pfnTypes.insert(std::make_pair(SymTagFunction,	 &CPdbListView::ParseFunctionList));
	m_pfnTypes.insert(std::make_pair(SymTagData,		 &CPdbListView::ParseDataList));
	m_pfnTypes.insert(std::make_pair(SymTagPublicSymbol, &CPdbListView::ParsePublicSymbolList));
	m_pfnTypes.insert(std::make_pair(SymTagCompiland,	 &CPdbListView::ParseCompliandList));

	m_dias.insert(std::make_pair(SymTagPublicSymbol, &CPdbListView::ParsePublics));
	m_dias.insert(std::make_pair(SymTagCompiland, &CPdbListView::ParseGlobals));
	m_dias.insert(std::make_pair(SymTagCompiland, &CPdbListView::ParselLines));
	m_dias.insert(std::make_pair(SymTagCompiland, &CPdbListView::ParseSecContribs));
	m_dias.insert(std::make_pair(SymTagCompiland, &CPdbListView::ParseDebugStreams));
}

CPdbListView::~CPdbListView()
{
}

BOOL CPdbListView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= (LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
	return CListView::PreCreateWindow(cs);
}

void CPdbListView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();
	CPdbDoc* pDoc = GetDocument();
	if (pDoc->DIA_PESCOPE == 0)
		return;

	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP;
	GetListCtrl().SetExtendedStyle(GetListCtrl().GetExtendedStyle() | dwExStyle);
	GetListCtrl().SetFont(((CPdbApp*)AfxGetApp())->GetFont());

	while (GetListCtrl().DeleteColumn(0));
	GetListCtrl().DeleteAllItems();
}

void CPdbListView::CleareListView(bool bDelCols)
{
	if (bDelCols)
		while (GetListCtrl().DeleteColumn(0));
	GetListCtrl().DeleteAllItems();
}

CString CPdbListView::SplitFilePath(PCWSTR pszName, CString* pszPath)
{
	CString sz = pszName;
	int nPos = sz.ReverseFind(L'\\');
	if (nPos > 0)
	{
		if (pszPath != NULL)
			*pszPath = sz.Left(nPos + 1);
		sz = (PCWSTR)sz + nPos + 1;
	}
	return sz;
}

bool CPdbListView::UpdateTableList(PDIA_NODE pdn)
{
	CleareListView();

	int nColCnt = PCNT_COLUMS[pdn->Category];
	for (int i = 0; i < nColCnt; i++)
		GetListCtrl().InsertColumn(i, PSZ_COLUMS[pdn->Category][i], LVCFMT_LEFT, 100);

	CComPtr<IDiaTable> pITable;
	if (pdn->IDiaPtr->QueryInterface(&pITable) != S_OK)
		return false;

	PFN_ITERTABLE pfn = m_pfnItrTbls[pdn->Category];
	(this->*pfn)(pITable);
	pITable = 0;
	return true;
}

bool CPdbListView::UpdateSymbolGroup(PDIA_NODE pdn)
{
	CComPtr<IDiaSymbol> pISymRoot;
	if (pdn->IDiaPtr->QueryInterface(&pISymRoot) != S_OK)
		return false;

	DIA_SYMTAGS symTag = (DIA_SYMTAGS)pdn->Category;
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (pISymRoot->findChildren(/*(symTag == SymTagTypedef) ? SymTagNull : */symTag, NULL, nsNone, &pIEnumSyms) == S_OK)
	{
		ULONG uCelt = 0, uSymId = 0;
		PFN_TYPE_MAP::iterator it = m_pfnTypes.find(symTag);
		if (it == m_pfnTypes.end())
		{
			pIEnumSyms = 0;
			return false;
		}

		GetListCtrl().InsertColumn(1, L"ID", LVCFMT_LEFT, 100);
		GetListCtrl().InsertColumn(1, L"Name", LVCFMT_LEFT, 100);
		int nColCnt = 6;
		PCWSTR* pcarCols = COL_SYMB_DATA;
		if (symTag == DIA_SYMTAGS::SymTagUDT)
			nColCnt = 4, pcarCols = COL_SYMB_UDT;
		else if (symTag == SymTagEnum || symTag == SymTagTypedef)
			nColCnt = 3, pcarCols = COL_SYMB_ENUMDEF;
		else if (symTag == DIA_SYMTAGS::SymTagFunction)
			nColCnt = 5, pcarCols = COL_SYMB_FUNC;
		else if (symTag == DIA_SYMTAGS::SymTagCompiland)
			nColCnt = 4, pcarCols = COL_SYMB_COMPLND;
		else if (symTag == DIA_SYMTAGS::SymTagPublicSymbol)
			nColCnt = 5, pcarCols = COL_SYMB_PUBSYMS;
		for (int i = 0; i < nColCnt; i++)
			GetListCtrl().InsertColumn(i + 2, pcarCols[i], LVCFMT_LEFT, 100);

		int nItemIdx = 0;
		CComPtr<IDiaSymbol> pISym;
		while (SUCCEEDED(pIEnumSyms->Next(1, &pISym, &uCelt)) && (uCelt == 1))
		{
			CComBSTR bszName;
			if (pISym->get_name(&bszName) != S_OK || bszName.Length() == 0)
				bszName = L"<no_name>";
			else
			{
				if (symTag == SymTagCompiland)
				{
					PWSTR pPos = _tcsrchr(bszName, L'\\');
					if (pPos != NULL)
						bszName = (PCWSTR)(pPos + 1);
				}
			}

			pISym->get_symIndexId(&uSymId);
			CString szId; szId.Format(L"%d", uSymId);
			int nLvItem = GetListCtrl().InsertItem(nItemIdx, szId);

			GetListCtrl().SetItem(nItemIdx, 1, TVIF_TEXT, bszName, 0, 0, 0, 0);
			PDIA_NODE pdn2 = new DIA_NODE(NODE_SYMBOL, pdn->Category, pISym);
			GetListCtrl().SetItemData(nLvItem, (DWORD_PTR)pdn2);
			(this->*it->second)(pISym, nItemIdx);
			pISym = 0; nItemIdx++;
		}
	}
	pIEnumSyms = 0;

	return true;
}

bool CPdbListView::UpdateSymbolList(PDIA_NODE pdn)
{
	CComPtr<IDiaSymbol> pISymRoot;
	if (pdn->IDiaPtr->QueryInterface(&pISymRoot) != S_OK)
		return false;

	GetListCtrl().InsertColumn(0, L"ID", LVCFMT_LEFT, 40);
	GetListCtrl().InsertColumn(1, L"SymTag", LVCFMT_LEFT, 100);
	GetListCtrl().InsertColumn(2, L"Name", LVCFMT_LEFT, 100);

	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	HRESULT hr = pISymRoot->findChildren(SymTagNull, NULL, nsNone, &pIEnumSyms);
	if (hr != S_OK)
		return false;

	int nItemIdx = 0;
	ULONG uCelt = 0, uSymId = 0;
	CComPtr<IDiaSymbol> pISymbol;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pISymbol, &uCelt)) && (uCelt == 1))
	{
		pISymbol->get_symIndexId(&uSymId);
		CString szId; szId.Format(L"%d", uSymId);
		int nLvItem = GetListCtrl().InsertItem(nItemIdx, szId);

		DIA_SYMTAGS eSymTag = DIA_SYMTAGS::SymTagNull;
		pISymbol->get_symTag((PDWORD)&eSymTag);
		GetListCtrl().SetItem(nLvItem, 1, TVIF_TEXT, PEPdb::GSZ_SYM_TAGS[eSymTag], 0, 0, 0, 0);

		CComBSTR bszName = L"";
		pISymbol->get_name(&bszName);
		PWSTR pPos = bszName;
		if (eSymTag == DIA_SYMTAGS::SymTagCompiland)
		{
			pPos = _tcsrchr(bszName, L'\\');
			if (pPos != NULL)
				pPos++;
			else
				pPos = bszName;
		}
		GetListCtrl().SetItem(nLvItem, 2, TVIF_TEXT, bszName, 0, 0, 0, 0);

		PDIA_NODE pdn2 = new DIA_NODE(NODE_SYMBOL, (WORD)eSymTag, pISymbol);
		GetListCtrl().SetItemData(nLvItem, (DWORD_PTR)pdn2);
		nItemIdx++;
		pISymbol = 0;
	}
	pIEnumSyms = 0;
	return true;
}

void CPdbListView::UpdateDIA(PDIA_NODE pdn)
{
	while (GetListCtrl().DeleteColumn(0));
	GetListCtrl().DeleteAllItems();

	switch (pdn->NodeType)
	{
		case NODE_TABLE : UpdateTableList(pdn); break;
		case NODE_GRPSYM: UpdateSymbolGroup(pdn); break;
		case NODE_SYMBOL: UpdateSymbolList(pdn); break;
	}
}


// CPdbListView 진단
#ifdef _DEBUG
void CPdbListView::AssertValid() const
{
	CListView::AssertValid();
}

void CPdbListView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CPdbDoc* CPdbListView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPdbDoc)));
	return (CPdbDoc*)m_pDocument;
}
#endif //_DEBUG

void CPdbListView::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}

void CPdbListView::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pnmlv = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pnmlv->lParam != NULL && pnmlv->uNewState)
	{
		CSplitterWnd* pPane = (CSplitterWnd*)GetParentSplitter(this, FALSE);
		CSplitterWnd* pPaneSub = (CSplitterWnd*)pPane->GetPane(1, 0);
		CPdbPropView* pProp = (CPdbPropView*)pPaneSub->GetPane(0, 0);
		CPdbTextView* pCode = (CPdbTextView*)pPaneSub->GetPane(0, 1);

		PDIA_NODE pdn = (PDIA_NODE)pnmlv->lParam;
		if (pdn->NodeType == NODE_SYMBOL)
		{
			pProp->UpdateDIAProps(pdn);
			pCode->UpdateCode(pdn);
		}
	}
	*pResult = 0;
}

void CPdbListView::OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pnmlv = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pnmlv->lParam != NULL)
		delete PDIA_NODE(pnmlv->lParam);
	*pResult = 0;
}
