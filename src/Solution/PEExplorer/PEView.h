#pragma once
#include "PEAnals.h"

class CPEView : public CWnd
{
	static PCWSTR GSZ_COL_NAMES[COL_IDX_MAX];

	CFont			m_tvFont;
	CImageList		m_imgPE;
	CTreeListCtrl	m_ctlPE;
	ANAL_MAP		m_anals;

// 생성입니다.
public:
	CPEView();
	virtual ~CPEView();

// 특성입니다.
public:

// 작업입니다.
public:
	static int CALLBACK NodeCompareProc(LPARAM lPrm1, LPARAM lPrm2, LPARAM lSort);
	void SortChildNode(HTREEITEM hNode);

	PPE_NODE GetPENode(HTREEITEM htiCur);

// 재정의입니다.
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

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
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnFileOpen();
	afx_msg void OnFileClose();
	afx_msg void OnUpdateFileClose(CCmdUI *pCmdUI);
	afx_msg void OnHexaView();
	afx_msg void OnCodeView();
	afx_msg void OnDataView();
	afx_msg void OnResourceView();
	afx_msg void OnCertInfoView();
	afx_msg void OnCopyDump();
	afx_msg void OnViewDetail();
	afx_msg void OnLviCopyItem(UINT uCmdId);
	afx_msg void OnLviFindItem();

};

