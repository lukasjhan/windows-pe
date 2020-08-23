#include "stdafx.h"
#include "PEFrame.h"
#include "PEDock.Project.h"
#include "Resource.h"
#include "PEApp.h"
#include "PEDoc.h"
#include "PEView.Html.h"
#include "PEView.DAsm.h"
#include "PEView.Src.h"
#include "PEView.VMem.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CProjectMgr, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CONTEXTMENU()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_NOTIFY(TVN_ITEMCHANGED, 4, OnTvnItemChangedTree)
	ON_NOTIFY(TVN_DELETEITEM, 4, OnTvnDeleteitemTree)
	ON_COMMAND(ID_PROPERTIES, OnProperties)
	ON_COMMAND(ID_OPEN, OnFileOpen)
	ON_UPDATE_COMMAND_UI(ID_OPEN, OnUpdateFileOpen)
	ON_COMMAND(IDM_VIEW_PROC_VAS, OnViewProcVAS)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_PROC_VAS, OnUpdateViewProcVAS)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProjectMgr
CProjectMgr::CProjectMgr()
{
	m_hDlls = m_hExec = NULL;
	m_pPEDoc = NULL;
}

CProjectMgr::~CProjectMgr()
{
}

void CProjectMgr::InitView(CPEDoc* pDoc)
{
	HTREEITEM hRoot = m_tvPrjMgr.InsertItem(pDoc->GetPrjName(), 0, 0);
	m_tvPrjMgr.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);
	PPRJ_NODE ppn = new PRJ_NODE(PRJ_NODE_PROJECT, 0, pDoc);
	ppn->Node = hRoot;
	m_tvPrjMgr.SetItemData(hRoot, (DWORD_PTR)ppn);

	PDBG_PROCESS pdbg = pDoc->GetDbgProcess();
	m_hExec = m_tvPrjMgr.InsertItem(pdbg->ModuleName, 1, 1, hRoot);
	ppn = new PRJ_NODE(PRJ_NODE_MODULE, 0, pdbg);
	ppn->Node = m_hExec;
	m_tvPrjMgr.SetItemData(m_hExec, (DWORD_PTR)ppn);

	m_hDlls = m_tvPrjMgr.InsertItem(L"Dlls", 3, 3, hRoot);
	ppn = new PRJ_NODE(PRJ_NODE_FOLDER, 0, pDoc->GetModuleMap());
	ppn->Node = m_hDlls;
	m_tvPrjMgr.SetItemData(m_hDlls, (DWORD_PTR)ppn);

	m_tvPrjMgr.Expand(m_hExec, TVE_EXPAND);
	m_tvPrjMgr.Expand(hRoot, TVE_EXPAND);
}

void CProjectMgr::UninitView(CPEDoc* pDoc)
{
	m_tvPrjMgr.DeleteAllItems();
	m_pPEDoc = NULL;
}

void CProjectMgr::UpdateProcess(int nOpt, PDBG_PROCESS pdp)
{
	if (nOpt == DBG_NOTI_APPEND)
	{
		m_tvPrjMgr.SetItemImage(m_hExec, 2, 2);
		if (m_tvPrjMgr.ItemHasChildren(m_hDlls))
		{
			HTREEITEM hDll = m_tvPrjMgr.GetChildItem(m_hDlls);
			while (hDll)
			{
				HTREEITEM hNext = m_tvPrjMgr.GetNextSiblingItem(hDll);
				m_tvPrjMgr.DeleteItem(hDll);
				hDll = hNext;
			}
			m_tvPrjMgr.SetItemData(m_hDlls, NULL);
		}
	}
	else
	{
		m_tvPrjMgr.SetItemImage(m_hExec, 1, 1);
		if (m_tvPrjMgr.ItemHasChildren(m_hDlls))
		{
			HTREEITEM hDll = m_tvPrjMgr.GetChildItem(m_hDlls);
			while (hDll)
			{
				m_tvPrjMgr.SetItemImage(hDll, 4, 4);
				hDll = m_tvPrjMgr.GetNextSiblingItem(hDll);
			}
		}
	}
}

