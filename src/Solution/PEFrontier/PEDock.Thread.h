#pragma once
#include "PEWrapCtrl.h"
#include "PEDoc.h"

class CThreadDock : public CDockablePane
{
	bool	m_bHalt;

public:
	CThreadDock();
	virtual ~CThreadDock();

	void EnableControls(int nOpt)
	{
		if (nOpt == 0)
		{
			m_wndView.EnableWindow(FALSE);
			m_wndView.DeleteAllItems();
		}
		else if (nOpt == 1)
			m_wndView.EnableWindow();
		else
		{
			if (nOpt == 2)
				m_wndView.SetBkColor(RGB(255, 255, 255));
			else
				m_wndView.SetBkColor(RGB(235, 235, 235));
			m_wndView.Invalidate();
		}
	}
	void UpdateItem(int nOpt, PDBG_THREAD pdt);

protected:
	CImageList	m_imgDock;
	CViewList	m_wndView;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

};
