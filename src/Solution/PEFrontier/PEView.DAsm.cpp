#include "stdafx.h"
#ifndef SHARED_HANDLERS
#	include "PEApp.h"
#endif
#include "PEDoc.h"
#include "PEView.DAsm.h"
#include "PEFrame.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


// CDAsmView
IMPLEMENT_DYNCREATE(CDAsmView, CListView)
BEGIN_MESSAGE_MAP(CDAsmView, CListView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
	ON_WM_DESTROY()
	ON_NOTIFY_REFLECT(LVN_DELETEITEM, OnLvnDeleteitem)
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, OnLvnGetdispinfo)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnNMCustomdraw)
	ON_NOTIFY_REFLECT(NM_CLICK, OnNMClick)
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnNMDblclk)
	ON_NOTIFY_REFLECT(NM_RCLICK, OnNMRClick)
	ON_NOTIFY_REFLECT(LVN_HOTTRACK, OnLvnHotTrack)
	ON_COMMAND(IDM_DEBUG_RUN2CURSOR, OnDebugRun2Cursor)
	ON_UPDATE_COMMAND_UI(IDM_DEBUG_RUN2CURSOR, OnUpdateDebugRun2Cursor)
	ON_COMMAND(IDM_DEBUG_GOTO_CURBP, OnDebugGotoCurBP)
	ON_UPDATE_COMMAND_UI(IDM_DEBUG_GOTO_CURBP, OnUpdateDebugGotoBP)
	ON_COMMAND(IDM_EXPLR_GO_CURPOS, OnExplorGotoCurPos)
	ON_UPDATE_COMMAND_UI(IDM_EXPLR_GO_CURPOS, OnUpdateExplorGotoCurPos)
	ON_COMMAND(IDM_EXPLR_GO_ENTPTR, OnExplorGotoEntryPoint)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(IDM_DEBUG_SAVE_ASM, OnDebugSaveAsm)
	ON_COMMAND(IDM_DEBUG_SET_BREAK, OnDebugSetBreak)
	ON_UPDATE_COMMAND_UI(IDM_DEBUG_SET_BREAK, OnUpdateDebugSetBreak)
END_MESSAGE_MAP()


// CDAsmView 생성/소멸
CDAsmView::CDAsmView()
{
	m_pdm		= NULL;
	m_dwDbgThId	= 0;
	m_nBrkItem	= m_nCurItem = -1;

	m_bShowRva  = false;
	m_bShowCode = true;
}

CDAsmView::~CDAsmView()
{
}

BOOL CDAsmView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= (LVS_REPORT | LVS_SHOWSELALWAYS);
	return CListView::PreCreateWindow(cs);
}

void CDAsmView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP;
	GetListCtrl().SetExtendedStyle(GetListCtrl().GetExtendedStyle() | dwExStyle);

	CBitmap bmp; bmp.LoadBitmap(IDB_DBG_BREAK);
	m_imgView.Create(18, 18, ILC_MASK | ILC_COLOR32, 0, 0);
	m_imgView.Add(&bmp, RGB(255, 0, 255));
	GetListCtrl().SetImageList(&m_imgView, LVSIL_SMALL);
	GetListCtrl().SetFont(((CPEApp*)AfxGetApp())->GetFont());

	GetListCtrl().InsertColumn(0, _T("주소"), LVCFMT_LEFT, 160);
	GetListCtrl().InsertColumn(1, _T("코드"), LVCFMT_LEFT, 250);
	GetListCtrl().InsertColumn(2, _T("니모닉"), LVCFMT_LEFT, 100);
	GetListCtrl().InsertColumn(3, _T("오퍼랜드"), LVCFMT_LEFT, 250);
	GetListCtrl().InsertColumn(4, _T("상세"), LVCFMT_LEFT, 320);

	CPEDoc* pDoc = GetDocument();
	PDASM_VIEW_PRM pdvp = (PDASM_VIEW_PRM)pDoc->GetInitPrm();
	m_pdm = pdvp->Module;
	ASSERT(m_pdm != NULL);
	m_dwDbgThId = pdvp->ThreadId;

	for (int i = 0; i < m_pdm->CBaseCnt; i++)
		InsertItem(i, false, m_pdm->CBaseList[i]);

	ScrollDAsmLine(pdvp->StartRva, (m_dwDbgThId) ? DASM_BRKPTR : DASM_SETPOS);
	GetListCtrl().SetFocus();
}

