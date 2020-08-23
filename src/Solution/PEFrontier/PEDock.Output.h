#pragma once

class CTextView : public CEdit
{
public:
	CTextView();
	virtual ~CTextView();

protected:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();

	DECLARE_MESSAGE_MAP()
};


class COutputDock : public CDockablePane
{
public:
	COutputDock();
	virtual ~COutputDock();

	void PrintOutputStr(DWORD dwDbgCode, PCWSTR pszMsg);

protected:
	CTextView	m_wndView;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	DECLARE_MESSAGE_MAP()
};
