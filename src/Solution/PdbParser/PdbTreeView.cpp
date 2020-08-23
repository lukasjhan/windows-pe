#include "stdafx.h"
#include "PdbParser.h"
#include "PdbDoc.h"
#include "PdbTreeView.h"
#include "PdbListView.h"
#include "PdbPropView.h"
#include "PdbTextView.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


// CPdbTreeView
IMPLEMENT_DYNCREATE(CPdbTreeView, CTreeView)
BEGIN_MESSAGE_MAP(CPdbTreeView, CTreeView)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, &CPdbTreeView::OnTvnSelchanged)
	ON_NOTIFY_REFLECT(TVN_DELETEITEM, &CPdbTreeView::OnTvnDeleteitem)
	ON_NOTIFY_REFLECT(TVN_ITEMEXPANDING, &CPdbTreeView::OnTvnItemexpanding)
END_MESSAGE_MAP()


// CPdbTreeView 생성/소멸
CPdbTreeView::CPdbTreeView()
{
}

CPdbTreeView::~CPdbTreeView()
{
}

HTREEITEM CPdbTreeView::AddItem(PCWSTR pszName, BOOL bHasChild, PDIA_NODE pParam, HTREEITEM hParent)
{
	DWORD dwMask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE | TVIF_CHILDREN | TVIF_HANDLE;
	TV_INSERTSTRUCT	tv;
	tv.hParent			= hParent;
	tv.hInsertAfter		= TVI_LAST;
	tv.item.mask		= dwMask;
	tv.item.state		= 0;
	tv.item.pszText		= (PWSTR)pszName;
	tv.item.cChildren	= bHasChild;
	tv.item.lParam		= (LPARAM)pParam;
	return GetTreeCtrl().InsertItem(&tv);
}

void CPdbTreeView::BuildGlobalNode(IDiaSymbol* pIBase, HTREEITEM htiUp)
{
	DIA_SYMTAGS symTags[] =
	{
		SymTagUDT, SymTagEnum, SymTagTypedef, SymTagFunction, SymTagData, SymTagPublicSymbol, SymTagCompiland, SymTagFunctionType
	};
	PCWSTR nodeNames[] =
	{
		L"UDTs", L"Enums", L"TypeDefs", L"Functions", L"GlobalData", L"PublicSymbols", L"Compilands", L"FunctionTypes"
	};

	for (int i = 0; i < sizeof(symTags) / sizeof(DIA_SYMTAGS); i++)
	{
		CComPtr<IDiaEnumSymbols> pIEnumSyms;
		HRESULT hr = pIBase->findChildren(symTags[i], NULL, nsNone, &pIEnumSyms);
		if (hr != S_OK)
			continue;

		LONG lCount = 0;
		if (pIEnumSyms->get_Count(&lCount) == S_OK && lCount > 0)
		{
			CString szName; szName.Format(L"%s(%d)", nodeNames[i], lCount);
			PDIA_NODE pdn = new DIA_NODE(NODE_GRPSYM, (WORD)symTags[i], pIBase);
			AddItem(szName, TRUE, pdn, htiUp);
		}
		pIEnumSyms = 0;
	}
}

HTREEITEM CPdbTreeView::BuildPEScopeNode(IDiaSymbol* pIPE)
{
	CString sz;

	CComBSTR bszName;
	pIPE->get_name(&bszName);
	sz.Format(L"GlobalSocpe: %s", bszName);
	HTREEITEM htiScope = GetTreeCtrl().InsertItem(sz);
	PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, (WORD)SymTagExe, pIPE);
	pdn->IsScaned = true;
	GetTreeCtrl().SetItemData(htiScope, DWORD_PTR(pdn));
	BuildGlobalNode(pIPE, htiScope);
	return htiScope;
}

