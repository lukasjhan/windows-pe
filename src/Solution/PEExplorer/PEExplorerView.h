#pragma once
#include "PEAnals.h"

class CPEExplorerView : public CWnd
{
	static PCWSTR GSZ_COL_NAMES[COL_IDX_MAX];

	CFont			m_tvFont;
	CImageList		m_imgPE;
	CTreeListCtrl	m_ctlPE;
	ANAL_MAP		m_anals;
	HCURSOR			m_hOldCur;
	bool			m_bHndCur;

// 생성입니다.
public:
	CPEExplorerView();

// 특성입니다.
public:

// 작업입니다.
public:
	static int CALLBACK NodeCompareProc(LPARAM lPrm1, LPARAM lPrm2, LPARAM lSort);
	void SortChildNode(HTREEITEM hNode);

	PEAnals* GetPEAnals(HTREEITEM htiCur);

// 재정의입니다.
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 구현입니다.
public:
	virtual ~CPEExplorerView();

	// 생성된 메시지 맵 함수
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint ptMousePos);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);

	afx_msg void OnTvnDeleteitemPEView(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblClickPEView(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickPEView(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMLClickPEView(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMHoverPEView(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnFileOpen();
	afx_msg void OnFileClose();
	afx_msg void OnUpdateFileClose(CCmdUI *pCmdUI);
	afx_msg void OnHexaDump();
	afx_msg void OnCopyDump();
	afx_msg void OnViewDetail();

};