void CProjectMgr::UpdateModule(int nOpt, PDBG_MODULE pdm)
{
	if (nOpt == DBG_NOTI_APPEND)
	{
		CString szName = pdm->ModuleName;
		szName.AppendFormat(L" (0x%s)", PEPlus::GetAddrForm(pdm->Is32Bit, pdm->ImageBase));
		HTREEITEM hDll = m_tvPrjMgr.InsertItem(szName, 5, 5, m_hDlls);
		PPRJ_NODE ppn = new PRJ_NODE(PRJ_NODE_MODULE, 0, pdm, hDll);
		m_tvPrjMgr.SetItemData(hDll, (DWORD_PTR)ppn);
		m_tvPrjMgr.Expand(m_hDlls, TVE_EXPAND);
	}
	else
	{
		if (m_tvPrjMgr.ItemHasChildren(m_hDlls))
		{
			HTREEITEM hDll = m_tvPrjMgr.GetChildItem(m_hDlls);
			while (hDll)
			{
				PPRJ_NODE ppn = (PPRJ_NODE)m_tvPrjMgr.GetItemData(hDll);
				if (pdm == ppn->Dbg)
				{
					m_tvPrjMgr.DeleteItem(hDll);
					break;
				}
				hDll = m_tvPrjMgr.GetNextSiblingItem(hDll);
			}
		}
	}
}

void CProjectMgr::AddChildBlock(PDBG_BLOCK pdb, PCWSTR pFncName, HTREEITEM hFunc)
{
	CString sz;
	sz.Format(L"Sub_%s (0x%08X+%d)", pFncName, pdb->BeginRVA, pdb->CodeSize);
	HTREEITEM hBlk = m_tvPrjMgr.InsertItem(sz, 13, 13, hFunc);
	PPRJ_NODE ppn = new PRJ_NODE(PRJ_NODE_FUNCTION, 0, pdb, hBlk);
	m_tvPrjMgr.SetItemData(hBlk, (DWORD_PTR)ppn);
	if (pdb->Childs == NULL)
		return;

	for (int i = 0; i < (int)pdb->Childs->size(); i++)
	{
		PDBG_BLOCK pchd = pdb->Childs->at(i);
		AddChildBlock(pchd, pFncName, hBlk);
	}
}

void CProjectMgr::AppenDAsmView(PDBG_MODULE pdm, PBYTE pHaltAddr, DWORD dwThreadId)
{
	CPEDoc* pDoc = GetPEDoc();
	if (pdm->Header == NULL || pdm->CodeSecs == NULL || pdm->FuncMap.size() == 0)
	{
		pDoc->SendCommand(TM_DISASSEMBLE, DASM_MODULE, (LPARAM)pdm);
	}

	if (pdm->FuncMap.size() > 0)
	{
		HTREEITEM hNode = NULL;
		if (pdm->IsExe)
			hNode = m_hExec;
		else
		{
			HTREEITEM hDll = m_tvPrjMgr.GetChildItem(m_hDlls);
			while (hDll)
			{
				PPRJ_NODE ppn = (PPRJ_NODE)m_tvPrjMgr.GetItemData(hDll);
				if (ppn->Dbg == pdm)
				{
					hNode = hDll;
					break;
				}
				hDll = m_tvPrjMgr.GetNextSiblingItem(hDll);
			}
		}
		if (hNode != NULL)
		{
			for (DBG_FUNC_MAP::iterator it = pdm->FuncMap.begin(); it != pdm->FuncMap.end(); it++)
			{
				PDBG_FUNC pdf = (PDBG_FUNC)it->second;
				if (pdf->Parent != NULL)
					continue;

				CString sz; sz.Format(L"%s (0x%08X+0x%X)", pdf->FuncName, pdf->BeginRVA, pdf->CodeSize);
				HTREEITEM hFunc = m_tvPrjMgr.InsertItem(sz, 13, 13, hNode);
				PPRJ_NODE ppn = new PRJ_NODE(PRJ_NODE_FUNCTION, 0, pdf, hFunc);
				m_tvPrjMgr.SetItemData(hFunc, (DWORD_PTR)ppn);

				if (pdf->Childs != NULL)
				{
					for (int i = 0; i < (int)pdf->Childs->size(); i++)
					{
						PDBG_BLOCK pdb = pdf->Childs->at(i);
						AddChildBlock(pdb, pdf->FuncName, hFunc);
					}
				}
			}
		}
	}

	DASM_VIEW_PRM ip(pdm, (DWORD)(pHaltAddr - pdm->ImageBase), dwThreadId);
	pDoc->SetInitPrm(&ip);
	CRuntimeClass* pRC = RUNTIME_CLASS(CDAsmView);
	CMDIChildWndEx* pChildFrm = ((CPEApp*)AfxGetApp())->AppendView(pDoc, IDR_PEDASM_TYPE, pRC);
	InsertViewInst(pdm->ImageBase, pChildFrm->GetActiveView());
	pChildFrm->SetWindowText(pdm->ModuleName);
}