void CDAsmView::OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/)
{
	if (m_nBrkItem >= 0)
	{
		PLINE_INFO pci = (PLINE_INFO)GetListCtrl().GetItemData(m_nBrkItem);
		PDBG_DASM pasm = (pci->IsAsm) ?
			(PDBG_DASM)pci->Tag : ((PDBG_THUNK)pci->Tag)->AsmCode;
		if (pasm->HasBPoint)
		{
			CPEDoc* pDoc = GetDocument();
			if (pDoc->BreakEntPos())
			{
				if (pDoc->GetDbgProcess() == m_pdm && 
					pDoc->GetDbgProcess()->EntryPoint == pasm->Code._offset)
					pasm->HasBPoint = 0;
			}
			if (pDoc->BreakThrStart())
			{
				PBYTE pTrgAddr = m_pdm->ImageBase + pasm->Code._offset;
				if (pDoc->GetRtlStartUp() == pTrgAddr)
					pasm->HasBPoint = 0;
			}
		}

		int nOldItem = m_nBrkItem;
		m_nBrkItem = -1;
		GetListCtrl().Update(nOldItem);
	}
	m_dwDbgThId  = 0;
}

// CDAsmView 진단
#ifdef _DEBUG
void CDAsmView::AssertValid() const
{
	CListView::AssertValid();
}

void CDAsmView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CPEDoc* CDAsmView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPEDoc)));
	return (CPEDoc*)m_pDocument;
}
#endif //_DEBUG


// CDAsmView 메시지 처리기
void CDAsmView::OnDestroy()
{
	CListView::OnDestroy();
	((CPEFrame*)AfxGetMainWnd())->GetProjectMgr()->RemoveViewInst(m_pdm->ImageBase);
}

void CDAsmView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CDAsmView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	G_PE_APP.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

void CDAsmView::OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pvi = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if (pvi->lParam != NULL)
		delete ((PLINE_INFO)pvi->lParam);
	*pResult = 0;
}

void CDAsmView::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
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
		case (CDDS_ITEMPREPAINT | CDDS_SUBITEM):
		{
			PLINE_INFO pci = PLINE_INFO(pvcd->nmcd.lItemlParam);
			if (!pci->IsAsm && ((PDBG_CBASE)pci->Tag)->CodeType != CT_THUNK)
			{
				PDBG_CBASE pdc = (PDBG_CBASE)pci->Tag;
				pvcd->clrText = RGB(255, 255, 255);
				switch (pdc->CodeType)
				{
					case CT_FUNC	: pvcd->clrTextBk = RGB(20, 108, 20); break;
					case CT_BLOCK	: pvcd->clrTextBk = RGB(60, 148, 60); break;
					case CT_DUMMY	: pvcd->clrTextBk = RGB(150, 150, 150); 
									  pvcd->clrText = RGB(220, 220, 220); break;
					case CT_THUNK	: pvcd->clrTextBk = RGB(255, 100, 255); 
									  pvcd->clrText = RGB(255, 0, 0); break;
					default			: pvcd->clrTextBk = RGB(255, 255, 255); 
									  pvcd->clrText = RGB(0, 0, 0); break;
				}
			}
			else
			{
				PDBG_DASM pasm = (pci->IsAsm) ? 
					(PDBG_DASM)pci->Tag : ((PDBG_THUNK)pci->Tag)->AsmCode;
				DECODED_INS& di = pasm->Code;
				pvcd->clrTextBk = RGB(240, 255, 240);
				if (di._opCode == OP_IID::OI_JMP ||
					(di._opCode >= OP_IID::OI_JO && di._opCode <= OP_IID::OI_JNLE))
					pvcd->clrText = RGB(255, 0, 0);
				else if (di._opCode == OP_IID::OI_CALL)
					pvcd->clrText = RGB(0, 0, 255);
				else if (di._opCode == OP_IID::OI_RET)
					pvcd->clrText = RGB(0, 150, 0);
				else if (di._opCode == OP_IID::OI_LEA)
					pvcd->clrText = RGB(150, 150, 0);
				else if (di._opCode == OP_IID::OI_INT_3)
					pvcd->clrText = RGB(128, 128, 128);
				else if (di._opCode == OP_IID::OI_NOP)
					pvcd->clrText = RGB(128, 128, 120);
				else if (di._opCode == OP_IID::OI_UNKNOWN)
				{
					pvcd->clrText = RGB(190, 190, 190);
					pvcd->clrTextBk = RGB(150, 0, 0);
				}
			}
			*pResult = CDRF_NEWFONT;
		}
		break;
		default:
			*pResult = CDRF_DODEFAULT;
		break;
	}
}

