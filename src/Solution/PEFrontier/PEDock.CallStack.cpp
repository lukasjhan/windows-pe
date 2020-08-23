#include "stdafx.h"
#include "PEFrame.h"
#include "PEDock.CallStack.h"
#include "Resource.h"
#include "PEApp.h"
#include "PEDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CCallStackDock, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(LVN_GETDISPINFO, 4, OnLvnGetdispinfo)
	ON_NOTIFY(NM_CLICK, 4, OnNMClick)
	ON_NOTIFY(NM_DBLCLK, 4, OnNMDblclk)
	ON_NOTIFY(NM_CUSTOMDRAW, 4, OnNMCustomdraw)
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_COMMAND(ID_OPEN, OnFileOpen)
	ON_COMMAND(ID_OPEN_WITH, OnFileOpenWith)
	ON_COMMAND(IDM_VIEW_PROC_VAS, OnDummyCompile)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CCallStackDock
CCallStackDock::CCallStackDock()
{
	m_bIs32 = m_bHalt = false;
	m_dwDbgThId = 0;
}

CCallStackDock::~CCallStackDock()
{
}

BOOL CCallStackDock::AddItem(int nIndex, LPARAM pInfo)
{
	LV_ITEM	lvi;
	lvi.mask		= LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvi.iItem		= nIndex;
	lvi.iSubItem	= 0;
	lvi.iImage		= I_IMAGECALLBACK;
	lvi.pszText		= LPSTR_TEXTCALLBACK;
	lvi.lParam		= (LPARAM)pInfo;
	if (m_wndView.InsertItem(&lvi) == -1)
		return FALSE;
	return TRUE;
}

void CCallStackDock::UpdateCallStack(bool bIs32bit, DWORD dwThrId, PDBG_CALL_STACK pCStk)
{
	if (pCStk == NULL)
	{
		m_bHalt = false;
		m_wndView.SetBkColor(RGB(235, 235, 235));
		m_wndView.Invalidate();
		return;
	}

	m_bHalt = true;
	m_bIs32 = bIs32bit;
	m_dwDbgThId = dwThrId;
	m_wndView.DeleteAllItems();
	int nIdx = 0;
	for (DBG_CALL_STACK::reverse_iterator it = pCStk->rbegin(); it != pCStk->rend(); it++)
	{
		AddItem(nIdx, (LPARAM)(*it));
		nIdx++;
	}
	m_wndView.SetBkColor(RGB(255, 255, 255));
	m_wndView.Invalidate();
}


/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar 메시지 처리기
int CCallStackDock::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rc; rc.SetRectEmpty();
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL;
	if (!m_wndView.Create(dwViewStyle, rc, this, 4))
	{
		TRACE0("중단점 뷰를 만들지 못했습니다.\n");
		return -1;
	}
	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP;
	m_wndView.SetExtendedStyle(m_wndView.GetExtendedStyle() | dwExStyle);

	CBitmap bmp; bmp.LoadBitmap(IDB_DBG_BREAK);
	m_imgView.Create(18, 18, ILC_MASK | ILC_COLOR32, 0, 0);
	m_imgView.Add(&bmp, RGB(230, 231, 232));
	m_wndView.SetImageList(&m_imgView, LVSIL_SMALL);

	m_wndView.SetFont(((CPEApp*)AfxGetApp())->GetFont());
	m_wndView.InsertColumn(0, _T("함수명"), LVCFMT_LEFT, 240);
	m_wndView.InsertColumn(1, _T("위치"), LVCFMT_LEFT, 150);
	m_wndView.InsertColumn(2, _T("스택"), LVCFMT_LEFT, 100);
	m_wndView.InsertColumn(3, _T("상세"), LVCFMT_LEFT, 120);
	m_wndView.SetBkColor(RGB(220, 220, 220));
	m_wndView.EnableWindow(FALSE);
	return 0;
}

void CCallStackDock::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	if (GetSafeHwnd() == NULL)
		return;

	CRect rc; GetClientRect(rc);
	m_wndView.SetWindowPos(NULL, rc.left + 1, rc.top + 1,
		rc.Width() - 2, rc.Height() - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CCallStackDock::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_wndView;
	ASSERT_VALID(pWndTree);

	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	if (point != CPoint(-1, -1))
	{
		// 클릭한 항목을 선택합니다.
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		HTREEITEM hTreeItem = pWndTree->HitTest(ptTree, &flags);
		if (hTreeItem != NULL)
		{
			pWndTree->SelectItem(hTreeItem);
		}
	}

	pWndTree->SetFocus();
	G_PE_APP.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_PRJMGR_DEFAULT, point.x, point.y, this, TRUE);
}

