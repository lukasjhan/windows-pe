#include "stdafx.h"
#include "PEFrame.h"
#include "PEDock.BookMark.h"
#include "Resource.h"
#include "PEApp.h"
#include "PEDoc.h"
#include "PEDlg.SimpleInput.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CBookMarkDock, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(LVN_GETDISPINFO, 4, OnLvnGetdispinfo)
	ON_NOTIFY(LVN_ENDLABELEDIT, 4, OnLvnEndLabelEdit)
	ON_NOTIFY(NM_CLICK, 4, OnNMClick)
	ON_NOTIFY(NM_DBLCLK, 4, OnNMDblclk)
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_COMMAND(ID_OPEN, OnFileOpen)
	ON_COMMAND(ID_OPEN_WITH, OnFileOpenWith)
	ON_COMMAND(IDM_VIEW_PROC_VAS, OnDummyCompile)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CBookMarkDock
CBookMarkDock::CBookMarkDock()
{
}

CBookMarkDock::~CBookMarkDock()
{
}

BOOL CBookMarkDock::AddItem(int nIndex, LPARAM pInfo)
{
	LV_ITEM	lvi;
	lvi.mask = LVIF_TEXT | LVIF_PARAM;
	lvi.iItem = nIndex;
	lvi.iSubItem = 0;
	lvi.pszText = LPSTR_TEXTCALLBACK;
	lvi.lParam = (LPARAM)pInfo;
	if (m_wndView.InsertItem(&lvi) == -1)
		return FALSE;
	m_wndView.SetCheck(nIndex);
	return TRUE;
}

void CBookMarkDock::AdjustLayout()
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

void CBookMarkDock::InitBookMarks(PDBG_NLBM_MAP pNlBms)
{
	int nIdx = 0;
	for (DBG_NLBM_MAP::iterator it = pNlBms->begin(); it != pNlBms->end(); it++)
	{
		for (DBG_MODBM_MAP::iterator ib = it->second->begin(); ib != it->second->end(); ib++)
		{
			AddItem(nIdx, (LPARAM)ib->second);
			nIdx++;
		}
	}
}

PDBG_BKMARK CBookMarkDock::NewBookMark(PBYTE pBMAddr, PDBG_MODULE pdm)
{
	PDBG_BKMARK pdb = new DBG_BKMARK((DWORD)(pBMAddr - pdm->ImageBase), pdm);

	CPEFrame* pMain = ((CPEFrame*)GetParentOwner());
	CPEDoc* pDoc = (CPEDoc*)pMain->GetActiveFrame()->GetActiveDocument();

	PDBG_BKMARK_MAP pbm = pDoc->GetBMarkMap();
	pbm->insert(std::make_pair(pdb->MarkAddr(), pdb));
	AddItem(m_wndView.GetItemCount(), (LPARAM)pdb);
	return pdb;
}

void CBookMarkDock::UpdateBookMark(int nOpt, PDBG_BKMARK pdb)
{
	CPEFrame* pMain = ((CPEFrame*)GetParentOwner());
	CPEDoc* pDoc = (CPEDoc*)pMain->GetActiveFrame()->GetActiveDocument();
	if (nOpt == DBG_BP_REMOVE)
	{
		int nItemCnt = m_wndView.GetItemCount();
		for (int i = 0; i < nItemCnt; i++)
		{
			PDBG_BKMARK pdb = (PDBG_BKMARK)m_wndView.GetItemData(i);
			if (pdb->MarkRVA == pdb->MarkRVA)
			{
				m_wndView.DeleteItem(i);
				break;
			}
		}
		PDBG_BKMARK_MAP pbm = pDoc->GetBMarkMap();
		pbm->erase(pdb->MarkAddr());
	}
}
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar 메시지 처리기
int CBookMarkDock::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rc; rc.SetRectEmpty();
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | 
		LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_EDITLABELS;
	if (!m_wndView.Create(dwViewStyle, rc, this, 4))
	{
		TRACE0("중단점 뷰를 만들지 못했습니다.\n");
		return -1;
	}
	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP;
	m_wndView.SetExtendedStyle(m_wndView.GetExtendedStyle() | dwExStyle);

	CBitmap bmp; bmp.LoadBitmap(IDR_DOCK_BOOKMARK);
	m_imgView.Create(18, 18, ILC_MASK | ILC_COLOR32, 0, 0);
	m_imgView.Add(&bmp, RGB(230, 231, 232));
	m_wndView.SetImageList(&m_imgView, LVSIL_SMALL);

	m_wndView.SetFont(((CPEApp*)AfxGetApp())->GetFont());
	m_wndView.InsertColumn(0, _T("레이블"), LVCFMT_LEFT, 150);
	m_wndView.InsertColumn(1, _T("주소"), LVCFMT_LEFT, 150);
	m_wndView.InsertColumn(2, _T("RVA"), LVCFMT_LEFT, 120);
	m_wndView.InsertColumn(3, _T("모듈명"), LVCFMT_LEFT, 120);
	m_wndView.InsertColumn(4, _T("설명"), LVCFMT_LEFT, 200);

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

