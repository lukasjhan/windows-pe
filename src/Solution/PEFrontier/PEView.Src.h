#pragma once

class CSrcView : public CEditView
{
protected: // serialization에서만 만들어집니다.
	CSrcView();
	DECLARE_DYNCREATE(CSrcView)

// 특성입니다.
public:
	CPEDoc* GetDocument() const;

// 작업입니다.
public:

// 재정의입니다.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// 구현입니다.
public:
	virtual ~CSrcView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();
	//virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
};

#ifndef _DEBUG  // PEText.cpp의 디버그 버전
inline CPEDoc* CSrcView::GetDocument() const
{
	return reinterpret_cast<CPEDoc*>(m_pDocument);
}
#endif

