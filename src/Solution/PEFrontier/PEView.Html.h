#pragma once

class CFrontierView : public CHtmlView
{
protected: // serialization������ ��������ϴ�.
	CFrontierView();
	DECLARE_DYNCREATE(CFrontierView)

// Ư���Դϴ�.
public:
	CPEDoc* GetDocument() const;

// �۾��Դϴ�.
public:

// �������Դϴ�.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // ���� �� ó�� ȣ��Ǿ����ϴ�.

// �����Դϴ�.
public:
	virtual ~CFrontierView();
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
};

#ifndef _DEBUG  // TestHtmlView.cpp�� ����� ����
inline CPEDoc* CFrontierView::GetDocument() const
{
	return reinterpret_cast<CPEDoc*>(m_pDocument);
}
#endif

