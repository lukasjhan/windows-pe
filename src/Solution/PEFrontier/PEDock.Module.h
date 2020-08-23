#pragma once
#include "PEWrapCtrl.h"

class CModuleDock : public CDockablePane
{
public:
	CModuleDock();
	virtual ~CModuleDock();

	void EnableControls(BOOL bEnable = TRUE)
	{
		m_wndView.EnableWindow(bEnable);
		if (!bEnable)
			m_wndView.DeleteAllItems();
	}
	void UpdateItem(int nOpt, PDBG_MODULE pdm);

protected:
	CImageList	m_imgDock;
	CViewList	m_wndView;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

};