int CPdbTreeView::BuildSymSubNode(IDiaSymbol* pISym, HTREEITEM htiUp, DIA_SYMTAGS eWantSym)
{
	int nSubCnt = 0;
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	HRESULT hr = pISym->findChildren(eWantSym, NULL, nsNone, &pIEnumSyms);
	if (hr == S_OK)
	{
		ULONG uCelt = 0;
		CComPtr<IDiaSymbol> pISubSym;
		while (SUCCEEDED(pIEnumSyms->Next(1, &pISubSym, &uCelt)) && (uCelt == 1))
		{
			DIA_SYMTAGS eSymTag = DIA_SYMTAGS::SymTagNull;
			pISubSym->get_symTag((PDWORD)&eSymTag);
			CComBSTR bszName = L"";
			pISubSym->get_name(&bszName);

			CString szName = bszName;
			if (eSymTag == DIA_SYMTAGS::SymTagCompiland)
			{
				PWSTR pPos = _tcsrchr(bszName, L'\\');
				if (pPos != NULL)
					szName = pPos + 1;
			}
			else if (eSymTag == DIA_SYMTAGS::SymTagUDT)
			{
				BOOL bCheck;
				if (eWantSym == DIA_SYMTAGS::SymTagUDT && pISubSym->get_nested(&bCheck) == S_OK && bCheck)
				{
					DWORD dwUpId, dwThId;
					pISym->get_symIndexId(&dwUpId);
					pISubSym->get_symIndexId(&dwThId);
					if (dwUpId == dwThId)
					{
						HTREEITEM hIter = GetTreeCtrl().GetChildItem(htiUp);
						while (hIter)
						{
							HTREEITEM hNext = GetTreeCtrl().GetNextSiblingItem(hIter);
							GetTreeCtrl().DeleteItem(hIter);
							hIter = hNext;
						}
						pISubSym = 0;
						return nSubCnt;
					}
				}
				UdtKind uk;
				if (pISubSym->get_udtKind((PDWORD)&uk) == S_OK)
					szName.Format(L"(%s)%s", PEPdb::GSZ_UDT_KINDS[uk], bszName);
			}

			if (eWantSym == DIA_SYMTAGS::SymTagNull)
			{
				CString sz; sz.Format(L"[%s]", PEPdb::GSZ_SYM_TAGS[eSymTag]);
				szName.Insert(0, sz);
			}

			BOOL bHasSub = FALSE;
			CComPtr<IDiaEnumSymbols> pIEnumSubs;
			if (pISubSym->findChildren(SymTagNull, NULL, nsNone, &pIEnumSubs) == S_OK)
			{
				LONG lCount = 0;
				if (pIEnumSubs->get_Count(&lCount) == S_OK && lCount > 0)
				{
					CComPtr<IDiaSymbol> pISubSym2;
					if (SUCCEEDED(pIEnumSubs->Next(1, &pISubSym2, &uCelt)) && (uCelt == 1))
					{
						bHasSub = TRUE;
						pISubSym2 = 0;
					}
				}
				pIEnumSubs = 0;
			}
			PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, eSymTag, pISubSym);
			AddItem(szName, bHasSub, pdn, htiUp);
			//HTREEITEM htiSub = GetTreeCtrl().InsertItem(szName, htiUp);
			//GetTreeCtrl().SetItemData(htiSub, DWORD_PTR(pdn));
			//if (eSymTag != DIA_SYMTAGS::SymTagTypedef && eSymTag != DIA_SYMTAGS::SymTagBaseClass)
			//	BuildSymSubNode(pISubSym, htiSub, eSymTag);
			pISubSym = 0;
			nSubCnt++;
		}
		pIEnumSyms = 0;
	}
	return nSubCnt;
}

