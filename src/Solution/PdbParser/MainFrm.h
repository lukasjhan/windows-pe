#pragma once


class CMySplitterWnd : public CSplitterWnd
{
protected:
	bool m_bShowSplitter;    // hide column number, -1 if all columns
	// are shown

	void OnDrawSplitter(CDC* pDC, ESplitType nType, const CRect& rect)
	{
		if (m_bShowSplitter)
			CSplitterWnd::OnDrawSplitter(pDC,nType,rect );
	}
	void OnInvertTracker(const CRect& rect)
	{
		if (m_bShowSplitter)
			CSplitterWnd::OnInvertTracker(rect);
	}

public:
	CMySplitterWnd() 
	{ 
		m_cxSplitterGap = m_cySplitterGap = 1; m_cxSplitter = m_cySplitter = 0;
		m_bShowSplitter = false;
	}

	void ShowSplitter(bool bShow)
	{
		m_bShowSplitter = bShow;
	}

// Generated message map functions
protected:
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};


class CPdbListView;
class CMainFrm : public CFrameWnd
{
	
protected: // serialization에서만 만들어집니다.
	CMainFrm();
	DECLARE_DYNCREATE(CMainFrm)

// 특성입니다.
protected:
	CSplitterWnd	m_spltMain;
	CSplitterWnd	m_spltNested;
	CSplitterWnd	m_spltLeft;

public:

// 작업입니다.
public:

// 재정의입니다.
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// 구현입니다.
public:
	virtual ~CMainFrm();
	CPdbListView* GetRightPane();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // 컨트롤 모음이 포함된 멤버입니다.
	CStatusBar        m_wndStatusBar;

// 생성된 메시지 맵 함수
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateViewStyles(CCmdUI* pCmdUI);
	afx_msg void OnViewStyle(UINT nCommandID);
	DECLARE_MESSAGE_MAP()

};


