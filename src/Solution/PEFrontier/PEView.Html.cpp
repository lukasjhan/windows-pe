#include "stdafx.h"
#ifndef SHARED_HANDLERS
#	include "PEApp.h"
#endif
#include "PEDoc.h"
#include "PEView.Html.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CFrontierView
IMPLEMENT_DYNCREATE(CFrontierView, CHtmlView)
BEGIN_MESSAGE_MAP(CFrontierView, CHtmlView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CFrontierView 생성/소멸
CFrontierView::CFrontierView()
{
}

CFrontierView::~CFrontierView()
{
}

BOOL CFrontierView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:
	return CHtmlView::PreCreateWindow(cs);
}

void CFrontierView::OnInitialUpdate()
{
	CHtmlView::OnInitialUpdate();
	//Navigate2(_T("http://www.msdn.microsoft.com/korea/visualc/"), NULL, NULL);
	LoadFromResource(IDR_PE_HTML);
}

void CFrontierView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CFrontierView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	G_PE_APP.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CFrontierView 진단

#ifdef _DEBUG
void CFrontierView::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CFrontierView::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}

CPEDoc* CFrontierView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPEDoc)));
	return (CPEDoc*)m_pDocument;
}
#endif //_DEBUG

// CFrontierView 메시지 처리기
