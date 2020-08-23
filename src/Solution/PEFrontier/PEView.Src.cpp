#include "stdafx.h"
#ifndef SHARED_HANDLERS
#	include "PEApp.h"
#endif
#include "PEDoc.h"
#include "PEView.Src.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif

// CSrcView
IMPLEMENT_DYNCREATE(CSrcView, CEditView)
BEGIN_MESSAGE_MAP(CSrcView, CEditView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CSrcView 생성/소멸
CSrcView::CSrcView()
{
	// TODO: 여기에 생성 코드를 추가합니다.
}

CSrcView::~CSrcView()
{
}

BOOL CSrcView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CEditView::PreCreateWindow(cs))
		return FALSE;
	cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);	// 자동 줄바꿈을 사용합니다.
	cs.style |= ES_READONLY;
	return TRUE;
}

void CSrcView::OnInitialUpdate()
{
	CEditView::OnInitialUpdate();
	GetEditCtrl().SetFont(((CPEApp*)AfxGetApp())->GetFont());
}

//void CSrcView::OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint)
//{
//	if (lHint == 0)
//		return;
//	CPEDoc* pDoc = GetDocument();
//	PBYTE pBase = pDoc->GetImgView();
//
//	CString sz;
//	PPE_NODE pn = PPE_NODE(lHint);
//	DWORD dwOffset = pn->Offset;
//	for (int i = 0; i < pn->Size; i++)
//	{
//		if (i % 16 == 0)
//		{
//			if (i > 0)
//				sz.Append(_T("\xd\xa"));
//			sz.AppendFormat(_T("%08X  "), dwOffset);
//		}
//		sz.AppendFormat(_T("%02X "), pBase[dwOffset]);
//		dwOffset++;
//	}
//	GetEditCtrl().SetWindowTextW(sz);
//}

// CSrcView 진단
#ifdef _DEBUG
void CSrcView::AssertValid() const
{
	CEditView::AssertValid();
}

void CSrcView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CPEDoc* CSrcView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPEDoc)));
	return (CPEDoc*)m_pDocument;
}
#endif //_DEBUG

// CSrcView 메시지 처리기
void CSrcView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CSrcView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	G_PE_APP.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}
