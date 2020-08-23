#include "stdafx.h"
#include "PEFrame.h"
#include "PEDock.BreakPnt.h"
#include "Resource.h"
#include "PEApp.h"
#include "PEDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CBreakDock, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(LVN_GETDISPINFO, 4, OnLvnGetdispinfo)
	ON_NOTIFY(NM_CLICK, 4, OnNMClick)
	ON_NOTIFY(NM_DBLCLK, 4, OnNMDblclk)
	ON_NOTIFY(LVN_BEGINLABELEDIT, 4, OnLvnBeginLabelEdit)
	ON_NOTIFY(LVN_ENDLABELEDIT, 4, OnLvnEndLabelEdit)
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_COMMAND(ID_OPEN, OnFileOpen)
	ON_COMMAND(ID_OPEN_WITH, OnFileOpenWith)
	ON_COMMAND(IDM_VIEW_PROC_VAS, OnDummyCompile)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBreakDock
CBreakDock::CBreakDock()
{
	m_nCurItem = -1;
}

CBreakDock::~CBreakDock()
{
}

BOOL CBreakDock::AddItem(int nIndex, LPARAM pInfo)
{
	LV_ITEM	lvi;
	lvi.mask	 = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvi.iItem	 = nIndex;
	lvi.iSubItem = 0;
	lvi.iImage	 = I_IMAGECALLBACK;
	lvi.pszText	 = LPSTR_TEXTCALLBACK;
	lvi.lParam	 = (LPARAM)pInfo;
	if (m_wndView.InsertItem(&lvi) == -1)
		return FALSE;
	m_wndView.SetCheck(nIndex);
	return TRUE;
}

