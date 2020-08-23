#pragma once

class CPdbTextView : public CEditView
{
protected: // serialization������ ��������ϴ�.
	CPdbTextView();
	DECLARE_DYNCREATE(CPdbTextView)

	CFont m_fnt;

// Ư���Դϴ�.
public:
	CPdbDoc* GetDocument() const;

// �۾��Դϴ�.
public:
	bool UpdateCode(PDIA_NODE pdn);

	static CString GetCode_Function(IDiaSymbol* pISymbol);
	static CString GetCode_Enum(IDiaSymbol* pISymbol);
	static CString GetCode_UDT(IDiaSymbol* pISymbol, int nDepth);

// �������Դϴ�.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// �����Դϴ�.
public:
	virtual ~CPdbTextView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ������ �޽��� �� �Լ�
protected:
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();

};

#ifndef _DEBUG  // PEText.cpp�� ����� ����
inline CPdbDoc* CPdbTextView::GetDocument() const
{
	return reinterpret_cast<CPdbDoc*>(m_pDocument);
}
#endif

