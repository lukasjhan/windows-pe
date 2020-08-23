#pragma once

class CPdbTextView : public CEditView
{
protected: // serialization에서만 만들어집니다.
	CPdbTextView();
	DECLARE_DYNCREATE(CPdbTextView)

	CFont m_fnt;

// 특성입니다.
public:
	CPdbDoc* GetDocument() const;

// 작업입니다.
public:
	bool UpdateCode(PDIA_NODE pdn);

	static CString GetCode_Function(IDiaSymbol* pISymbol);
	static CString GetCode_Enum(IDiaSymbol* pISymbol);
	static CString GetCode_UDT(IDiaSymbol* pISymbol, int nDepth);

// 재정의입니다.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// 구현입니다.
public:
	virtual ~CPdbTextView();
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

};

#ifndef _DEBUG  // PEText.cpp의 디버그 버전
inline CPdbDoc* CPdbTextView::GetDocument() const
{
	return reinterpret_cast<CPdbDoc*>(m_pDocument);
}
#endif

