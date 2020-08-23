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
	
protected: // serialization������ ��������ϴ�.
	CMainFrm();
	DECLARE_DYNCREATE(CMainFrm)

// Ư���Դϴ�.
protected:
	CSplitterWnd	m_spltMain;
	CSplitterWnd	m_spltNested;
	CSplitterWnd	m_spltLeft;

public:

// �۾��Դϴ�.
public:

// �������Դϴ�.
public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// �����Դϴ�.
public:
	virtual ~CMainFrm();
	CPdbListView* GetRightPane();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // ��Ʈ�� ������ ���Ե� ����Դϴ�.
	CStatusBar        m_wndStatusBar;

// ������ �޽��� �� �Լ�
protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnUpdateViewStyles(CCmdUI* pCmdUI);
	afx_msg void OnViewStyle(UINT nCommandID);
	DECLARE_MESSAGE_MAP()

};