void CDAsmView::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO* pnmdv = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	PLINE_INFO pci = (PLINE_INFO)pnmdv->item.lParam;

	if (pnmdv->item.mask & LVIF_IMAGE)
		pnmdv->item.iImage = GetImageIndex(pci, pnmdv->item.iItem);

	if (pnmdv->item.mask & LVIF_TEXT)
	{
		CString sz;
		if (!pci->IsAsm && ((PDBG_CBASE)pci->Tag)->CodeType != CT_THUNK)
		{
			GetDispCodesStr((PDBG_CBASE)pci->Tag, pnmdv->item.iSubItem, sz);
		}
		else
		{
			PDBG_DASM pasm = (pci->IsAsm) ?
				(PDBG_DASM)pci->Tag : ((PDBG_THUNK)pci->Tag)->AsmCode;
			GetDispDAsmStr(pasm, pnmdv->item.iSubItem, sz);
		}
		if (!sz.IsEmpty())
		{
			if (sz.GetLength() > pnmdv->item.cchTextMax - 1)
				sz = sz.Left(pnmdv->item.cchTextMax - 1);
			wcscpy_s(pnmdv->item.pszText, pnmdv->item.cchTextMax, sz);
		}
	}
	*pResult = 0;
}

void CDAsmView::OnNMClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pia = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint point(pia->ptAction);
	UINT uFlags = 0;
	int nItem = GetListCtrl().HitTest(point, &uFlags);
	if (nItem < 0)
		return;

	if (uFlags == LVHT_ONITEMICON)
	{
		PLINE_INFO pci = (PLINE_INFO)GetListCtrl().GetItemData(nItem);
		if (pci->IsAsm || ((PDBG_CBASE)pci->Tag)->CodeType == CT_THUNK)
		{
			PDBG_DASM pasm = (pci->IsAsm) ?
				(PDBG_DASM)pci->Tag : ((PDBG_THUNK)pci->Tag)->AsmCode;
			PBYTE pTrgAddr = m_pdm->ImageBase + pasm->Code._offset;
			CPEFrame* pFrame = (CPEFrame*)AfxGetMainWnd();

			if (pia->uKeyFlags & LVKF_CONTROL)
			{
				PDBG_BKMARK_MAP pbm = GetDocument()->GetBMarkMap();
				DBG_BKMARK_MAP::iterator id = pbm->find(pTrgAddr);
				pasm->HasBMark = (id == pbm->end());
				if (id == pbm->end())
					pFrame->GetBookMarkView()->NewBookMark(pTrgAddr, m_pdm);
				else
					pFrame->GetBookMarkView()->UpdateBookMark(DBG_BP_REMOVE, id->second);
			}
			else
			{
				int nBrkOpt = (pasm->HasBPoint) ? DBG_BP_REMOVE : DBG_BP_CREATE;
				pFrame->GetBreakView()->UpdateBreakPoint(nBrkOpt, pTrgAddr, m_pdm);
				pasm->HasBPoint = !pasm->HasBPoint;
			}
			GetListCtrl().Update(nItem);
			GetDocument()->SetModifiedFlag();
		}
		else
		{
			if (((PDBG_CBASE)pci->Tag)->CodeType == CT_NONE)
				return;

			PDBG_DUMMY pdb = (PDBG_DUMMY)pci->Tag;
			if (pci->Expanded)
			{
				for (int i = 0; i < pdb->CodeCount; i++)
					GetListCtrl().DeleteItem(nItem + 1);
				pci->Expanded = FALSE;
			}
			else
			{
				PDBG_BLOCK pdb = (PDBG_BLOCK)pci->Tag;
				if (pdb->CodeCount == 0)
					GetDocument()->SendCommand(TM_DISASSEMBLE, DASM_RANGE, (LPARAM)pdb);
				for (int i = 0; i < pdb->CodeCount; i++)
					InsertItem(nItem + i + 1, true, pdb->AsmCodes[i]);
				pci->Expanded = TRUE;
			}
			if (m_nCurItem > nItem)
			{
				if (pci->Expanded)
					m_nCurItem += pdb->CodeCount;
				else
					m_nCurItem -= pdb->CodeCount;
			}
			if (m_nBrkItem > nItem)
			{
				if (pci->Expanded)
					m_nBrkItem += pdb->CodeCount;
				else
					m_nBrkItem -= pdb->CodeCount;
			}
			GetListCtrl().Update(nItem);
		}
	}
	*pResult = 0;
}

