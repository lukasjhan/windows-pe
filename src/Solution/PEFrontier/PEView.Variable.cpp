#include "stdafx.h"
#include "MainFrm.h"
#include "PEView.Module.h"
#include "Resource.h"
#include "PEFrontier.h"
#include "MainFrm.h"
#include "PEDoc.h"


//////////////////////////////////////////////////////////////////////
// ����/�Ҹ�
//////////////////////////////////////////////////////////////////////
CModuleView::CModuleView()
{
}

CModuleView::~CModuleView()
{
}

BOOL CModuleView::AddItem(int nIndex, LPARAM pInfo)
{
	LV_ITEM	lvi;
	lvi.mask	 = LVIF_TEXT | LVIF_PARAM/* | LVIF_IMAGE*/;
	lvi.iItem	 = nIndex;
	lvi.iSubItem = 0;
	//lvi.iImage = I_IMAGECALLBACK;
	lvi.pszText	 = LPSTR_TEXTCALLBACK;
	lvi.lParam	 = (LPARAM)pInfo;
	if (m_wndView.InsertItem(&lvi) == -1)
		return FALSE;
	return TRUE;
}


BEGIN_MESSAGE_MAP(CModuleView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_GETDISPINFO, 2, OnLvnGetdispinfo)
	ON_MESSAGE(WM_DEBUG_EVENT, OnDebugEvent)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModuleView �޽��� ó����
int CModuleView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// �並 ����ϴ�.
	CRect rc; rc.SetRectEmpty();
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL;
	if (!m_wndView.Create(dwViewStyle, rc, this, 2))
	{
		TRACE0("Ŭ���� �並 ������ ���߽��ϴ�.\n");
		return -1;      // ������ ���߽��ϴ�.
	}
	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	m_wndView.SetExtendedStyle(m_wndView.GetExtendedStyle() | dwExStyle);

	//m_imgBrk.Create(IDB_DBG_BREAK, 18, 6, RGB(255, 255, 255));
	//GetListCtrl().SetImageList(&m_imgBrk, LVSIL_SMALL);
	m_wndView.SetFont(((CPEApp*)AfxGetApp())->GetFont());

	m_wndView.InsertColumn(0, _T("�̸�"), LVCFMT_LEFT, 140);
	m_wndView.InsertColumn(1, _T("���"), LVCFMT_LEFT, 250);
	m_wndView.InsertColumn(2, _T("�ּ�"), LVCFMT_LEFT, 100);
	m_wndView.InsertColumn(3, _T("��ȣ����"), LVCFMT_LEFT, 250);
	m_wndView.InsertColumn(4, _T("��"), LVCFMT_LEFT, 320);
	return 0;
}

void CModuleView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	if (GetSafeHwnd() == NULL)
		return;
	CRect rc; GetClientRect(rc);
	m_wndView.SetWindowPos(NULL, rc.left + 1, rc.top + 1,
		rc.Width() - 2, rc.Height() - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CModuleView::OnPaint()
{
	CPaintDC dc(this);
	CRect rc; m_wndView.GetWindowRect(rc);
	ScreenToClient(rc);
	rc.InflateRect(1, 1);
	dc.Draw3dRect(rc, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CModuleView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndView.SetFocus();
}

LRESULT CModuleView::OnDebugEvent(WPARAM wParam, LPARAM lParam)
{
	PDBG_DLL pdd = (PDBG_DLL)lParam;
	if (wParam == DBG_NOTI_CLEAR)
	{
		m_wndView.DeleteAllItems();
		return 0;
	}

	int nItemCnt = m_wndView.GetItemCount();
	if (wParam == DBG_NOTI_APPEND)
	{
		AddItem(nItemCnt, lParam);
	}
	else
	{
		for (int i = 0; i < nItemCnt; i++)
		{
			PDBG_DLL pvdd = (PDBG_DLL)m_wndView.GetItemData(i);
			if (pvdd->BaseOfDll == pdd->BaseOfDll)
			{
				m_wndView.DeleteItem(i);
				break;
			}
		}
	}
	return 0;
}

void CModuleView::OnContextMenu(CWnd* pWnd, CPoint point)
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
		// Ŭ���� �׸��� �����մϴ�.
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

void CModuleView::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pnmdvi = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	PDBG_DLL pdd = (PDBG_DLL)pnmdvi->item.lParam;

	switch (pnmdvi->item.iSubItem)
	{
		case 0:	// �̸�
			//_tcscpy
			wsprintf(pnmdvi->item.pszText, L"%s", pdd->ModuleName);
			break;
		case 1:	// ���
		break;
		case 2:	// �ּ�
#ifdef _WIN64
			wsprintf(pnmdvi->item.pszText, L"0x%016I64X", pdd->ModuleName);
#else
			wsprintf(pnmdvi->item.pszText, L"0x%08X", pdd->BaseOfDll);
#endif
		break;
		case 3:	// ��ȣ ����
			wsprintf(pnmdvi->item.pszText, L"%s", (pdd->DiaInterface != NULL) ? L"����" : L"����");
		break;
		case 4:	// ��
		break;
	}
	*pResult = 0;
}
