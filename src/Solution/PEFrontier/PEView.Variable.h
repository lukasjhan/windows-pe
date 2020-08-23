#pragma once
#include "PEWrapCtrl.h"

class CModuleView : public CDockablePane
{
public:
	CModuleView();
	virtual ~CModuleView();

protected:
	CViewList	m_wndView;

	BOOL AddItem(int nIndex, LPARAM pInfo);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnDebugEvent(WPARAM, LPARAM);
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()
};
