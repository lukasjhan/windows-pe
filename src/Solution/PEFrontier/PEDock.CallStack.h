#pragma once
#include "PEWrapCtrl.h"

class CCallStackDock : public CDockablePane
{
public:
	CCallStackDock();
	virtual ~CCallStackDock();

	void EnableControls(BOOL bEnable = TRUE)
	{
		m_wndView.EnableWindow(bEnable);
		if (!bEnable)
			m_wndView.DeleteAllItems();
	}
	void UpdateCallStack(bool bIs32bit, DWORD dwThrId, PDBG_CALL_STACK pCStk);

protected:
	CImageList		m_imgView;
	CViewList		m_wndView;
	bool			m_bIs32;
	bool			m_bHalt;
	DWORD			m_dwDbgThId;

	BOOL AddItem(int nIndex, LPARAM pInfo);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnProperties();
	afx_msg void OnFileOpen();
	afx_msg void OnFileOpenWith();
	afx_msg void OnDummyCompile();
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	DECLARE_MESSAGE_MAP()
};

