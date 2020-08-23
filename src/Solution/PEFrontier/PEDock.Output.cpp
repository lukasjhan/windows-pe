#include "stdafx.h"
#include "PEFrame.h"
#include "PEDock.Output.h"
#include "Resource.h"
#include "PEApp.h"
#include "PEFrame.h"
#include "PEDoc.h"


/////////////////////////////////////////////////////////////////////////////
// COutputList1
CTextView::CTextView()
{
}

CTextView::~CTextView()
{
}

BEGIN_MESSAGE_MAP(CTextView, CEdit)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextView 메시지 처리기
void CTextView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu menu;
	menu.LoadMenu(IDR_OUTPUT_POPUP);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}

	SetFocus();
}

void CTextView::OnViewOutput()
{
	CDockablePane* pParentBar = DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());
	if (pMainFrame != NULL && pParentBar != NULL)
	{
		pMainFrame->SetFocus();
		pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();
	}
}
/////////////////////////////////////////////////////////////////////////////





/////////////////////////////////////////////////////////////////////////////
// COutputDock
BEGIN_MESSAGE_MAP(COutputDock, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// 생성/소멸
COutputDock::COutputDock()
{
}

COutputDock::~COutputDock()
{
}

void COutputDock::PrintOutputStr(DWORD dwDbgCode, PCWSTR pszMsg)
{
	CString szMsg;
	szMsg.Format(L"%s : %s\xd\xa", CPEDoc::GSZ_DBG_EVENTS[dwDbgCode], pszMsg);
	int nTxtLen = m_wndView.GetWindowTextLength();
	m_wndView.SetSel(nTxtLen, nTxtLen);
	m_wndView.ReplaceSel(szMsg);
}

/////////////////////////////////////////////////////////////////////////////
// COutputDock 메시지 처리기
int COutputDock::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// 뷰를 만듭니다.
	CRect rc; rc.SetRectEmpty();
	const DWORD dwStyle = ES_MULTILINE | ES_NOHIDESEL | ES_READONLY | 
		ES_WANTRETURN | /*ES_AUTOHSCROLL | */WS_CHILD | WS_VISIBLE | WS_VSCROLL;
	if (!m_wndView.Create(dwStyle, rc, this, 2))
	{
		TRACE0("출력 창을 만들지 못했습니다.\n");
		return -1;
	}
	m_wndView.SetFont(((CPEApp*)AfxGetApp())->GetFont());
	return 0;
}

void COutputDock::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	if (GetSafeHwnd() == NULL)
		return;
	CRect rc; GetClientRect(rc);
	m_wndView.SetWindowPos(NULL, rc.left + 1, rc.top + 1,
		rc.Width() - 2, rc.Height() - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void COutputDock::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndView.SetFocus();
}

void COutputDock::OnPaint()
{
	CPaintDC dc(this);
	CRect rc; m_wndView.GetWindowRect(rc);
	ScreenToClient(rc);
	rc.InflateRect(1, 1);
	dc.Draw3dRect(rc, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CTextView::OnEditCopy()
{
	MessageBox(_T("출력 복사"));
}

void CTextView::OnEditClear()
{
	MessageBox(_T("출력 지우기"));
}
