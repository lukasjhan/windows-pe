#include "stdafx.h"
#include "PEFrame.h"
#include "PEDock.Thread.h"
#include "Resource.h"
#include "PEApp.h"
#include "PEFrame.h"
#include "PEDoc.h"

PCTSTR G_PRIORITIES[] = 
{
	L"휴지", L"낮음", L"조금 낮음", L"보통", L"조금 높음", L"높음", L"실시간"
};


BEGIN_MESSAGE_MAP(CThreadDock, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_GETDISPINFO, 2, OnLvnGetdispinfo)
	ON_NOTIFY(NM_CUSTOMDRAW, 2, OnNMCustomdraw)
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////
// 생성/소멸
//////////////////////////////////////////////////////////////////////
CThreadDock::CThreadDock()
{
	m_bHalt = false;
}

CThreadDock::~CThreadDock()
{
}

void CThreadDock::UpdateItem(int nOpt, PDBG_THREAD pdt)
{
	if (nOpt == DBG_NOTI_CLEAR)
	{
		m_wndView.DeleteAllItems();
		return;
	}

	int nItemCnt = m_wndView.GetItemCount();
	if (nOpt == DBG_NOTI_APPEND)
	{
		LV_ITEM		 lvi;
		lvi.mask	 = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
		lvi.iItem	 = m_wndView.GetItemCount();
		lvi.iSubItem = 0;
		lvi.iImage	 = I_IMAGECALLBACK;
		lvi.pszText	 = LPSTR_TEXTCALLBACK;
		lvi.lParam	 = (LPARAM)pdt;
		m_wndView.InsertItem(&lvi);
	}
	else
	{
		for (int i = 0; i < nItemCnt; i++)
		{
			PDBG_THREAD pvd = (PDBG_THREAD)m_wndView.GetItemData(i);
			if (pvd->ThreadId == pdt->ThreadId)
			{
				m_wndView.DeleteItem(i);
				break;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
// CThreadDock 메시지 처리기
int CThreadDock::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// 뷰를 만듭니다.
	CRect rc; rc.SetRectEmpty();
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL;
	if (!m_wndView.Create(dwViewStyle, rc, this, 2))
	{
		TRACE0("스레드 뷰를 만들지 못했습니다.\n");
		return -1;
	}
	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP;
	m_wndView.SetExtendedStyle(m_wndView.GetExtendedStyle() | dwExStyle);

	m_imgDock.Create(IDB_DOCK_THREAD, 18, 2, RGB(255, 0, 255));
	m_wndView.SetImageList(&m_imgDock, LVSIL_SMALL);
	m_wndView.SetFont(((CPEApp*)AfxGetApp())->GetFont());

	m_wndView.InsertColumn(0, _T("  ID"), LVCFMT_LEFT, 100);
	m_wndView.InsertColumn(1, _T("메인"), LVCFMT_CENTER, 40);
	m_wndView.InsertColumn(2, _T("이름"), LVCFMT_LEFT, 250);
	m_wndView.InsertColumn(3, _T("시작 주소"), LVCFMT_LEFT, 150);
	m_wndView.InsertColumn(4, _T("스택 베이스"), LVCFMT_LEFT, 150);
	m_wndView.InsertColumn(5, _T("우선순위"), LVCFMT_CENTER, 60);
	m_wndView.SetBkColor(RGB(220, 220, 220));
	m_wndView.EnableWindow(FALSE);
	return 0;
}

void CThreadDock::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	if (GetSafeHwnd() == NULL)
		return;
	CRect rc; GetClientRect(rc);
	m_wndView.SetWindowPos(NULL, rc.left + 1, rc.top + 1,
		rc.Width() - 2, rc.Height() - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CThreadDock::OnPaint()
{
	CPaintDC dc(this);
	CRect rc; m_wndView.GetWindowRect(rc);
	ScreenToClient(rc);
	rc.InflateRect(1, 1);
	dc.Draw3dRect(rc, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CThreadDock::OnContextMenu(CWnd* pWnd, CPoint point)
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
	CMenu menu;
	menu.LoadMenu(IDR_POPUP_SORT);

	CMenu* pSumMenu = menu.GetSubMenu(0);

	if (AfxGetMainWnd()->IsKindOf(RUNTIME_CLASS(CMDIFrameWndEx)))
	{
		CMFCPopupMenu* pPopupMenu = new CMFCPopupMenu;

		if (!pPopupMenu->Create(this, point.x, point.y, (HMENU)pSumMenu->m_hMenu, FALSE, TRUE))
			return;

		((CMDIFrameWndEx*)AfxGetMainWnd())->OnShowPopupMenu(pPopupMenu);
		UpdateDialogControls(this, FALSE);
	}
}

void CThreadDock::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndView.SetFocus();
}

void CThreadDock::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pnmdvi = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	PDBG_THREAD pdt = (PDBG_THREAD)pnmdvi->item.lParam;

	if (pnmdvi->item.mask & LVIF_IMAGE)
	{
		int nImgIdx = pdt->IsMain ? 0 : 2;
		if (pdt->Active)
			nImgIdx++;
		pnmdvi->item.iImage = nImgIdx;
	}

	if (pnmdvi->item.mask & LVIF_TEXT)
	{
		switch (pnmdvi->item.iSubItem)
		{
			case 0:	// ID
				wsprintf(pnmdvi->item.pszText, L"%d", pdt->ThreadId);
			break;
			case 1:	// 메인
				if (pdt->IsMain)
					wcscpy_s(pnmdvi->item.pszText, 3, L"■");
			break;
			case 2:	// 이름
				pnmdvi->item.pszText = (PWSTR)(PCWSTR)pdt->ThreadName;
			break;
			case 3:	// 시작 주소
				wsprintf(pnmdvi->item.pszText, L"0x%s",
					PEPlus::GetAddrForm(pdt->Process->Is32Bit, pdt->StartAddr));
			break;
			case 4:	// 스택 베이스
				wsprintf(pnmdvi->item.pszText, L"0x%s",
					PEPlus::GetAddrForm(pdt->Process->Is32Bit, pdt->StackBase));
			break;
			case 5:	// 우선순위
				if (pdt->Priority == THREAD_PRIORITY_IDLE)
					pnmdvi->item.pszText = (PWSTR)G_PRIORITIES[0];
				else if (pdt->Priority == THREAD_PRIORITY_TIME_CRITICAL)
					pnmdvi->item.pszText = (PWSTR)G_PRIORITIES[6];
				else
					pnmdvi->item.pszText = (PWSTR)
						G_PRIORITIES[pdt->Priority + THREAD_PRIORITY_HIGHEST + 1];
			break;
		}
	}
	*pResult = 0;
}

void CThreadDock::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
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

