#include "stdafx.h"
#	ifndef SHARED_HANDLERS
#include "PdbParser.h"
#endif
#include "PdbDoc.h"
#include "PdbPropView.h"
#include "PdbTextView.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif



// CPdbPropView
IMPLEMENT_DYNCREATE(CPdbPropView, CListView)
BEGIN_MESSAGE_MAP(CPdbPropView, CListView)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CPdbPropView::OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, &CPdbPropView::OnLvnItemchanged)
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, &CPdbPropView::OnLvnDeleteitem)
END_MESSAGE_MAP()

// CPdbPropView 생성/소멸
CPdbPropView::CPdbPropView()
{
	int i = 0;
	memset(m_pfnSymTags, 0, sizeof(PFN_PROP_SYMTAG) * DIA_SYMTAGS::SymTagMax);
	m_pfnSymTags[i++] = &CPdbPropView::PropViewNull;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewExe;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewCompiland;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewCompilandDetails;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewCompilandEnv;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewAll; // PropViewFunction;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewBlockAnnote;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewData;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewBlockAnnote;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewLabel;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewPublicSymbol;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewUDT;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewEnum;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewFunctionType;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewPointerType;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewArrayType;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewBaseType;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewTypedef;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewBaseClass;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewFriend;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewFunctionArgType;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewFuncDebugStartEnd;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewFuncDebugStartEnd;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewUsingNamespace;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewVTableShape;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewVTable;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewCustom;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewThunk;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewCustomType;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewManagedType;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewDimension;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewCallSite;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewInlineSite;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewBaseInterface;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewVectorType;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewMatrixType;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewHLSLType;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewAll;
	m_pfnSymTags[i++] = &CPdbPropView::PropViewAll;
}

CPdbPropView::~CPdbPropView()
{
}

BOOL CPdbPropView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= (LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS);
	return CListView::PreCreateWindow(cs);
}

void CPdbPropView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();
	CPdbDoc* pDoc = GetDocument();
	if (pDoc->DIA_PESCOPE == 0)
		return;

	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP;
	GetListCtrl().SetExtendedStyle(GetListCtrl().GetExtendedStyle() | dwExStyle);
	GetListCtrl().SetFont(((CPdbApp*)AfxGetApp())->GetFont());

	GetListCtrl().InsertColumn(0, L"Property", LVCFMT_LEFT, 150);
	GetListCtrl().InsertColumn(1, L"Value", LVCFMT_LEFT, 350);
}

bool CPdbPropView::UpdateDIAProps(PDIA_NODE pdn)
{
	if (pdn != NULL)
	{
		CComPtr<IDiaSymbol> pISymbol;
		if (pdn->IDiaPtr->QueryInterface(&pISymbol) != S_OK)
			return false;

		GetListCtrl().DeleteAllItems();
		PropViewAll(pISymbol);

		//DIA_SYMTAGS tag = (DIA_SYMTAGS)pdn->Category;
		//PFN_PROP_SYMTAG pfnProp = m_pfnSymTags[tag];
		//(this->*pfnProp)(pISymbol);
		pISymbol = 0;
	}
	return true;
}


// CPdbPropView 진단
#ifdef _DEBUG
void CPdbPropView::AssertValid() const
{
	CListView::AssertValid();
}

void CPdbPropView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CPdbDoc* CPdbPropView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPdbDoc)));
	return (CPdbDoc*)m_pDocument;
}
#endif //_DEBUG

void CPdbPropView::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	LPNMITEMACTIVATE pia = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint point(pia->ptAction);
	int nItem = GetListCtrl().HitTest(point);
	if (nItem < 0)
		return;

	PDIA_NODE pdn = (PDIA_NODE)GetListCtrl().GetItemData(nItem);
	if (pdn == NULL)
		return;
	UpdateDIAProps(pdn);
}

