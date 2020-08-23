#pragma once
#include "PEWrapCtrl.h"

class CBreakToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}
	virtual BOOL AllowShowOnList() const { return FALSE; }
};

#define BPI_NOBP		0
#define BPI_NOBP_CUR	1
#define BPI_NOBP_POS	2
#define BPI_BP			3
#define BPI_BP_CUR		4
#define BPI_BP_POS		5
#define BPI_DISBP		6
#define BPI_DISBP_CUR	7
#define BPI_DISBP_POS	8
#define BPI_CONBP		9
#define BPI_CONBP_CUR	10
#define BPI_CONBP_POS	11
#define BPI_EP			12
#define BPI_EP_CUR		13
#define BPI_EP_POS		14
#define BPI_BMARK		15
#define BPI_BMARK_CUR	16
#define BPI_BMARK_POS	17
#define BPI_CODES_PLUS	18
#define BPI_CODES_MINUS	19


#define BPD_NOBP		0
#define BPD_BP			1
#define BPD_CONBP		2
#define BPD_DISBP		3
#define BPD_BP_CUR		4
#define BPD_BP_POS		5
#define BPD_BP_NL		6

class CBreakDock : public CDockablePane
{
// 생성입니다.
public:
	CBreakDock();
	virtual ~CBreakDock();

	void OnChangeVisualStyle();
	void InitBreakPoint(PDBG_NLBRK_MAP pNlBrks);
	void UpdateBreakPoint(int nOpt, PBYTE pBPAddr, PDBG_MODULE pdm);
	void UpdateBreakItem(PBYTE pBPAddr);
	void UpdateBrkPntList()
	{
		m_wndView.Invalidate();
	}
	CPEDoc* GetDocument()
	{
		CMDIFrameWndEx* pFrm = (CMDIFrameWndEx*)GetParentOwner();
		return (CPEDoc*)pFrm->GetActiveFrame()->GetActiveDocument();
	}

// 특성입니다.
protected:
	CImageList		m_imgView;
	CViewList		m_wndView;
	CImageList		m_FileViewImages;
	CBreakToolBar	m_tbMain;
	int				m_nCurItem;

	BOOL AddItem(int nIndex, LPARAM pInfo);
	void AdjustLayout();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnBeginLabelEdit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndLabelEdit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnProperties();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenWith();
	afx_msg void OnDummyCompile();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	DECLARE_MESSAGE_MAP()
};