void CBookMarkDock::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CBookMarkDock::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*) &m_wndView;
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

void CBookMarkDock::OnPaint()
{
	CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

	CRect rectTree;
	m_wndView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CBookMarkDock::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndView.SetFocus();
}

void CBookMarkDock::OnChangeVisualStyle()
{
	m_tbMain.CleanUpLockedImages();
	m_tbMain.LoadBitmap(G_PE_APP.m_bHiColorIcons ? IDB_EXPLORER_24 : IDR_EXPLORER, 0, 0, TRUE);

	m_FileViewImages.DeleteImageList();

	UINT uiBmpId = G_PE_APP.m_bHiColorIcons ? IDB_FILE_VIEW_24 : IDB_FILE_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(_T("비트맵을 로드할 수 없습니다. %x\n"), uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (G_PE_APP.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_FileViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_FileViewImages.Add(&bmp, RGB(255, 0, 255));

	m_wndView.SetImageList(&m_FileViewImages, TVSIL_NORMAL);
}

void CBookMarkDock::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pnmdv = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	PDBG_BKMARK pdb = (PDBG_BKMARK)pnmdv->item.lParam;

	switch (pnmdv->item.iSubItem)
	{
		case 0:	// 라벨
			pnmdv->item.pszText = (PWSTR)(PCWSTR)pdb->Label;
		break;
		case 1:	// 주소
			if (pdb->Loaded)
				wsprintf(pnmdv->item.pszText, PEPlus::GetAddrForm
				(pdb->Module->Is32Bit, pdb->Module->ImageBase + pdb->MarkRVA));
			else
			{
				PWSTR pPos = wcsrchr(pdb->ModPath, L'\\');
				if (pPos == NULL) pPos = pdb->ModPath;
				else pPos++;
				wsprintf(pnmdv->item.pszText, L"%s!0x%08X", pPos, pdb->MarkRVA);
			}
		break;
		case 2:	// RVA
			wsprintf(pnmdv->item.pszText, L"0x%08X", pdb->MarkRVA);
		break;
		case 3:	// 모듈명
			if (pdb->Loaded)
				pnmdv->item.pszText = (PWSTR)(PCWSTR)pdb->Module->ModuleName;
			else
				pnmdv->item.pszText = (PWSTR)(PCWSTR)pdb->ModPath;
			break;
		case 4:	// 설명
			pnmdv->item.pszText = (PWSTR)(PCWSTR)pdb->Desc;
		break;
	}
	*pResult = 0;
}

void CBookMarkDock::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pia = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint point(pia->ptAction);

	UINT uFlags = 0;
	int nItem = m_wndView.HitTest(point, &uFlags);
	if (nItem < 0)
		return;
	*pResult = 0;
}

void CBookMarkDock::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pia = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint point(pia->ptAction);

	UINT uFlags = 0;
	int nItem = m_wndView.HitTest(point, &uFlags);
	if (nItem < 0)
		return;

	if (uFlags & LVHT_ONITEMLABEL)
	{
		PDBG_BKMARK pdb = (PDBG_BKMARK)m_wndView.GetItemData(nItem);
		if (pia->iSubItem == 4)
		{
			CSimpleInput si(L"책갈피 - 상세 설명", pdb->Desc);
			if (si.DoModal() == IDOK)
			{
				pdb->Desc = si.m_szInput;
				m_wndView.Update(nItem);
			}
		}
		else
		{
			CPEFrame* pMain = (CPEFrame*)GetParentOwner();
			pMain->UpdateDAsmView(pdb->Module, pdb->MarkRVA);
		}
	}
	*pResult = 0;
}

void CBookMarkDock::OnLvnEndLabelEdit(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pnvdi = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if (pnvdi->item.mask & LVIF_TEXT)
	{
		CString sz = pnvdi->item.pszText;
		sz.Trim();
		if (sz.IsEmpty())
			return;

		PDBG_BKMARK pbm = (PDBG_BKMARK)pnvdi->item.lParam;
		pbm->Label = sz;
	}
	*pResult = 0;
}


void CBookMarkDock::OnEditCopy()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CBookMarkDock::OnEditClear()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CBookMarkDock::OnProperties()
{
	AfxMessageBox(_T("속성...."));

}

void CBookMarkDock::OnFileOpen()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CBookMarkDock::OnFileOpenWith()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CBookMarkDock::OnDummyCompile()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CBookMarkDock::OnEditCut()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}