void CPdbPropView::OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pnmlv = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pnmlv->lParam != NULL && pnmlv->uNewState)
	{
		//CSplitterWnd* pPane = (CSplitterWnd*)GetParentSplitter(this, FALSE);
		//CPdbTextView* pCode = (CPdbTextView*)pPane->GetPane(1, 0);
		//PUSHORT pCat = PUSHORT(pnmlv->lParam);
		//if (*pCat == DIA_CAT_SYMBOL)
		//	pCode->UpdateCode((DIA_NODE<IDiaSymbol>*)pnmlv->lParam);
	}
	*pResult = 0;
}

void CPdbPropView::OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pnmlv = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pnmlv->lParam != NULL)
		delete (PDIA_NODE)pnmlv->lParam;
	*pResult = 0;
}




void CPdbPropView::PropViewNull(IDiaSymbol* pISym)
{

}

void CPdbPropView::PropViewExe(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CComBSTR bszRet;
	BOOL fRet;
	GUID uidRet;
	CString sz; int nItem = 0;

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_age(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Age");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_guid(&uidRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Guid");
		WCHAR wszGuid[65];
		StringFromGUID2(uidRet, wszGuid, 64);
		sz = wszGuid;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_signature(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Signature");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_machineType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"MachineType");
		CV_CPU_TYPE_e eCpu = (CV_CPU_TYPE_e)-1;
		switch (dwRet)
		{
			case IMAGE_FILE_MACHINE_I386 : eCpu = CV_CFL_80386; break;
			case IMAGE_FILE_MACHINE_IA64 : eCpu = CV_CFL_IA64; break;
			case IMAGE_FILE_MACHINE_AMD64: eCpu = CV_CFL_AMD64; break;
		}
		if ((int)eCpu < 0)
			sz.Format(L"0x%08X", dwRet);
		else
			sz.Format(L"0x%X(%s)", dwRet, PEPdb::GSZ_CPU_TYPES[eCpu]);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symbolsFileName(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymbolsFileName");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isCTypes(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsCTypes");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isStripped(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsStripped");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewCompiland(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CComBSTR bszRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_libraryName(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LibraryName");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_sourceFileName(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SourceFileName");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_editAndContinueEnabled(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"EditAndContinueEnabled");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewCompilandDetails(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CComBSTR bszRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_compilerName(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"CompilerName");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_language(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Language");
		sz.Format(L"%s (%d)", PEPdb::GSZ_LANG_SPECS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_platform(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Platform");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_backEndMajor(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BackEndMajor");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_backEndMinor(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BackEndMinor");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_backEndBuild(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BackEndBuild");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_frontEndMajor(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"FrontEndMajor");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_frontEndMinor(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"FrontEndMinor");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_frontEndBuild(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"FrontEndBuild");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_editAndContinueEnabled(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"EditAndContinueEnabled");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasDebugInfo(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasDebugInfo");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasManagedCode(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasManagedCode");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasSecurityChecks(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasSecurityChecks");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isCVTCIL(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsCVTCIL");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isDataAligned(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsDataAligned");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isHotpatchable(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsHotpatchable");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isLTCG(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsLTCG");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isMSILNetmodule(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsMSILNetmodule");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewCompilandEnv(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CComBSTR bszRet;
	CComVariant vtRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_value(&vtRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Value");
		sz = PEPdb::ToStr_Variant(&vtRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewFunction(IDiaSymbol* pISym)
{
	DWORD dwRet;
	ULONGLONG llRet;
	CComBSTR bszRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_locationType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LocationType");
		sz.Format(L"%s (%d)", PEPdb::GSZ_LOC_TYPES[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_relativeVirtualAddress(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"RelativeVirtualAddress");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualAddress(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualAddress");
		sz.Format(L"0x%016I64X", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressSection(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressSection");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressOffset");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_length(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Length");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_undecoratedName(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UndecoratedName");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ConstType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_volatileType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VolatileType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isStatic(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsStatic");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_customCallingConvention(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"CustomCallingConvention");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_farReturn(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"FarReturn");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasAlloca(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasAlloca");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasEH(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasEH");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasEHa(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasEHa");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasInlAsm(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasInlAsm");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasLongJump(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasLongJump");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasSecurityChecks(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasSecurityChecks");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasSEH(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasSEH");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasSetJump(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasSetJump");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_interruptReturn(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"InterruptReturn");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_intro(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Intro");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_inlSpec(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"InlSpec");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isNaked(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsNaked");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_noInline(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NoInline");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_notReached(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NotReached");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_noReturn(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NoReturn");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_noStackOrdering(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NoStackOrdering");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_optimizedCodeDebugInfo(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"OptimizedCodeDebugInfo");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unalignedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnalignedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_access(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Access");
		sz.Format(L"%s (%d)", PEPdb::GSZ_ACCESS_SPECS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_pure(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Pure");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtual(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Virtual");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualBaseOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualBaseOffset");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_token(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Token");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewBlockAnnote(IDiaSymbol* pISym)
{
	DWORD dwRet;
	ULONGLONG llRet;
	CComBSTR bszRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_locationType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LocationType");
		sz.Format(L"%s (%d)", PEPdb::GSZ_LOC_TYPES[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_relativeVirtualAddress(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"RelativeVirtualAddress");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualAddress(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualAddress");
		sz.Format(L"0x%016I64X", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressSection(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressSection");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressOffset");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_length(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Length");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewData(IDiaSymbol* pISym)
{
	DWORD dwRet;
	LONG lRet;
	ULONGLONG llRet;
	CComBSTR bszRet;
	BOOL fRet;
	CComVariant vtRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, (WORD)tag, pIRet);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		GetListCtrl().SetItemData(nItem, DWORD_PTR(pdn));
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_locationType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LocationType");
		sz.Format(L"%s (%d)", PEPdb::GSZ_LOC_TYPES[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_dataKind(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"DataKind");
		sz.Format(L"%s (%d)", PEPdb::GSZ_DATA_KINDS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualAddress(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualAddress");
		sz.Format(L"%016I64X", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_relativeVirtualAddress(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"RelativeVirtualAddress");
		sz.Format(L"%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressSection(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressSection");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressOffset");
		sz.Format(L"%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_registerId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"RegisterId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_offset(&lRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Offset");
		sz.Format(L"%d", lRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_bitPosition(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BitPosition");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_length(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Length");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_value(&vtRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Value");
		sz = PEPdb::ToStr_Variant(&vtRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_access(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Access");
		sz.Format(L"%s (%d)", PEPdb::GSZ_ACCESS_SPECS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressTaken(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressTaken");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_compilerGenerated(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"CompilerGenerated");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ConstType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isAggregated(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsAggregated");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isSplitted(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsSplitted");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unalignedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnalignedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_volatileType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VolatileType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_slot(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Slot");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_token(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Token");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewLabel(IDiaSymbol* pISym)
{
	DWORD dwRet;
	ULONGLONG llRet;
	CComBSTR bszRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_locationType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LocationType");
		sz.Format(L"%s (%d)", PEPdb::GSZ_LOC_TYPES[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_relativeVirtualAddress(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"RelativeVirtualAddress");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualAddress(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualAddress");
		sz.Format(L"0x%016I64X", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressSection(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressSection");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressOffset");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_customCallingConvention(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"CustomCallingConvention");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_farReturn(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"FarReturn");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_interruptReturn(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"InterruptReturn");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_noInline(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NoInline");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_notReached(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NotReached");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_noReturn(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NoReturn");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_optimizedCodeDebugInfo(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"OptimizedCodeDebugInfo");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewPublicSymbol(IDiaSymbol* pISym)
{
	DWORD dwRet;
	ULONGLONG llRet;
	CComBSTR bszRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_locationType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LocationType");
		sz.Format(L"%s (%d)", PEPdb::GSZ_LOC_TYPES[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_undecoratedName(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UndecoratedName");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_relativeVirtualAddress(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"RelativeVirtualAddress");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressSection(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressSection");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressOffset");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_length(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Length");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_code(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Code");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_function(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Function");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewUDT(IDiaSymbol* pISym)
{
	DWORD dwRet;
	ULONGLONG llRet;
	CComBSTR bszRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_length(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Length");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_udtKind(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UdtKind");
		sz.Format(L"%s (%d)", PEPdb::GSZ_UDT_KINDS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualTableShape(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualTableShape");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], (tag == SymTagVTableShape) ?
			PEPdb::ToStr_VTableSahpe(pIRet) : PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualTableShapeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualTableShapeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constructor(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Constructor");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ConstType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasAssignmentOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasAssignmentOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasCastOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasCastOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasNestedTypes(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasNestedTypes");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_nested(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Nested");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_overloadedOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"OverloadedOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_packed(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Packed");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_scoped(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Scoped");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unalignedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnalignedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_volatileType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VolatileType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewEnum(IDiaSymbol* pISym)
{
	DWORD dwRet;
	ULONGLONG llRet;
	CComBSTR bszRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_length(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Length");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_baseType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BaseType");
		sz.Format(L"%s (%d)", PEPdb::GSZ_BASE_TYPES[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constructor(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Constructor");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ConstType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasAssignmentOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasAssignmentOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasCastOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasCastOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasNestedTypes(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasNestedTypes");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_nested(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Nested");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_overloadedOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"OverloadedOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_packed(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Packed");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_scoped(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Scoped");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unalignedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnalignedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_volatileType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VolatileType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewFunctionType(IDiaSymbol* pISym)
{
	DWORD dwRet;
	LONG lRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_count(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Count");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_callingConvention(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"CallingConvention");
		sz.Format(L"%s (%d)", PEPdb::GSZ_CALL_CONVS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_objectPointerType(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ObjectPointerType");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_thisAdjust(&lRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ThisAdjust");
		sz.Format(L"%d", lRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ConstType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unalignedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnalignedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_volatileType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VolatileType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewPointerType(IDiaSymbol* pISym)
{
	DWORD dwRet;
	ULONGLONG llRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_length(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Length");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ConstType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_reference(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Reference");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unalignedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnalignedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_volatileType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VolatileType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewArrayType(IDiaSymbol* pISym)
{
	DWORD dwRet;
	ULONGLONG llRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_length(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Length");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_arrayIndexType(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ArrayIndexType");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_arrayIndexTypeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ArrayIndexTypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_count(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Count");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_rank(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Rank");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ConstType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unalignedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnalignedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_volatileType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VolatileType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewBaseType(IDiaSymbol* pISym)
{
	DWORD dwRet;
	ULONGLONG llRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_length(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Length");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_baseType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BaseType");
		sz.Format(L"%s (%d)", PEPdb::GSZ_BASE_TYPES[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ConstType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unalignedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnalignedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_volatileType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VolatileType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewTypedef(IDiaSymbol* pISym)
{
	DWORD dwRet;
	ULONGLONG llRet;
	CComBSTR bszRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_length(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Length");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_baseType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BaseType");
		sz.Format(L"%s (%d)", PEPdb::GSZ_BASE_TYPES[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_udtKind(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UdtKind");
		sz.Format(L"%s (%d)", PEPdb::GSZ_UDT_KINDS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualTableShape(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualTableShape");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualTableShapeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualTableShapeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unalignedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnalignedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constructor(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Constructor");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ConstType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasAssignmentOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasAssignmentOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasCastOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasCastOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasNestedTypes(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasNestedTypes");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_nested(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Nested");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_overloadedOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"OverloadedOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_packed(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Packed");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_reference(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Reference");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_scoped(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Scoped");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_volatileType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VolatileType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewBaseClass(IDiaSymbol* pISym)
{
	DWORD dwRet;
	LONG lRet;
	ULONGLONG llRet;
	CComBSTR bszRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_length(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Length");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_access(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Access");
		sz.Format(L"%s (%d)", PEPdb::GSZ_ACCESS_SPECS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constructor(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Constructor");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ConstType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasAssignmentOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasAssignmentOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasCastOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasCastOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasNestedTypes(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasNestedTypes");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_indirectVirtualBaseClass(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IndirectVirtualBaseClass");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_nested(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Nested");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_offset(&lRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Offset");
		sz.Format(L"%d", lRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_overloadedOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"OverloadedOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_packed(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Packed");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_scoped(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Scoped");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_udtKind(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UdtKind");
		sz.Format(L"%s (%d)", PEPdb::GSZ_UDT_KINDS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unalignedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnalignedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualBaseClass(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualBaseClass");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualBaseDispIndex(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualBaseDispIndex");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualBasePointerOffset(&lRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualBasePointerOffset");
		sz.Format(L"%d", lRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualBaseTableType(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualBaseTableType");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualTableShape(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualTableShape");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualTableShapeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualTableShapeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_volatileType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VolatileType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewFriend(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CComBSTR bszRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewFunctionArgType(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewFuncDebugStartEnd(IDiaSymbol* pISym)
{
	DWORD dwRet;
	ULONGLONG llRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_locationType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LocationType");
		sz.Format(L"%s (%d)", PEPdb::GSZ_LOC_TYPES[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_relativeVirtualAddress(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"RelativeVirtualAddress");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualAddress(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualAddress");
		sz.Format(L"0x%016I64X", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressSection(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressSection");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressOffset");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_customCallingConvention(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"CustomCallingConvention");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_farReturn(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"FarReturn");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_interruptReturn(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"InterruptReturn");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isStatic(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsStatic");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_noInline(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NoInline");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_notReached(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NotReached");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_noReturn(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NoReturn");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_optimizedCodeDebugInfo(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"OptimizedCodeDebugInfo");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewUsingNamespace(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CComBSTR bszRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewVTableShape(IDiaSymbol* pISym)
{
	DWORD dwRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_count(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Count");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ConstType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unalignedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnalignedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_volatileType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VolatileType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewVTable(IDiaSymbol* pISym)
{
	DWORD dwRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ConstType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unalignedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnalignedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_volatileType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VolatileType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewCustom(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CComBSTR bszRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	DWORD dwSize = 0;
	HRESULT hr = pISym->get_dataBytes(0, &dwSize, NULL);
	if (hr == S_OK)
	{
		PBYTE pData = new BYTE[dwSize];
		hr = pISym->get_dataBytes(dwSize, &dwSize, pData);
		if (hr == S_OK)
		{
			sz.Format(L"Size=%d, Data=", dwSize);
			int nSize = (dwSize > 10) ? 10 : (int)dwSize;
			for (int i = 0; i < nSize; i++)
				sz.AppendFormat(L" %02X", pData[i]);
			if (nSize < (int)dwSize)
				sz.Append(L"...");
			GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
			nItem++;
		}
		delete[] pData;
	}
}

void CPdbPropView::PropViewThunk(IDiaSymbol* pISym)
{
	DWORD dwRet;
	ULONGLONG llRet;
	CComBSTR bszRet;
	BOOL fRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_locationType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LocationType");
		sz.Format(L"%s (%d)", PEPdb::GSZ_LOC_TYPES[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_thunkOrdinal(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ThunkOrdinal");
		sz.Format(L"%s (%d)", PEPdb::GSZ_THUNK_ORDS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_relativeVirtualAddress(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"RelativeVirtualAddress");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualAddress(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualAddress");
		sz.Format(L"0x%016I64X", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressSection(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressSection");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressOffset");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_length(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Length");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_targetRelativeVirtualAddress(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TargetRelativeVirtualAddress");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_targetVirtualAddress(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TargetVirtualAddress");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_targetSection(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TargetSection");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_targetOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TargetOffset");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ConstType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_volatileType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VolatileType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isStatic(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsStatic");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unalignedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnalignedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_access(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Access");
		sz.Format(L"%s (%d)", PEPdb::GSZ_ACCESS_SPECS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_pure(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Pure");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_intro(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Intro");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtual(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Virtual");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualBaseOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualBaseOffset");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewCustomType(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_oemId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"OemId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_oemSymbolId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"OemSymbolId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

}

void CPdbPropView::PropViewManagedType(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CComBSTR bszRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewDimension(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lowerBound(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LowerBound");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lowerBoundId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LowerBoundId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_upperBound(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UpperBound");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_upperBoundId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UpperBoundId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewCallSite(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CComBSTR bszRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		pIRet = 0;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_relativeVirtualAddress(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"RelativeVirtualAddress");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressSection(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressSection");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressOffset");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewInlineSite(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewBaseInterface(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewVectorType(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewMatrixType(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewHLSLType(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewCaller(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewCallee(IDiaSymbol* pISym)
{
	DWORD dwRet;
	CString sz; int nItem = 0;

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}

void CPdbPropView::PropViewAll(IDiaSymbol* pISym)
{
	DWORD dwRet;
	LONG lRet;
	ULONGLONG llRet;
	CComBSTR bszRet;
	BOOL fRet;
	GUID uidRet;
	CComVariant vtRet;
	CComPtr<IDiaSymbol> pIRet;
	CString sz; int nItem = 0; PDIA_NODE pdn = NULL;

	if (pISym->get_symIndexId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymIndexId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symTag(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymTag");
		sz.Format(L"%s (%d)", PEPdb::GSZ_SYM_TAGS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lexicalParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		CComBSTR bszName = L"";
		pIRet->get_name(&bszName);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], bszName);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);

		PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, (WORD)tag, pIRet);
		GetListCtrl().SetItemData(nItem, DWORD_PTR(pdn));
		pIRet = 0;
		nItem++;
	}

	if (pISym->get_lexicalParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LexicalParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_access(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Access");
		sz.Format(L"%s (%d)", PEPdb::GSZ_ACCESS_SPECS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressOffset");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressSection(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressSection");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_addressTaken(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"AddressTaken");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_age(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Age");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_arrayIndexType(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ArrayIndexType");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);

		PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, (WORD)tag, pIRet);
		GetListCtrl().SetItemData(nItem, DWORD_PTR(pdn));
		pIRet = 0;
		nItem++;
	}

	if (pISym->get_arrayIndexTypeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ArrayIndexTypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_backEndMajor(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BackEndMajor");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_backEndMinor(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BackEndMinor");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_backEndBuild(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BackEndBuild");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_baseDataOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BaseDataOffset");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_baseDataSlot(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BaseDataSlot");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_baseSymbol(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BaseSymbol");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);

		PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, (WORD)tag, pIRet);
		GetListCtrl().SetItemData(nItem, DWORD_PTR(pdn));
		pIRet = 0;
		nItem++;
	}

	if (pISym->get_baseSymbolId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BaseSymbolId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_baseType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BaseType");
		sz.Format(L"%s (%d)", PEPdb::GSZ_BASE_TYPES[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_bitPosition(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BitPosition");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_builtInKind(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"BuiltInKind");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_callingConvention(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"CallingConvention");
		sz.Format(L"%s (%d)", PEPdb::GSZ_CALL_CONVS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_classParent(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParent");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);

		PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, (WORD)tag, pIRet);
		GetListCtrl().SetItemData(nItem, DWORD_PTR(pdn));
		pIRet = 0;
		nItem++;
	}

	if (pISym->get_classParentId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ClassParentId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_code(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Code");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_compilerGenerated(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"CompilerGenerated");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_compilerName(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"CompilerName");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_constructor(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Constructor");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_container(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Container");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);

		PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, (WORD)tag, pIRet);
		GetListCtrl().SetItemData(nItem, DWORD_PTR(pdn));
		pIRet = 0;
		nItem++;
	}

	if (pISym->get_constType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ConstType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_count(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Count");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_countLiveRanges(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"CountLiveRanges");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_customCallingConvention(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"CustomCallingConvention");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_dataKind(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"DataKind");
		sz.Format(L"%s (%d)", PEPdb::GSZ_DATA_KINDS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_editAndContinueEnabled(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"EditAndContinueEnabled");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_farReturn(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"FarReturn");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_frontEndMajor(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"FrontEndMajor");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_frontEndMinor(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"FrontEndMinor");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_frontEndBuild(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"FrontEndBuild");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_function(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Function");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_guid(&uidRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Guid");
		WCHAR wszGuid[65];
		StringFromGUID2(uidRet, wszGuid, 64);
		sz = wszGuid;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasAlloca(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasAlloca");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasAssignmentOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasAssignmentOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasCastOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasCastOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasDebugInfo(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasDebugInfo");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasEH(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasEH");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasEHa(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasEHa");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasInlAsm(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasInlAsm");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasLongJump(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasLongJump");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasManagedCode(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasManagedCode");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasNestedTypes(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasNestedTypes");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasSecurityChecks(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasSecurityChecks");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasSEH(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasSEH");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_hasSetJump(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"HasSetJump");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_indirectVirtualBaseClass(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IndirectVirtualBaseClass");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_inlSpec(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"InlSpec");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_interruptReturn(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"InterruptReturn");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_intro(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Intro");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isAcceleratorGroupSharedLocal(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsAcceleratorGroupSharedLocal");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isAcceleratorPointerTagLiveRange(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsAcceleratorPointerTagLiveRange");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isAcceleratorStubFunction(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsAcceleratorStubFunction");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isAggregated(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsAggregated");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isCTypes(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsCTypes");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isCVTCIL(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsCVTCIL");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isDataAligned(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsDataAligned");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isHLSLData(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsHLSLData");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isHotpatchable(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsHotpatchable");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isLTCG(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsLTCG");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isMatrixRowMajor(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsMatrixRowMajor");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isMSILNetmodule(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsMSILNetmodule");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isMultipleInheritance(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsMultipleInheritance");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isNaked(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsNaked");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isOptimizedAway(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsOptimizedAway");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isPointerBasedOnSymbolValue(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsPointerBasedOnSymbolValue");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isPointerToDataMember(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsPointerToDataMember");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isPointerToMemberFunction(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsPointerToMemberFunction");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isReturnValue(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsReturnValue");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isSdl(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsSdl");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isSingleInheritance(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsSingleInheritance");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isSplitted(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsSplitted");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isStatic(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsStatic");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isStripped(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsStripped");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_isVirtualInheritance(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"IsVirtualInheritance");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_language(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Language");
		sz.Format(L"%s (%d)", PEPdb::GSZ_LANG_SPECS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_length(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Length");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_libraryName(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LibraryName");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_liveRangeLength(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LiveRangeLength");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_liveRangeStartAddressSection(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LiveRangeStartAddressSection");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_liveRangeStartAddressOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LiveRangeStartAddressOffset");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_liveRangeStartRelativeVirtualAddress(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LiveRangeStartRelativeVirtualAddress");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_locationType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LocationType");
		sz.Format(L"%s (%d)", PEPdb::GSZ_LOC_TYPES[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_lowerBound(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LowerBound");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);

		PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, (WORD)tag, pIRet);
		GetListCtrl().SetItemData(nItem, DWORD_PTR(pdn));
		pIRet = 0;
		nItem++;
	}

	if (pISym->get_lowerBoundId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"LowerBoundId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_machineType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"MachineType");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_managed(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Managed");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_memorySpaceKind(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"MemorySpaceKind");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_msil(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Msil");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_name(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Name");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_nested(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Nested");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_noInline(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NoInline");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_noReturn(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NoReturn");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_noStackOrdering(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NoStackOrdering");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_notReached(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NotReached");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_numberOfAcceleratorPointerTags(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NumberOfAcceleratorPointerTags");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_numberOfModifiers(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NumberOfModifiers");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_numberOfRegisterIndices(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NumberOfRegisterIndices");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_numberOfRows(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NumberOfRows");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_numberOfColumns(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"NumberOfColumns");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_objectFileName(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ObjectFileName");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_objectPointerType(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ObjectPointerType");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);

		PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, (WORD)tag, pIRet);
		GetListCtrl().SetItemData(nItem, DWORD_PTR(pdn));
		pIRet = 0;
		nItem++;
	}

	if (pISym->get_oemId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"OemId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_oemSymbolId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"OemSymbolId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_offset(&lRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Offset");
		sz.Format(L"%d", lRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_optimizedCodeDebugInfo(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"OptimizedCodeDebugInfo");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_overloadedOperator(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"OverloadedOperator");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_packed(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Packed");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_platform(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Platform");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_pure(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Pure");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_rank(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Rank");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_reference(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Reference");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_registerId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"RegisterId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_registerType(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"RegisterType");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_relativeVirtualAddress(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"RelativeVirtualAddress");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_restrictedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"RestrictedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_samplerSlot(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SamplerSlot");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_scoped(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Scoped");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_signature(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Signature");
		sz.Format(L"0x%08X", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_sizeInUdt(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SizeInUdt");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_slot(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Slot");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_sourceFileName(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SourceFileName");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_stride(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Stride");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_subType(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SubType");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);

		PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, (WORD)tag, pIRet);
		GetListCtrl().SetItemData(nItem, DWORD_PTR(pdn));
		pIRet = 0;
		nItem++;
	}

	if (pISym->get_subTypeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SubTypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_symbolsFileName(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"SymbolsFileName");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_targetOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TargetOffset");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_targetRelativeVirtualAddress(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TargetRelativeVirtualAddress");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_targetSection(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TargetSection");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_targetVirtualAddress(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TargetVirtualAddress");
		sz.Format(L"%I64d", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_textureSlot(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TextureSlot");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_thisAdjust(&lRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ThisAdjust");
		sz.Format(L"%d", lRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_thunkOrdinal(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"ThunkOrdinal");
		sz.Format(L"%s (%d)", PEPdb::GSZ_THUNK_ORDS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_timeStamp(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TimeStamp");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_token(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Token");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_type(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Type");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);

		PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, (WORD)tag, pIRet);
		GetListCtrl().SetItemData(nItem, DWORD_PTR(pdn));
		pIRet = 0;
		nItem++;
	}

	if (pISym->get_typeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"TypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_uavSlot(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UavSlot");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_udtKind(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UdtKind");
		sz.Format(L"%s (%d)", PEPdb::GSZ_UDT_KINDS[dwRet], dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unalignedType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnalignedType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_undecoratedName(&bszRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UndecoratedName");
		sz = bszRet;
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_unmodifiedTypeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UnmodifiedTypeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_upperBound(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UpperBound");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);

		PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, (WORD)tag, pIRet);
		GetListCtrl().SetItemData(nItem, DWORD_PTR(pdn));
		pIRet = 0;
		nItem++;
	}

	if (pISym->get_upperBoundId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"UpperBoundId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_value(&vtRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Value");
		sz = PEPdb::ToStr_Variant(&vtRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtual(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"Virtual");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualAddress(&llRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualAddress");
		sz.Format(L"0x%016I64X", llRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualBaseClass(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualBaseClass");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualBaseDispIndex(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualBaseDispIndex");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualBaseOffset(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualBaseOffset");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualBasePointerOffset(&lRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualBasePointerOffset");
		sz.Format(L"%d", lRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_virtualBaseTableType(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualBaseTableType");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);

		PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, (WORD)tag, pIRet);
		GetListCtrl().SetItemData(nItem, DWORD_PTR(pdn));
		pIRet = 0;
		nItem++;
	}

	if (pISym->get_virtualTableShape(&pIRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualTableShape");
		DIA_SYMTAGS tag = DIA_SYMTAGS::SymTagNull;
		pIRet->get_symTag((PDWORD)&tag);
		sz.Format(L"[%s] %s", PEPdb::GSZ_SYM_TAGS[tag], PEPdb::ToStr_Type(pIRet));
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);

		PDIA_NODE pdn = new DIA_NODE(NODE_SYMBOL, (WORD)tag, pIRet);
		GetListCtrl().SetItemData(nItem, DWORD_PTR(pdn));
		pIRet = 0;
		nItem++;
	}

	if (pISym->get_virtualTableShapeId(&dwRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VirtualTableShapeId");
		sz.Format(L"%d", dwRet);
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}

	if (pISym->get_volatileType(&fRet) == S_OK)
	{
		GetListCtrl().InsertItem(nItem, L"VolatileType");
		sz = fRet ? L"yes" : L"no";
		GetListCtrl().SetItem(nItem, 1, TVIF_TEXT, sz, 0, 0, 0, 0);
		nItem++;
	}
}
