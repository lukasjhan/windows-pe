#include "stdafx.h"
#include "PEFrame.h"
#include "PEDock.Module.h"
#include "Resource.h"
#include "PEApp.h"
#include "PEFrame.h"
#include "PEDoc.h"


//////////////////////////////////////////////////////////////////////
// 생성/소멸
//////////////////////////////////////////////////////////////////////
CModuleDock::CModuleDock()
{
}

CModuleDock::~CModuleDock()
{
}

void CModuleDock::UpdateItem(int nOpt, PDBG_MODULE pdm)
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
		lvi.iImage	 = pdm->IsExe ? 0 : 1;
		lvi.pszText  = LPSTR_TEXTCALLBACK;
		lvi.lParam	 = (LPARAM)pdm;
		m_wndView.InsertItem(&lvi);
	}
	else
	{
		for (int i = 0; i < nItemCnt; i++)
		{
			PDBG_MODULE pvdd = (PDBG_MODULE)m_wndView.GetItemData(i);
			if (pvdd->ImageBase == pdm->ImageBase)
			{
				m_wndView.DeleteItem(i);
				break;
			}
		}
	}
}

BEGIN_MESSAGE_MAP(CModuleDock, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(LVN_GETDISPINFO, 2, OnLvnGetdispinfo)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CModuleDock 메시지 처리기
int CModuleDock::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	// 뷰를 만듭니다.
	CRect rc; rc.SetRectEmpty();
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL;
	if (!m_wndView.Create(dwViewStyle, rc, this, 2))
	{
		TRACE0("클래스 뷰를 만들지 못했습니다.\n");
		return -1;      // 만들지 못했습니다.
	}
	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP;
	m_wndView.SetExtendedStyle(m_wndView.GetExtendedStyle() | dwExStyle);

	m_imgDock.Create(IDB_DOCK_MODULE, 18, 2, RGB(255, 0, 255));
	m_wndView.SetImageList(&m_imgDock, LVSIL_SMALL);
	m_wndView.SetFont(((CPEApp*)AfxGetApp())->GetFont());

	m_wndView.InsertColumn(0, _T("이름"), LVCFMT_LEFT, 140);
	m_wndView.InsertColumn(1, _T("경로"), LVCFMT_LEFT, 200);
	m_wndView.InsertColumn(2, _T("시작 주소"), LVCFMT_LEFT, 150);
	m_wndView.InsertColumn(3, _T("크기"), LVCFMT_RIGHT, 150);
	m_wndView.InsertColumn(4, _T("심볼"), LVCFMT_CENTER, 45);
	m_wndView.InsertColumn(5, _T("심볼 경로"), LVCFMT_LEFT, 320);
	m_wndView.EnableWindow(FALSE);

	return 0;
}

void CModuleDock::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	if (GetSafeHwnd() == NULL)
		return;
	CRect rc; GetClientRect(rc);
	m_wndView.SetWindowPos(NULL, rc.left + 1, rc.top + 1,
		rc.Width() - 2, rc.Height() - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CModuleDock::OnPaint()
{
	CPaintDC dc(this);
	CRect rc; m_wndView.GetWindowRect(rc);
	ScreenToClient(rc);
	rc.InflateRect(1, 1);
	dc.Draw3dRect(rc, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CModuleDock::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndView.SetFocus();
}

void CModuleDock::OnContextMenu(CWnd* pWnd, CPoint point)
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

void CModuleDock::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pnmdvi = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	PDBG_MODULE pdm = (PDBG_MODULE)pnmdvi->item.lParam;

	switch (pnmdvi->item.iSubItem)
	{
		case 0:	// 이름
			_tcscpy_s(pnmdvi->item.pszText, pdm->ModuleName.GetLength() + 1, pdm->ModuleName);
		break;

		case 1:	// 경로
			_tcscpy_s(pnmdvi->item.pszText, pdm->ModulePath.GetLength() + 1, pdm->ModulePath);
		break;

		case 2:	// 시작 주소
			wsprintf(pnmdvi->item.pszText, L"0x%s", PEPlus::GetAddrForm(pdm->Is32Bit, pdm->ImageBase));
		break;

		case 3:	// 크기
			wsprintf(pnmdvi->item.pszText, L"0x%X(%d)", pdm->ImageSize, pdm->ImageSize);
			break;

		case 4:	// 기호 파일
			wsprintf(pnmdvi->item.pszText, L"%s", (pdm->DiaRef != NULL) ? L"있음" : L"없음");
		break;

		case 5:	// 상세
			if (pdm->DiaRef != NULL)
			{
				_tcscpy_s(pnmdvi->item.pszText,
					pdm->DiaRef->PDB_PATH.GetLength() + 1, pdm->DiaRef->PDB_PATH);
			}
		break;
	}
	*pResult = 0;
}