void CCallStackDock::OnPaint()
{
	CPaintDC dc(this);
	CRect rc; m_wndView.GetWindowRect(rc);
	ScreenToClient(rc);
	rc.InflateRect(1, 1);
	dc.Draw3dRect(rc, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CCallStackDock::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndView.SetFocus();
}

void CCallStackDock::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pnmdvi = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	PDBG_CALLSITE pcs = (PDBG_CALLSITE)pnmdvi->item.lParam;

	if (pnmdvi->item.mask & LVIF_IMAGE)
	{
		pnmdvi->item.iImage = 0;
	}

	if (pnmdvi->item.mask & LVIF_TEXT)
	{
		switch (pnmdvi->item.iSubItem)
		{
		case 0:	// 함수명
			pnmdvi->item.pszText = (PWSTR)(PCWSTR)pcs->FuncName;
			break;
#ifdef _X86_
		case 1:	// 위치
			wsprintf(pnmdvi->item.pszText, L"0x%s", PEPlus::GetAddrForm(false, (PBYTE)pcs->Context.Eip));
			break;
		case 2:	// RVA
			wsprintf(pnmdvi->item.pszText, L"0x%s", PEPlus::GetAddrForm(false, (PBYTE)pcs->Context.Esp));
			break;
#else
		case 1:	// 위치
			wsprintf(pnmdvi->item.pszText, L"0x%s", 
				PEPlus::GetAddrForm(m_bIs32, (PBYTE)((m_bIs32) ? pcs->CtxWow.Eip : pcs->Context.Rip)));
			break;
		case 2:	// RVA
			wsprintf(pnmdvi->item.pszText, L"0x%s", 
				PEPlus::GetAddrForm(m_bIs32, (PBYTE)((m_bIs32) ? pcs->CtxWow.Esp : pcs->Context.Rsp)));
			break;
#endif
		case 3:	// 설명
			break;
		}
	}
	*pResult = 0;
}

void CCallStackDock::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pia = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint point(pia->ptAction);

	UINT uFlags = 0;
	int nItem = m_wndView.HitTest(point, &uFlags);
	if (nItem < 0)
		return;
	*pResult = 0;
}

void CCallStackDock::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pia = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint point(pia->ptAction);

	UINT uFlags = 0;
	int nItem = m_wndView.HitTest(point, &uFlags);
	if (nItem < 0)
		return;

	CPEFrame* pMain = ((CPEFrame*)GetParentOwner());
	PDBG_CALLSITE pcs = (PDBG_CALLSITE)m_wndView.GetItemData(nItem);
#ifdef _X86_
	PBYTE pAddr = (PBYTE)pcs->Context.Eip;
	PVOID pCtx  = &pcs->Context;
	pMain->UpdateDAsmView(pAddr, m_dwDbgThId, pCtx);
#else
	PBYTE pAddr = NULL;
	PVOID pCtx = NULL;
	if (m_bIs32)
	{
		pAddr = (PBYTE)pcs->CtxWow.Eip;
		pCtx = &pcs->CtxWow;
	}
	else
	{
		pAddr = (PBYTE)pcs->Context.Rip;
		pCtx = &pcs->Context;
	}
	pMain->UpdateDAsmView(pAddr, m_dwDbgThId, pCtx);
#endif
	*pResult = 0;
}

void CCallStackDock::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCUSTOMDRAW pvcd = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	switch (pvcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYSUBITEMDRAW;
			break;
		case CDDS_ITEMPREPAINT:
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
			break;
		case (CDDS_ITEMPREPAINT | CDDS_SUBITEM) :
		{
			pvcd->clrTextBk = (m_bHalt) ? RGB(255, 255, 255) : RGB(235, 235, 235);
			*pResult = CDRF_NEWFONT;
		}
		break;
	default:
		*pResult = CDRF_DODEFAULT;
		break;
	}
}

void CCallStackDock::OnProperties()
{
	AfxMessageBox(_T("속성...."));

}

void CCallStackDock::OnFileOpen()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CCallStackDock::OnFileOpenWith()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CCallStackDock::OnDummyCompile()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CCallStackDock::OnEditCut()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CCallStackDock::OnEditCopy()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CCallStackDock::OnEditClear()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}
