#pragma once
#include "PEWrapCtrl.h"

class CBookToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}
	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CBookMarkDock : public CDockablePane
{
public:
	CBookMarkDock();
	virtual ~CBookMarkDock();

	void OnChangeVisualStyle();
	void InitBookMarks(PDBG_NLBM_MAP pNlBms);
	PDBG_BKMARK NewBookMark(PBYTE pBMAddr, PDBG_MODULE pdm);
	void UpdateBookMark(int nOpt, PDBG_BKMARK pdb);
	void UpdateBookMarkList()
	{
		m_wndView.Invalidate();
	}

protected:
	CImageList		m_imgView;
	CViewList		m_wndView;
	CImageList		m_FileViewImages;
	CBookToolBar	m_tbMain;

	BOOL AddItem(int nIndex, LPARAM pInfo);
	void AdjustLayout();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndLabelEdit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnProperties();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenWith();
	afx_msg void OnDummyCompile();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	DECLARE_MESSAGE_MAP()

};