void CBreakDock::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
		return;

	CRect rc; GetClientRect(rc);
	int cyTlb = m_tbMain.CalcFixedLayout(FALSE, TRUE).cy;
	m_tbMain.SetWindowPos(NULL, rc.left, rc.top, 
		rc.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndView.SetWindowPos(NULL, rc.left + 1, rc.top + cyTlb + 1, 
		rc.Width() - 2, rc.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CBreakDock::InitBreakPoint(PDBG_NLBRK_MAP pNlBrks)
{
	int nIdx = 0;
	for (DBG_NLBRK_MAP::iterator it = pNlBrks->begin(); it != pNlBrks->end(); it++)
	{
		for (DBG_MODBRK_MAP::iterator ib = it->second->begin(); ib != it->second->end(); ib++)
		{
			if (!ib->second->IsTemp)
				AddItem(nIdx, (LPARAM)ib->second);
			nIdx++;
		}
	}
}

void CBreakDock::UpdateBreakPoint(int nOpt, PBYTE pBPAddr, PDBG_MODULE pdm)
{
	PDBG_BRKPNT pdb = NULL;
	CPEFrame* pMain = ((CPEFrame*)GetParentOwner());
	CPEDoc* pDoc = (CPEDoc*)pMain->GetActiveFrame()->GetActiveDocument();
	int nItemCnt = m_wndView.GetItemCount();
	if (nOpt == DBG_BP_CREATE)
	{
		PVOID arPrms[3] = { pBPAddr, pdm, 0 };
		pDoc->SendCommand(TM_BREAK_EVENT, nOpt, (LPARAM)arPrms);
		pdb = (PDBG_BRKPNT)arPrms[2];
		AddItem(nItemCnt, (LPARAM)pdb);
	}
	if (nOpt == DBG_BP_REMOVE)
	{
		int nIdx = 0;
		for (; nIdx < nItemCnt; nIdx++)
		{
			PDBG_BRKPNT pdb2 = (PDBG_BRKPNT)m_wndView.GetItemData(nIdx);
			if (pdb2->HaltAddr() == pBPAddr)
			{
				pdb = pdb2;
				break;
			}
		}
		if (pdb != NULL)
		{
			pDoc->SendCommand(TM_BREAK_EVENT, nOpt, (LPARAM)pdb);
			m_wndView.DeleteItem(nIdx);
		}
	}
}

void CBreakDock::UpdateBreakItem(PBYTE pBPAddr)
{
	int nItemCnt = m_wndView.GetItemCount();
	for (int i = 0; i < nItemCnt; i++)
	{
		PDBG_BRKPNT pdb = (PDBG_BRKPNT)m_wndView.GetItemData(i);
		if (pBPAddr == pdb->HaltAddr())
		{
			if (m_nCurItem >= 0)
			{
				int nOldItem = m_nCurItem;
				m_nCurItem = -1;
				m_wndView.Update(nOldItem);
			}
			m_nCurItem = i;
			m_wndView.Update(m_nCurItem);
			m_wndView.EnsureVisible(m_nCurItem, FALSE);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar 메시지 처리기
int CBreakDock::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rc; rc.SetRectEmpty();
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN | LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_EDITLABELS;
	if (!m_wndView.Create(dwViewStyle, rc, this, 4))
	{
		TRACE0("중단점 뷰를 만들지 못했습니다.\n");
		return -1;
	}
	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | 
		LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_INFOTIP;
	m_wndView.SetExtendedStyle(m_wndView.GetExtendedStyle() | dwExStyle);

	CBitmap bmp; bmp.LoadBitmap(IDR_DOCK_BREAKPOINT);
	m_imgView.Create(18, 18, ILC_MASK | ILC_COLOR32, 0, 0);
	m_imgView.Add(&bmp, RGB(230, 231, 232));
	m_wndView.SetImageList(&m_imgView, LVSIL_SMALL);

	m_wndView.SetFont(((CPEApp*)AfxGetApp())->GetFont());
	m_wndView.InsertColumn(0, _T("레이블"), LVCFMT_LEFT, 80);
	m_wndView.InsertColumn(1, _T("주소"), LVCFMT_LEFT, 160);
	m_wndView.InsertColumn(2, _T("RVA"), LVCFMT_LEFT, 100);
	m_wndView.InsertColumn(3, _T("모듈명"), LVCFMT_LEFT, 100);
	m_wndView.InsertColumn(4, _T("조건"), LVCFMT_LEFT, 100);

	// 뷰 이미지를 로드합니다.
	m_FileViewImages.Create(IDB_FILE_VIEW, 16, 0, RGB(255, 0, 255));
	m_wndView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);

	m_tbMain.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_tbMain.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* 잠금 */);

	OnChangeVisualStyle();

	m_tbMain.SetPaneStyle(m_tbMain.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_tbMain.SetPaneStyle(m_tbMain.GetPaneStyle() & 
		~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | 
		CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_tbMain.SetOwner(this);
	m_tbMain.SetRouteCommandsViaFrame(FALSE);
	AdjustLayout();

	return 0;
}

void CBreakDock::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CBreakDock::OnContextMenu(CWnd* pWnd, CPoint point)
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
	G_PE_APP.GetContextMenuManager()->
		ShowPopupMenu(IDR_POPUP_PRJMGR_DEFAULT, point.x, point.y, this, TRUE);
}

void CBreakDock::OnPaint()
{
	CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

	CRect rc;
	m_wndView.GetWindowRect(rc);
	ScreenToClient(rc);
	rc.InflateRect(1, 1);
	dc.Draw3dRect(rc, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CBreakDock::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndView.SetFocus();
}

void CBreakDock::OnChangeVisualStyle()
{
	m_tbMain.CleanUpLockedImages();
	m_tbMain.LoadBitmap(IDB_EXPLORER_24, 0, 0, TRUE);

	m_FileViewImages.DeleteImageList();
	CBitmap bmp;
	if (!bmp.LoadBitmap(IDB_FILE_VIEW_24))
	{
		TRACE(_T("비트맵을 로드할 수 없습니다.n"));
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);
	m_FileViewImages.Create(16, bmpObj.bmHeight, ILC_COLOR24 | ILC_MASK, 0, 0);
	m_FileViewImages.Add(&bmp, RGB(255, 0, 255));
	m_wndView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);
}

void CBreakDock::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pnmdv = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	PDBG_BRKPNT pdb = (PDBG_BRKPNT)pnmdv->item.lParam;

	if (pnmdv->item.mask & LVIF_IMAGE)
	{
		if (!pdb->Loaded)
			pnmdv->item.iImage = BPD_BP_NL;
		else
		{
			if (pnmdv->item.iItem == m_nCurItem)
				pnmdv->item.iImage = BPD_BP_CUR;
			else
			{
				if (pdb->Active)
					pnmdv->item.iImage = (pdb->HaltIf.IsEmpty() ? BPD_BP : BPD_CONBP);
				else
					pnmdv->item.iImage = BPD_DISBP;
			}
		}
	}

	if (pnmdv->item.mask & LVIF_TEXT)
	{
		switch (pnmdv->item.iSubItem)
		{
			case 0:	// 라벨
				pnmdv->item.pszText = (PWSTR)(PCWSTR)pdb->Label;
			break;
			case 1:	// 주소
				if (pdb->Loaded)
					wsprintf(pnmdv->item.pszText, PEPlus::GetAddrForm
						(pdb->Module->Is32Bit, pdb->Module->ImageBase + pdb->HaltRVA));
				else
				{
					PWSTR pPos = wcsrchr(pdb->ModPath, L'\\');
					if (pPos == NULL) pPos = pdb->ModPath;
					else pPos++;
					wsprintf(pnmdv->item.pszText, L"%s!0x%08X", pPos, pdb->HaltRVA);
				}
			break;
			case 2:	// RVA
				wsprintf(pnmdv->item.pszText, L"0x%08X", pdb->HaltRVA);
			break;
			case 3:	// 모듈명
				if (pdb->Loaded)
					pnmdv->item.pszText = (PWSTR)(PCWSTR)pdb->Module->ModuleName;
				else
					pnmdv->item.pszText = (PWSTR)(PCWSTR)pdb->ModPath;
			break;
			case 4:	// 조건
				pnmdv->item.pszText = (PWSTR)(PCWSTR)pdb->HaltIf;
			break;
		}
	}
	*pResult = 0;
}

void CBreakDock::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pia = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint point(pia->ptAction);

	UINT uFlags = 0;
	int nItem = m_wndView.HitTest(point, &uFlags);
	if (nItem < 0)
		return;

	if (uFlags == LVHT_ONITEMSTATEICON)
	{
		PDBG_BRKPNT pdb = (PDBG_BRKPNT)m_wndView.GetItemData(nItem);
		pdb->Active = !m_wndView.GetCheck(nItem);

		if (pdb->Loaded)
		{
			CPEFrame* pMain = ((CPEFrame*)GetParentOwner());
			CPEDoc* pDoc = (CPEDoc*)pMain->GetActiveFrame()->GetActiveDocument();
			WPARAM nOpt = (pdb->Active) ? DBG_BP_SET : DBG_BP_CLEAR;
			pDoc->SendCommand(TM_BREAK_EVENT, nOpt, (LPARAM)pdb);
			pMain->UpdateDAsmItem(pdb->Module, pdb->HaltRVA);
		}
	}
	*pResult = 0;
}

void CBreakDock::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pia = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint point(pia->ptAction);

	UINT uFlags = 0;
	int nItem = m_wndView.HitTest(point, &uFlags);
	if (nItem < 0)
		return;

	PDBG_BRKPNT pdb = (PDBG_BRKPNT)m_wndView.GetItemData(nItem);
	if (!pdb->Loaded)
		return;

	CPEFrame* pMain = ((CPEFrame*)GetParentOwner());
	if (uFlags == LVHT_ONITEMSTATEICON)
	{
		pdb->Active = !m_wndView.GetCheck(nItem);
		CPEDoc* pDoc = (CPEDoc*)pMain->GetActiveFrame()->GetActiveDocument();
		WPARAM nOpt = (pdb->Active) ? DBG_BP_SET : DBG_BP_CLEAR;
		pDoc->SendCommand(TM_BREAK_EVENT, nOpt, (LPARAM)pdb);
		pMain->UpdateDAsmItem(pdb->Module, pdb->HaltRVA);
	}
	else if (uFlags & LVHT_ONITEMLABEL)
		pMain->UpdateDAsmView(pdb->Module, pdb->HaltRVA);
	*pResult = 0;
}