void CProjectMgr::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
		return;

	CRect rc; GetClientRect(rc);
	int cyTlb = m_tbMain.CalcFixedLayout(FALSE, TRUE).cy;
	m_tbMain.SetWindowPos(NULL, rc.left, rc.top, rc.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_tvPrjMgr.SetWindowPos(NULL, rc.left + 1, rc.top + cyTlb + 1,
		rc.Width() - 2, rc.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}
/////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar 메시지 처리기
int CProjectMgr::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rc; rc.SetRectEmpty();
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | 
		TVS_LINESATROOT | TVS_HASBUTTONS | TVS_SHOWSELALWAYS;
	if (!m_tvPrjMgr.Create(dwViewStyle, rc, this, 4))
	{
		TRACE0("관리자 뷰를 만들지 못했습니다.\n");
		return -1; 
	}

	m_imgView.Create(IDB_DOCK_PRJMNG, 16, 0, RGB(255, 0, 255));
	m_tvPrjMgr.SetImageList(&m_imgView, LVSIL_NORMAL);
	m_tvPrjMgr.SetFont(((CPEApp*)AfxGetApp())->GetFont());

	m_tbMain.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_tbMain.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE /* 잠금 */);

	m_tbMain.SetPaneStyle(m_tbMain.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_tbMain.SetPaneStyle(m_tbMain.GetPaneStyle() & 
		~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | 
		CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_tbMain.SetOwner(this);
	m_tbMain.SetRouteCommandsViaFrame(FALSE);
	AdjustLayout();
	return 0;
}

void CProjectMgr::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CProjectMgr::OnPaint()
{
	CPaintDC dc(this);
	CRect rc;
	m_tvPrjMgr.GetWindowRect(rc);
	ScreenToClient(rc);
	rc.InflateRect(1, 1);
	dc.Draw3dRect(rc, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CProjectMgr::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTreeCtrl* pWndTree = (CTreeCtrl*)&m_tvPrjMgr;
	ASSERT_VALID(pWndTree);
	if (pWnd != pWndTree)
	{
		CDockablePane::OnContextMenu(pWnd, point);
		return;
	}

	HTREEITEM hSelItem = NULL;
	if (point != CPoint(-1, -1))
	{
		CPoint ptTree = point;
		pWndTree->ScreenToClient(&ptTree);

		UINT flags = 0;
		hSelItem = pWndTree->HitTest(ptTree, &flags);
		if (hSelItem != NULL)
			pWndTree->SelectItem(hSelItem);
	}
	pWndTree->SetFocus();
	UINT uPopResId = IDR_POPUP_PRJMGR_BACKBROUND;
	if (hSelItem != NULL)
	{
		uPopResId = IDR_POPUP_PRJMGR_DEFAULT;
		PPRJ_NODE ppn = (PPRJ_NODE)m_tvPrjMgr.GetItemData(hSelItem);
		if (ppn->Type == PRJ_NODE_MODULE)
			uPopResId = IDR_POPUP_PRJMGR_MODULE;
	}
	G_PE_APP.GetContextMenuManager()->ShowPopupMenu(uPopResId, point.x, point.y, this, TRUE);
}

void CProjectMgr::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_tvPrjMgr.SetFocus();
}

void CProjectMgr::OnTvnItemChangedTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMTVITEMCHANGE* pnvn = reinterpret_cast<NMTVITEMCHANGE*>(pNMHDR);
	if (pnvn->uStateNew)
	{
		PPRJ_NODE ppn = (PPRJ_NODE)pnvn->lParam;
		if (ppn != NULL && ppn->Type == PRJ_NODE_FUNCTION)
		{
			PDBG_DUMMY pdb = (PDBG_DUMMY)ppn->Dbg;
			CDAsmView* pView = (CDAsmView*)FindViewInst(pdb->Module->ImageBase);
			pView->ScrollDAsmLine(pdb->BeginRVA, DASM_SETPOS);
			pView->GetParentFrame()->ActivateFrame();
		}
	}
	*pResult = 0;
}

