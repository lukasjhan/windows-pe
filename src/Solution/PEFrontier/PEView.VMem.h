#pragma once

class CVMemView : public CListView
{
	CImageList		m_imgView;
	DBG_VMOBJ_MAP	m_vmos;
	DBG_HEAP_MAP	m_heaps;
	bool			m_bIs32;

	BOOL AddItem(int nIndex, LPARAM pInfo);

protected:
	CVMemView();
	DECLARE_DYNCREATE(CVMemView)
	virtual void OnInitialUpdate(); // 생성 후 처음 호출되었습니다.

public:
	virtual ~CVMemView();

	CPEDoc* GetDocument() const;
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

public:

};
#ifndef _DEBUG  // PEView.cpp의 디버그 버전
inline CPEDoc* CVMemView::GetDocument() const
   { return reinterpret_cast<CPEDoc*>(m_pDocument); }
#endif

