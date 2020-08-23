#pragma once
#include "PEWrapCtrl.h"
#include "PEDoc.h"

class CRegstrDock : public CDockablePane
{
#define PROC_32BIT	0
#define PROC_WOW32	1
#define	PROC_64BIT	2
	PVOID	m_pctx, m_pbak;
	DWORD	m_dwThrId;
	bool	m_bIs32;
	bool	m_bHalt;

public:
	CRegstrDock();
	virtual ~CRegstrDock();

	void InitThreadCtx(bool bIs32);
	void UpdateThreadCtx(DWORD dwThrId, PVOID pCtx);
	void UninitThreadCtx();

protected:
	CViewList	m_wndView;

	BOOL AddItem(int nIndex, PDWORD_PTR* value);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnBeginLabelEdit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnEndLabelEdit(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnDestroy();
};