HTREEITEM CPdbTreeView::BuildTableNode(IDiaSession* pISess)
{
	static PWSTR pszNodes[] =
	{
		DiaTable_Symbols,  DiaTable_Sections, DiaTable_SrcFiles, 
		DiaTable_LineNums, DiaTable_SegMap,   DiaTable_FrameData
	};

	HRESULT hr = S_OK;
	HTREEITEM htiTables = GetTreeCtrl().InsertItem(L"Tables");
	PDIA_NODE pdt = new DIA_NODE(NODE_GRPTBL, 0);
	pdt->IsScaned = TRUE;
	GetTreeCtrl().SetItemData(htiTables, DWORD_PTR(pdt));

	CComPtr<IDiaEnumTables> pIEnumTbls;
	if (SUCCEEDED(pISess->getEnumTables(&pIEnumTbls)))
	{
		ULONG ulCelt = 0;
		CComPtr<IDiaTable> pITable;
		while (SUCCEEDED(hr = pIEnumTbls->Next(1, &pITable, &ulCelt)) && ulCelt == 1)
		{
			CComBSTR bszTblName;
			if (pITable->get_name(&bszTblName) == S_OK)
			{
				pdt = NULL;
				for (int i = 0; i < sizeof(pszNodes) / sizeof(PWSTR); i++)
				{
					if (wcsicmp(bszTblName, pszNodes[i]) == 0)
					{
						pdt = new DIA_NODE(NODE_TABLE, i, pITable);
						break;
					}
				}
				if (pdt != NULL)
				{
					HTREEITEM htiTbl = GetTreeCtrl().InsertItem(bszTblName, htiTables);
					pdt->IsScaned = TRUE;
					GetTreeCtrl().SetItemData(htiTbl, DWORD_PTR(pdt));
				}
			}
			pITable = 0;
		}
	}
	return htiTables;
}

typedef std::map<DIA_SYMTAGS, HTREEITEM> TAG_MAP;
int CPdbTreeView::BuildSymbolNode(IDiaSymbol* pISym, HTREEITEM htiUp)
{
	DIA_SYMTAGS eSymTag = DIA_SYMTAGS::SymTagNull;
	pISym->get_symTag((PDWORD)&eSymTag);
	CComBSTR bszName = L"";
	pISym->get_name(&bszName);
	PWSTR pPos = bszName;
	if (eSymTag == DIA_SYMTAGS::SymTagCompiland)
	{
		pPos = _tcsrchr(bszName, L'\\');
		if (pPos != NULL)
			pPos++;
		else
			pPos = bszName;
	}
	CString szName;
	szName.Format(L"%s:%s", PEPdb::GSZ_SYM_TAGS[eSymTag], pPos);
	HTREEITEM htiSub = GetTreeCtrl().InsertItem(szName, htiUp);
	//DIA_NODE<IDiaSymbol>* pdn = new DIA_NODE<IDiaSymbol>(DIA_CAT_SYMBOL, eSymTag, pISym);
	//GetTreeCtrl().SetItemData(htiSub, DWORD_PTR(pdn));

	TAG_MAP mapGrp;
	int nSubCnt = 0;
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	HRESULT hr = pISym->findChildren(DIA_SYMTAGS::SymTagNull, NULL, nsNone, &pIEnumSyms);
	if (hr == S_OK)
	{
		CComPtr<IDiaSymbol> pISubSym;
		ULONG uCelt = 0, uModIdx = 1;
		while (SUCCEEDED(pIEnumSyms->Next(1, &pISubSym, &uCelt)) && (uCelt == 1))
		{
			HTREEITEM hParent = htiSub;
			if (htiUp == TVI_ROOT)
			{
				eSymTag = DIA_SYMTAGS::SymTagNull;
				pISubSym->get_symTag((PDWORD)&eSymTag);
				TAG_MAP::iterator it = mapGrp.find(eSymTag);
				if (it == mapGrp.end())
				{
					hParent = GetTreeCtrl().InsertItem(PEPdb::GSZ_SYM_TAGS[eSymTag], htiSub);
					//DIA_NODE<IDiaSymbol>* pdn = new DIA_NODE<IDiaSymbol>(DIA_CAT_SYMBOL, eSymTag, pISym);
					mapGrp.insert(std::make_pair(eSymTag, hParent));
				}
				else
					hParent = it->second;
			}
			BuildSymbolNode(pISubSym, hParent);
			pISubSym = 0;
			nSubCnt++;
		}
		pIEnumSyms = 0;
	}
	return nSubCnt;

}



