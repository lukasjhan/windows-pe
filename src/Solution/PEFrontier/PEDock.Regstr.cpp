#include "stdafx.h"
#include "PEFrame.h"
#include "PEDock.Regstr.h"
#include "Resource.h"
#include "PEApp.h"
#include "PEFrame.h"
#include "PEDoc.h"

PCWSTR G_SZREG32[] =
{
	L"EIP", L"ESP", L"EBP", L"EAX", L"EBX", L"ECX", 
	L"EDX", L"ESI", L"EDI", L"EFLAGS"
};

PCWSTR G_SZREG64[] =
{
	L"RIP", L"RSP", L"RBP", L"RAX", L"RBX", L"RCX", L"RDX", L"RSI", 
	L"RDI", L"R8",  L"R9",  L"R10", L"R11", L"R12", L"R13", L"R14", 
	L"R15", L"EFLGS"
};



BEGIN_MESSAGE_MAP(CRegstrDock, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_CONTEXTMENU()
	ON_WM_SETFOCUS()
	ON_NOTIFY(LVN_GETDISPINFO, 2, OnLvnGetdispinfo)
	ON_NOTIFY(LVN_BEGINLABELEDIT, 2, OnLvnBeginLabelEdit)
	ON_NOTIFY(LVN_ENDLABELEDIT, 2, OnLvnEndLabelEdit)
	ON_NOTIFY(LVN_DELETEITEM, 2, OnLvnDeleteitem)
	ON_NOTIFY(NM_CUSTOMDRAW, 2, OnNMCustomdraw)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////
// 생성/소멸
//////////////////////////////////////////////////////////////////////
CRegstrDock::CRegstrDock()
{
	m_pctx = m_pbak = NULL;
	m_dwThrId = 0;
	m_bIs32 = false;
}

CRegstrDock::~CRegstrDock()
{
}

BOOL CRegstrDock::AddItem(int nIndex, PDWORD_PTR* value)
{
	LV_ITEM	lvi;
	lvi.mask	 = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem	 = nIndex;
	lvi.iSubItem = 0;
	lvi.pszText	 = LPSTR_TEXTCALLBACK;
	lvi.lParam	 = (LPARAM)value;
	if (m_wndView.InsertItem(&lvi) == -1)
		return FALSE;
	return TRUE;
}

void CRegstrDock::InitThreadCtx(bool bIs32)
{
	m_bIs32 = bIs32;
	if (m_bIs32)
	{
		m_pctx = new WOW64_CONTEXT(); memset(m_pctx, 0, sizeof(WOW64_CONTEXT));
		m_pbak = new WOW64_CONTEXT(); memset(m_pbak, 0, sizeof(WOW64_CONTEXT));

		static int GPR_OFFS32[] =
		{
			offsetof(WOW64_CONTEXT, Eip),
			offsetof(WOW64_CONTEXT, Esp),
			offsetof(WOW64_CONTEXT, Ebp),
			offsetof(WOW64_CONTEXT, Eax),
			offsetof(WOW64_CONTEXT, Ebx),
			offsetof(WOW64_CONTEXT, Ecx),
			offsetof(WOW64_CONTEXT, Edx),
			offsetof(WOW64_CONTEXT, Esi),
			offsetof(WOW64_CONTEXT, Edi),
			offsetof(WOW64_CONTEXT, EFlags)
		};
		for (int i = 0; i < sizeof(GPR_OFFS32) / sizeof(int); i++)
		{
			PDWORD_PTR* pGprs = new PDWORD_PTR[2];
			pGprs[0] = (PDWORD_PTR)((PBYTE)m_pctx + GPR_OFFS32[i]);
			pGprs[1] = (PDWORD_PTR)((PBYTE)m_pbak + GPR_OFFS32[i]);
			AddItem(i, pGprs);
		}
	}
	else
	{
#ifdef _WIN64
		m_pctx = new CONTEXT(); memset(m_pctx, 0, sizeof(CONTEXT));
		m_pbak = new CONTEXT(); memset(m_pbak, 0, sizeof(CONTEXT));

		static int GPR_OFFS64[] =
		{
			offsetof(CONTEXT, Rip),
			offsetof(CONTEXT, Rsp),
			offsetof(CONTEXT, Rbp),
			offsetof(CONTEXT, Rax),
			offsetof(CONTEXT, Rbx),
			offsetof(CONTEXT, Rcx),
			offsetof(CONTEXT, Rdx),
			offsetof(CONTEXT, Rsi),
			offsetof(CONTEXT, Rdi),
			offsetof(CONTEXT, R8),
			offsetof(CONTEXT, R9),
			offsetof(CONTEXT, R10),
			offsetof(CONTEXT, R11),
			offsetof(CONTEXT, R12),
			offsetof(CONTEXT, R13),
			offsetof(CONTEXT, R14),
			offsetof(CONTEXT, R15),
			offsetof(CONTEXT, EFlags)
		};
		for (int i = 0; i < sizeof(GPR_OFFS64) / sizeof(int); i++)
		{
			PDWORD_PTR* pGprs = new PDWORD_PTR[2];
			pGprs[0] = (PDWORD_PTR)((PBYTE)m_pctx + GPR_OFFS64[i]);
			pGprs[1] = (PDWORD_PTR)((PBYTE)m_pbak + GPR_OFFS64[i]);
			AddItem(i, pGprs);
		}
#endif
	}
	m_wndView.EnableWindow();
}

void CRegstrDock::UpdateThreadCtx(DWORD dwThrId, PVOID pCtx)
{
	if (pCtx == NULL)
	{
		m_bHalt = false;
		m_wndView.SetBkColor(RGB(235, 235, 235));
		m_wndView.Invalidate();
		return;
	}

	m_bHalt = true;
	m_dwThrId = dwThrId;
	if (m_bIs32)
	{
		PWOW64_CONTEXT pcxt = (PWOW64_CONTEXT)pCtx;
		*((PWOW64_CONTEXT)m_pbak) = *((PWOW64_CONTEXT)m_pctx);
		*((PWOW64_CONTEXT)m_pctx) = *pcxt;
	}
	else
	{
		PCONTEXT pcxt = (PCONTEXT)pCtx;
		*((PCONTEXT)m_pbak) = *((PCONTEXT)m_pctx);
		*((PCONTEXT)m_pctx) = *pcxt;
	}
	m_wndView.SetBkColor(RGB(255, 255, 255));
	m_wndView.Invalidate();
}

void CRegstrDock::UninitThreadCtx()
{
	if (m_pctx != NULL)
	{
		if (m_bIs32)
			delete ((PWOW64_CONTEXT)m_pctx);
		else
			delete ((PCONTEXT)m_pctx);
		m_pctx = NULL;
	}
	if (m_pbak != NULL)
	{
		if (m_bIs32)
			delete ((PWOW64_CONTEXT)m_pbak);
		else
			delete ((PCONTEXT)m_pbak);
		m_pbak = NULL;
	}
	m_wndView.DeleteAllItems();
	m_wndView.EnableWindow(FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// CRegstrDock 메시지 처리기
int CRegstrDock::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rc; rc.SetRectEmpty();
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_EDITLABELS;
	if (!m_wndView.Create(dwViewStyle, rc, this, 2))
	{
		TRACE0("레지스터 뷰를 만들지 못했습니다.\n");
		return -1;
	}
	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP;
	m_wndView.SetExtendedStyle(m_wndView.GetExtendedStyle() | dwExStyle);
	m_wndView.SetFont(((CPEApp*)AfxGetApp())->GetFont());
	m_wndView.InsertColumn(0, _T("값"), LVCFMT_LEFT, 150);
	m_wndView.InsertColumn(1, _T("레지스터"), LVCFMT_LEFT, 100);
	m_wndView.InsertColumn(2, _T("상세"), LVCFMT_LEFT, 150);
	m_wndView.SetBkColor(RGB(220, 220, 220));
	m_wndView.EnableWindow(FALSE);
	return 0;
}

void CRegstrDock::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	if (GetSafeHwnd() == NULL)
		return;
	CRect rc; GetClientRect(rc);
	m_wndView.SetWindowPos(NULL, rc.left + 1, rc.top + 1,
		rc.Width() - 2, rc.Height() - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CRegstrDock::OnPaint()
{
	CPaintDC dc(this);
	CRect rc; m_wndView.GetWindowRect(rc);
	ScreenToClient(rc);
	rc.InflateRect(1, 1);
	dc.Draw3dRect(rc, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CRegstrDock::OnContextMenu(CWnd* pWnd, CPoint point)
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

void CRegstrDock::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndView.SetFocus();
}

void CRegstrDock::OnDestroy()
{
	CDockablePane::OnDestroy();
	UninitThreadCtx();
}

void CRegstrDock::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pnmdvi = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	PDWORD_PTR* pValPos = (PDWORD_PTR*)pnmdvi->item.lParam;

	CString sz;
	switch (pnmdvi->item.iSubItem)
	{
		case 0:	// 값
			sz = PEPlus::GetAddrForm((m_bIs32), (PBYTE)(*pValPos[0]));
			wsprintf(pnmdvi->item.pszText, sz);
		break;
		case 1:	// 메인
			if (m_bIs32)
				pnmdvi->item.pszText = (PWSTR)G_SZREG32[pnmdvi->item.iItem];
			else
				pnmdvi->item.pszText = (PWSTR)G_SZREG64[pnmdvi->item.iItem];
		break;
		case 2:
		{
			bool bMore = false;
			if (m_bIs32 && pnmdvi->item.iItem == 9)
				bMore = true;
			else if (!m_bIs32 && pnmdvi->item.iItem == 17)
				bMore = true;
			if (bMore)
			{
				static PCWSTR pszFlags[] = 
				{ L"CF", L"PF", L"AF", L"ZF", L"SF", L"IF", L"DF", L"OF" };
				static int pnFlgIdx[] = { 0, 2, 4, 6, 7, 8, 10, 11 };
				DWORD dwVal = (DWORD)(*pValPos[0]);
				for (int i = 0; i < sizeof(pnFlgIdx) / sizeof(int); i++)
				{
					if (!sz.IsEmpty()) sz.Append(L", ");
					int v = ((dwVal & (1 << pnFlgIdx[i])) != 0);
					sz.AppendFormat(L"%s:%d", pszFlags[i], v);
					wsprintf(pnmdvi->item.pszText, sz);
				}
			}
		}
		break;
	}
	*pResult = 0;
}

void CRegstrDock::OnLvnBeginLabelEdit(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pnvdi = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if (m_bIs32)
		return;

	if (pnvdi->item.mask & LVIF_TEXT)
	{
		CString sz = pnvdi->item.pszText;
		sz.Replace(L'`', L'');
		wcscpy_s(pnvdi->item.pszText, sz.GetLength() + 1, sz);
	}
	*pResult = 0;
}

void CRegstrDock::OnLvnEndLabelEdit(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pnvdi = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if (pnvdi->item.mask & LVIF_TEXT)
	{
		CString sz = pnvdi->item.pszText;
		sz.Trim();
		if (sz.IsEmpty())
			return;

		PBYTE pVal = 0;
		try
		{
			pVal = PEPlus::AddrStr2Value(m_bIs32, sz);
		}
		catch (HRESULT /*hr*/)
		{
			AfxMessageBox(L"주소 형식이 잘못되었습니다.");
			*pResult = 1;
			return;
		}

		PDWORD_PTR* pValPos = (PDWORD_PTR*)pnvdi->item.lParam;
		*((PBYTE*)pValPos[0]) = pVal;
	}
	*pResult = 0;
}

void CRegstrDock::OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pvi = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pvi->lParam != NULL)
		delete[] ((PDWORD_PTR*)pvi->lParam);
	*pResult = 0;
}

void CRegstrDock::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
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
			PDWORD_PTR* pValPos = (PDWORD_PTR*)pvcd->nmcd.lItemlParam;
			if (*pValPos[0] != *pValPos[1])
				pvcd->clrText = RGB(255, 0, 0);
			else
				pvcd->clrText = RGB(0, 0, 0);
			pvcd->clrTextBk = (m_bHalt) ? RGB(255, 255, 255) : RGB(235, 235, 235);
			*pResult = CDRF_NEWFONT;
		}
		break;
		default:
			*pResult = CDRF_DODEFAULT;
		break;
	}
}