void CBreakDock::OnLvnBeginLabelEdit(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pnvdi = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if (pnvdi->item.mask & LVIF_TEXT)
	{
		CString sz = pnvdi->item.pszText;
		wcscpy_s(pnvdi->item.pszText, sz.GetLength() + 1, sz);
	}
	*pResult = 0;
}

void CBreakDock::OnLvnEndLabelEdit(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pnvdi = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if (pnvdi->item.mask & LVIF_TEXT)
	{
		CString sz = pnvdi->item.pszText;
		sz.Trim();
		if (sz.IsEmpty())
			return;

		PDBG_BRKPNT pdb = (PDBG_BRKPNT)pnvdi->item.lParam;
		if (pdb->Label == sz)
			return;
		pdb->Label = sz;
		CPEFrame* pMain = ((CPEFrame*)GetParentOwner());
		pMain->GetActiveFrame()->GetActiveDocument()->SetModifiedFlag();
	}
	*pResult = 0;
}

void CBreakDock::OnProperties()
{
	AfxMessageBox(_T("속성...."));

}

void CBreakDock::OnFileOpen()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CBreakDock::OnFileOpenWith()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CBreakDock::OnDummyCompile()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CBreakDock::OnEditCut()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CBreakDock::OnEditCopy()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CBreakDock::OnEditClear()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}
