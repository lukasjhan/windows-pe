#pragma once

class CSrcView : public CEditView
{
protected: // serialization������ ��������ϴ�.
	CSrcView();
	DECLARE_DYNCREATE(CSrcView)

// Ư���Դϴ�.
public:
	CPEDoc* GetDocument() const;

// �۾��Դϴ�.
public:

// �������Դϴ�.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:

// �����Դϴ�.
public:
	virtual ~CSrcView();
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
	//virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
};

#ifndef _DEBUG  // PEText.cpp�� ����� ����
inline CPEDoc* CSrcView::GetDocument() const
{
	return reinterpret_cast<CPEDoc*>(m_pDocument);
}
#endif