void CProjectMgr::OnTvnDeleteitemTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pntv = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	if (pntv->itemOld.lParam != NULL)
		delete ((PPRJ_NODE)pntv->itemOld.lParam);
	*pResult = 0;
}

void CProjectMgr::OnFileOpen()
{
	HTREEITEM hSelItem = m_tvPrjMgr.GetSelectedItem();
	PPRJ_NODE ppn = (PPRJ_NODE)m_tvPrjMgr.GetItemData(hSelItem);
	if (ppn == NULL || ppn->Type != PRJ_NODE_MODULE)
		return;

	PDBG_MODULE pdm = (PDBG_MODULE)ppn->Dbg;
	AppenDAsmView(pdm);
	//UINT nViewID = IDR_PESOURCE_TYPE;
	//CRuntimeClass* pRC = RUNTIME_CLASS(CSrcView);
	//if (hSelItem == m_tvPrjMgr.GetRootItem())
	//{
	//	nViewID = IDR_PEFrontierTYPE;
	//	pRC = RUNTIME_CLASS(CFrontierView);
	//}
	//else if (hSelItem == m_hExec)
	//{
	//	nViewID = IDR_PEDASM_TYPE;
	//	pRC = RUNTIME_CLASS(CDAsmView);
	//}
	//CFrameWnd* pFrame = ((CPEFrame*)AfxGetMainWnd())->GetActiveFrame();
	//CDocument* pDoc = pFrame->GetActiveDocument();
	//((CPEApp*)AfxGetApp())->AppendView(pDoc, nViewID, pRC);
}

void CProjectMgr::OnUpdateFileOpen(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;
	HTREEITEM hSelItem = m_tvPrjMgr.GetSelectedItem();
	PPRJ_NODE ppn = (PPRJ_NODE)m_tvPrjMgr.GetItemData(hSelItem);
	if (ppn != NULL && ppn->Type == PRJ_NODE_MODULE)
	{
		PDBG_MODULE pdm = (PDBG_MODULE)ppn->Dbg;
		CDAsmView* pView = (CDAsmView*)FindViewInst(pdm->ImageBase);
		bEnable = (pView == NULL);
	}
	pCmdUI->Enable(bEnable);
}

void CProjectMgr::OnViewProcVAS()
{
	HTREEITEM hSelItem = m_tvPrjMgr.GetSelectedItem();
	if (hSelItem == NULL || hSelItem != m_hExec)
		return;

	UINT nViewID = IDR_PETYPE_VIEW_VMEM;
	CRuntimeClass* pRC = RUNTIME_CLASS(CVMemView);
	CFrameWnd* pFrame = ((CPEFrame*)AfxGetMainWnd())->GetActiveFrame();
	CDocument* pDoc = pFrame->GetActiveDocument();
	((CPEApp*)AfxGetApp())->AppendView(pDoc, nViewID, pRC);
}

void CProjectMgr::OnUpdateViewProcVAS(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;
	CFrameWnd* pFrame = ((CPEFrame*)AfxGetMainWnd())->GetActiveFrame();
	CPEDoc* pDoc = (CPEDoc*)pFrame->GetActiveDocument();
	if (pDoc->UnderDebug())
	{
		bEnable = TRUE;
	}
	pCmdUI->Enable(bEnable);
}


void CProjectMgr::OnEditCut()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CProjectMgr::OnEditCopy()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CProjectMgr::OnEditClear()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
}

void CProjectMgr::OnProperties()
{
	AfxMessageBox(_T("속성...."));
}