BOOL CPdbTreeView::PreCreateWindow(CREATESTRUCT& cs)
{
	return CTreeView::PreCreateWindow(cs);
}

void CPdbTreeView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();
	CPdbDoc* pDoc = GetDocument();
	if (pDoc->DIA_PESCOPE == 0)
		return;

	GetTreeCtrl().ModifyStyle(NULL, TVS_HASBUTTONS | TVS_HASLINES | 
		TVS_FULLROWSELECT | TVS_LINESATROOT | TVS_SHOWSELALWAYS);
	GetTreeCtrl().SetFont(((CPdbApp*)AfxGetApp())->GetFont());
	GetTreeCtrl().DeleteAllItems();

	HTREEITEM htiScope = BuildPEScopeNode(pDoc->DIA_PESCOPE);
	GetTreeCtrl().Expand(htiScope, TVE_EXPAND);
	HTREEITEM htiTables = BuildTableNode(pDoc->DIA_SESSION);
	GetTreeCtrl().Expand(htiTables, TVE_EXPAND);
}

// CPdbTreeView 진단
#ifdef _DEBUG
void CPdbTreeView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CPdbTreeView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CPdbDoc* CPdbTreeView::GetDocument()
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPdbDoc)));
	return (CPdbDoc*)m_pDocument;
}
#endif //_DEBUG


// CPdbTreeView 메시지 처리기
void CPdbTreeView::OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pnmtv = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	if (pnmtv->itemNew.lParam != 0)
	{
		CSplitterWnd* pPane = (CSplitterWnd*)GetParentSplitter(this, FALSE)->GetPane(0, 1);
		CPdbListView* pDetail = (CPdbListView*)pPane->GetPane(0, 0);
		CSplitterWnd* pPane2 = (CSplitterWnd*)pPane->GetPane(1, 0);
		CPdbPropView* pProp = (CPdbPropView*)pPane2->GetPane(0, 0);

		PDIA_NODE pdn = (PDIA_NODE)pnmtv->itemNew.lParam;
		BeginWaitCursor();
		pProp->UpdateDIAProps((pdn->NodeType == NODE_SYMBOL) ? pdn : NULL);
		pDetail->UpdateDIA(pdn);
		EndWaitCursor();
	}
	*pResult = 0;
}

void CPdbTreeView::OnTvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pnmtv = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	if (pnmtv->itemOld.lParam != 0)
	{
		PDIA_NODE pdn = (PDIA_NODE)pnmtv->itemOld.lParam;
		delete pdn;
	}
	*pResult = 0;
}

void CPdbTreeView::OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pnmtv = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	*pResult = 0;

	if ((pnmtv->action & TVE_EXPAND) != TVE_EXPAND)
		return;
	if (pnmtv->itemNew.lParam == NULL)
		return;

	PDIA_NODE pdn = (PDIA_NODE)pnmtv->itemNew.lParam;
	if (pdn->IsScaned || 
		(pdn->Category != SymTagExe && pdn->NodeType <= NODE_TABLE))
		return;

	CComPtr<IDiaSymbol> pIBase;
	if (pdn->IDiaPtr->QueryInterface(&pIBase) != S_OK)
		return;
	DIA_SYMTAGS eWantSym = DIA_SYMTAGS::SymTagNull;
	if (pdn->NodeType == NODE_GRPSYM)
		eWantSym = (DIA_SYMTAGS)pdn->Category;
	BuildSymSubNode(pIBase, pnmtv->itemNew.hItem, eWantSym);
	pdn->IsScaned = true;
	pIBase = 0;
}
