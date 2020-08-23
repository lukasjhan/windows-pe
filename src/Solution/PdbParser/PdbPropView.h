#pragma once

class CPdbDoc;
class CPdbPropView : public CListView
{
	typedef void (CPdbPropView::*PFN_PROP_SYMTAG)(IDiaSymbol*);
	PFN_PROP_SYMTAG m_pfnSymTags[DIA_SYMTAGS::SymTagMax];
	void PropViewNull(IDiaSymbol* pISym);
	void PropViewExe(IDiaSymbol* pISym);
	void PropViewCompiland(IDiaSymbol* pISym);
	void PropViewCompilandDetails(IDiaSymbol* pISym);
	void PropViewCompilandEnv(IDiaSymbol* pISym);
	void PropViewFunction(IDiaSymbol* pISym);
	void PropViewBlockAnnote(IDiaSymbol* pISym);
	void PropViewData(IDiaSymbol* pISym);
	void PropViewLabel(IDiaSymbol* pISym);
	void PropViewPublicSymbol(IDiaSymbol* pISym);
	void PropViewUDT(IDiaSymbol* pISym);
	void PropViewEnum(IDiaSymbol* pISym);
	void PropViewFunctionType(IDiaSymbol* pISym);
	void PropViewPointerType(IDiaSymbol* pISym);
	void PropViewArrayType(IDiaSymbol* pISym);
	void PropViewBaseType(IDiaSymbol* pISym);
	void PropViewTypedef(IDiaSymbol* pISym);
	void PropViewBaseClass(IDiaSymbol* pISym);
	void PropViewFriend(IDiaSymbol* pISym);
	void PropViewFunctionArgType(IDiaSymbol* pISym);
	void PropViewFuncDebugStartEnd(IDiaSymbol* pISym);
	void PropViewUsingNamespace(IDiaSymbol* pISym);
	void PropViewVTableShape(IDiaSymbol* pISym);
	void PropViewVTable(IDiaSymbol* pISym);
	void PropViewCustom(IDiaSymbol* pISym);
	void PropViewThunk(IDiaSymbol* pISym);
	void PropViewCustomType(IDiaSymbol* pISym);
	void PropViewManagedType(IDiaSymbol* pISym);
	void PropViewDimension(IDiaSymbol* pISym);
	void PropViewCallSite(IDiaSymbol* pISym);
	void PropViewInlineSite(IDiaSymbol* pISym);
	void PropViewBaseInterface(IDiaSymbol* pISym);
	void PropViewVectorType(IDiaSymbol* pISym);
	void PropViewMatrixType(IDiaSymbol* pISym);
	void PropViewHLSLType(IDiaSymbol* pISym);
	void PropViewCaller(IDiaSymbol* pISym);
	void PropViewCallee(IDiaSymbol* pISym);
	void PropViewAll(IDiaSymbol* pISym);

protected:
	CPdbPropView();
	DECLARE_DYNCREATE(CPdbPropView)
	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);

	virtual void OnInitialUpdate(); // 생성 후 처음 호출되었습니다.

public:
	virtual ~CPdbPropView();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
	CPdbDoc* GetDocument() const;

	bool UpdateDIAProps(PDIA_NODE pdn);

};

#ifndef _DEBUG  // PdbDetail.cpp의 디버그 버전
inline CPdbDoc* CPdbPropView::GetDocument() const
   { return reinterpret_cast<CPdbDoc*>(m_pDocument); }
#endif