void CDAsmView::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pia = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint point(pia->ptAction);
	UINT uFlags = 0;
	int nItem = GetListCtrl().HitTest(point, &uFlags);
	if (nItem < 0 || uFlags == LVHT_ONITEMICON)
		return;

	PLINE_INFO pci = (PLINE_INFO)GetListCtrl().GetItemData(nItem);
	if (pci->IsAsm || ((PDBG_CBASE)pci->Tag)->CodeType <= CT_THUNK)
		return;

	PDBG_DUMMY pdb = (PDBG_DUMMY)pci->Tag;
	if (pci->Expanded)
	{
		for (int i = 0; i < pdb->CodeCount; i++)
			GetListCtrl().DeleteItem(nItem + 1);
		pci->Expanded = FALSE;
	}
	else
	{
		PDBG_BLOCK pdb = (PDBG_BLOCK)pci->Tag;
		if (pdb->CodeCount == 0)
			GetDocument()->SendCommand(TM_DISASSEMBLE, DASM_RANGE, (LPARAM)pdb);
		for (int i = 0; i < pdb->CodeCount; i++)
			InsertItem(nItem + i + 1, true, pdb->AsmCodes[i]);
		pci->Expanded = TRUE;
	}
	if (m_nCurItem > nItem)
	{
		if(pci->Expanded)
			m_nCurItem += pdb->CodeCount;
		else
			m_nCurItem -= pdb->CodeCount;
	}
	if (m_nBrkItem > nItem)
	{
		if (pci->Expanded)
			m_nBrkItem += pdb->CodeCount;
		else
			m_nBrkItem -= pdb->CodeCount;
	}
	GetListCtrl().Update(nItem);

	*pResult = 0;
}

void CDAsmView::OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pia = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint point(pia->ptAction);

	UINT uFlags = 0;
	int nItem = GetListCtrl().HitTest(point, &uFlags);
	if (nItem < 0)
		return;

	if (uFlags & LVHT_ONITEM)
	{
		ClientToScreen(&point);
#ifndef SHARED_HANDLERS
		G_PE_APP.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
	}
	*pResult = 0;
}

void CDAsmView::OnLvnHotTrack(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;
}

