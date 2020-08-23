#pragma once

class CPdbDoc;
class CPdbTreeView : public CTreeView
{
	HTREEITEM AddItem(PCWSTR pszName, BOOL, PDIA_NODE, HTREEITEM);

protected: // serialization������ ��������ϴ�.
	CPdbTreeView();
	DECLARE_DYNCREATE(CPdbTreeView)

	HTREEITEM BuildTableNode(IDiaSession* pISess);
	HTREEITEM BuildPEScopeNode(IDiaSymbol* pIPE);

	void BuildGlobalNode(IDiaSymbol* pIBase, HTREEITEM htiUp);
	void BuildAllNodes(IDiaSymbol* pIBase, HTREEITEM htiUp);
	int BuildSymSubNode(IDiaSymbol* pISym, HTREEITEM htiUp, DIA_SYMTAGS eWantSym);
	int BuildSymbolNode(IDiaSymbol* pISym, HTREEITEM htiUp);

	virtual void OnInitialUpdate();

	afx_msg void OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

public:
	virtual ~CPdbTreeView();
	CPdbDoc* GetDocument();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};

#ifndef _DEBUG  // PdbView.cpp�� ����� ����
inline CPdbDoc* CPdbTreeView::GetDocument()
   { return reinterpret_cast<CPdbDoc*>(m_pDocument); }
#endif