void CDAsmView::OnDebugRun2Cursor()
{
	if (m_nBrkItem < 0)
		return;

	POSITION pos = GetListCtrl().GetFirstSelectedItemPosition();
	int nWantItem = GetListCtrl().GetNextSelectedItem(pos);
	if (nWantItem < 0)
	{
		AfxMessageBox(L"선택된 항목이 없습니다.");
		return;
	}

	DWORD dwHaltRVA = 0;
	PLINE_INFO pci = (PLINE_INFO)GetListCtrl().GetItemData(nWantItem);
	if (pci->IsAsm)
		dwHaltRVA = ((PDBG_DASM)pci->Tag)->Code._offset;
	else
	{
		PDBG_CBASE pdc = (PDBG_CBASE)pci->Tag;
		if (pdc->CodeType == CT_NONE || pdc->CodeType == CT_DUMMY)
			return;
		if (pdc->CodeType == CT_THUNK)
			dwHaltRVA = ((PDBG_THUNK)pci->Tag)->AsmCode->Code._offset;
		else
			dwHaltRVA = pdc->BeginRVA;
	}

	CPEDoc* pDoc = GetDocument();
	PVOID arPrms[3] = { (PVOID)dwHaltRVA, (PVOID)m_pdm, (PVOID)m_dwDbgThId };
	pDoc->SendCommand(TM_BREAK_RESUME, (WPARAM)IDM_DEBUG_RUN2CURSOR, (LPARAM)arPrms);

	((CPEFrame*)AfxGetMainWnd())->UpdateDockPanes();
	nWantItem = m_nBrkItem;
	m_nBrkItem = -1;
	GetListCtrl().Update(nWantItem);
	m_dwDbgThId = 0;
}

void CDAsmView::OnUpdateDebugRun2Cursor(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_dwDbgThId > 0 && GetListCtrl().GetSelectedCount() > 0);
}

void CDAsmView::OnDebugGotoCurBP()
{
	GetListCtrl().EnsureVisible(m_nBrkItem, FALSE);
	GetListCtrl().Update(m_nBrkItem);
}

void CDAsmView::OnUpdateDebugGotoBP(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nBrkItem >= 0);
}

void CDAsmView::OnExplorGotoCurPos()
{
	GetListCtrl().EnsureVisible(m_nCurItem, FALSE);
	GetListCtrl().Update(m_nCurItem);
}

void CDAsmView::OnUpdateExplorGotoCurPos(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_nCurItem >= 0);
}

void CDAsmView::OnExplorGotoEntryPoint()
{
	ScrollDAsmLine(m_pdm->EntryPoint, DASM_SETPOS);
}

void CDAsmView::OnEditCopy()
{

}

void CDAsmView::OnUpdateEditCopy(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetListCtrl().GetSelectedCount() > 0);
}

void CDAsmView::OnDebugSaveAsm()
{
	TCHAR szDefExt[] = _T("asm");
	TCHAR szFilter[] = _T("어셈블리 코드(*.asm)|*.asm|모든 파일(*.*)|*.*||");
	CString szDef = GetDocument()->GetPathName();
	int nPos = szDef.ReverseFind(L'\\');
	if (nPos > 0)
		szDef = szDef.Left(nPos) + m_pdm->ModuleName + L".asm";
	else
		szDef = m_pdm->ModuleName + L".asm";
	CFileDialog dlg(FALSE, szDefExt, szDef, OFN_OVERWRITEPROMPT, szFilter);
	if (dlg.DoModal() != IDOK)
		return;

}

void CDAsmView::OnDebugSetBreak()
{
	POSITION pos = GetListCtrl().GetFirstSelectedItemPosition();
	int nItem = GetListCtrl().GetNextSelectedItem(pos);
	if (nItem < 0)
	{
		AfxMessageBox(L"선택된 항목이 없습니다.");
		return;
	}

	PLINE_INFO pci = (PLINE_INFO)GetListCtrl().GetItemData(nItem);
	if (pci->IsAsm || ((PDBG_CBASE)pci->Tag)->CodeType == CT_THUNK)
	{
		PDBG_DASM pasm = (pci->IsAsm) ?
			(PDBG_DASM)pci->Tag : ((PDBG_THUNK)pci->Tag)->AsmCode;
		PBYTE pTrgAddr = m_pdm->ImageBase + pasm->Code._offset;
		CPEFrame* pFrame = (CPEFrame*)AfxGetMainWnd();

		int nBrkOpt = (pasm->HasBPoint) ? DBG_BP_REMOVE : DBG_BP_CREATE;
		pFrame->GetBreakView()->UpdateBreakPoint(nBrkOpt, pTrgAddr, m_pdm);
		pasm->HasBPoint = !pasm->HasBPoint;
		GetListCtrl().Update(nItem);
	}
}

void CDAsmView::OnUpdateDebugSetBreak(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(GetListCtrl().GetSelectedCount() == 1);
}
