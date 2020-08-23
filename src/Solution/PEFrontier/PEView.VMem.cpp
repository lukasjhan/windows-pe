#include "stdafx.h"
#ifndef SHARED_HANDLERS
#	include "PEApp.h"
#endif
#include "PEDoc.h"
#include "PEView.VMem.h"
#include "PEDlg.MemDump.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


// CVMemView
IMPLEMENT_DYNCREATE(CVMemView, CListView)
BEGIN_MESSAGE_MAP(CVMemView, CListView)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY_REFLECT(LVN_GETDISPINFO, &CVMemView::OnLvnGetdispinfo)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CVMemView::OnNMCustomdraw)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CVMemView::OnNMDblclk)
	ON_NOTIFY_REFLECT(NM_RCLICK, &CVMemView::OnNMRClick)
END_MESSAGE_MAP()

// CVMemView 생성/소멸
CVMemView::CVMemView()
{
}

CVMemView::~CVMemView()
{
	for (DBG_HEAP_MAP::iterator it = m_heaps.begin(); it != m_heaps.end(); it++)
		delete it->second;
	for (DBG_VMOBJ_MAP::iterator it = m_vmos.begin(); it != m_vmos.end(); it++)
		delete it->second;
}

BOOL CVMemView::AddItem(int nIndex, LPARAM pInfo)
{
	PDBG_VMOBJ pdvm = (PDBG_VMOBJ)pInfo;
	int nImgIdx = 4;
	if (pdvm->ObjType > PAGE_TYPE::PT_NONE)
		nImgIdx = pdvm->ObjType - 1;
	else
	{
		if (pdvm->Mbi.State & MEM_COMMIT)
			nImgIdx = 6;
		else if (pdvm->Mbi.State & MEM_RESERVE)
			nImgIdx = 5;
	}

	LV_ITEM	lvi;
	lvi.mask	 = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
	lvi.iItem	 = nIndex;
	lvi.iSubItem = 0;
	lvi.iImage	 = nImgIdx;
	lvi.pszText  = LPSTR_TEXTCALLBACK;
	lvi.lParam	 = (LPARAM)pInfo;
	if (GetListCtrl().InsertItem(&lvi) == -1)
		return FALSE;
	return TRUE;
}

BOOL CVMemView::PreCreateWindow(CREATESTRUCT& cs)
{
	cs.style &= ~LVS_TYPEMASK;
	cs.style |= (LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL);
	return CListView::PreCreateWindow(cs);
}

void CVMemView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();

	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	GetListCtrl().SetExtendedStyle(GetListCtrl().GetExtendedStyle() | dwExStyle);

	CBitmap bmp; bmp.LoadBitmap(IDB_VIEW_VMEM);
	m_imgView.Create(18, 18, ILC_MASK | ILC_COLOR32, 0, 0);
	m_imgView.Add(&bmp, RGB(255, 255, 255));
	GetListCtrl().SetImageList(&m_imgView, LVSIL_SMALL);
	GetListCtrl().SetFont(((CPEApp*)AfxGetApp())->GetFont());

	GetListCtrl().InsertColumn(0, _T("주소"), LVCFMT_LEFT, 150);
	GetListCtrl().InsertColumn(1, _T("상태"), LVCFMT_CENTER, 80);
	GetListCtrl().InsertColumn(2, _T("보호"), LVCFMT_CENTER, 100);
	GetListCtrl().InsertColumn(3, _T("크기"), LVCFMT_RIGHT, 150);
	GetListCtrl().InsertColumn(4, _T("기본주소"), LVCFMT_LEFT, 150);
	GetListCtrl().InsertColumn(5, _T("객체"), LVCFMT_CENTER, 80);
	GetListCtrl().InsertColumn(6, _T("섹션"), LVCFMT_LEFT, 80);
	GetListCtrl().InsertColumn(7, _T("이름"), LVCFMT_LEFT, 150);

	CPEDoc* pDoc = GetDocument();
	m_bIs32 = pDoc->GetDbgProcess()->Is32Bit;
	pDoc->SendCommand(TM_WALK_PROCMEM, (WPARAM)&m_heaps, (LPARAM)&m_vmos);

	int nIndex = 0;
	for (DBG_VMOBJ_MAP::reverse_iterator it = m_vmos.rbegin(); it != m_vmos.rend(); it++)
		AddItem(nIndex, (LPARAM)it->second);
}

// CVMemView 진단
#ifdef _DEBUG
void CVMemView::AssertValid() const
{
	CListView::AssertValid();
}

void CVMemView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CPEDoc* CVMemView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPEDoc)));
	return (CPEDoc*)m_pDocument;
}
#endif //_DEBUG


void CVMemView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
//	G_PE_APP.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}

void CVMemView::OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	static PCWSTR MEM_PROPS[]  = { L"프리", L"예약", L"커밋" };
	static PCWSTR PAGE_TYPES[] = { L"", L"힙", L"스택", L"EXE", L"DLL" };
	static PCWSTR PAGE_PROPS[] = 
	{ 
		L"접근금지",			// PAGE_NOACCESS 
		L"읽기전용",			// PAGE_READONLY 
		L"읽기/쓰기",			// PAGE_READWRITE
		L"쓰기복사",			// PAGE_WRITECOPY
		L"실행",				// PAGE_EXECUTE          
		L"읽기실행",			// PAGE_EXECUTE_READ     
		L"읽기/쓰기실행",		// PAGE_EXECUTE_READWRITE
		L"쓰기복사실행",		// PAGE_EXECUTE_WRITECOPY
		L"가드",				// PAGE_GUARD       
		L"캐시없음",			// PAGE_NOCACHE     
		L"쓰기결합",			// PAGE_WRITECOMBINE(0x400)
	};
	NMLVDISPINFO* pnmdv = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	PDBG_VMOBJ pdvm = (PDBG_VMOBJ)pnmdv->item.lParam;

	switch (pnmdv->item.iSubItem)
	{
		case 0:	//주소
			wsprintf(pnmdv->item.pszText, 
				PEPlus::GetAddrForm(m_bIs32, (PBYTE)pdvm->Mbi.BaseAddress));
		break;
		case 1:	//상태
		{
			if (pdvm->Mbi.State & MEM_COMMIT)
				pnmdv->item.pszText = (PWSTR)MEM_PROPS[2];
			else if (pdvm->Mbi.State & MEM_RESERVE)
				pnmdv->item.pszText = (PWSTR)MEM_PROPS[1];
			else
				pnmdv->item.pszText = (PWSTR)MEM_PROPS[0];
		}
		break;
		case 2:	//보호
		{
			CString sz;
			for (int i = 0; i < 11; i++)
			{
				if (pdvm->Mbi.Protect & (1 << i))
				{
					if (!sz.IsEmpty())
						sz.Append(L"|");
					sz.Append(PAGE_PROPS[i]);
				}
			}
			wsprintf(pnmdv->item.pszText, sz);
		}
		break;
		case 3:	//크기
			wsprintf(pnmdv->item.pszText, L"%s(0x%X)", 
				PEPlus::Size2Units((UINT64)pdvm->Mbi.RegionSize), pdvm->Mbi.RegionSize);
		break;
		case 4:	//기본주소
			wsprintf(pnmdv->item.pszText, L"0x%s", 
				PEPlus::GetAddrForm(m_bIs32, (PBYTE)pdvm->Mbi.AllocationBase));
		break;
		case 5:	//객체
			pnmdv->item.pszText = (PWSTR)PAGE_TYPES[pdvm->ObjType];
		break;
		case 6:	//섹션
			pnmdv->item.pszText = (PWSTR)(PCTSTR)pdvm->Section;
		break;
		case 7:	//이름
			pnmdv->item.pszText = (PWSTR)(PCTSTR)pdvm->Module;
		break;
	}
	*pResult = 0;
}

void CVMemView::OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVCUSTOMDRAW pvcd = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	PDBG_VMOBJ pvmo = PDBG_VMOBJ(pvcd->nmcd.lItemlParam);

	switch (pvcd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYSUBITEMDRAW;
		break;
		case CDDS_ITEMPREPAINT:
			*pResult = CDRF_NOTIFYSUBITEMDRAW;
		break;
		case (CDDS_ITEMPREPAINT | CDDS_SUBITEM) :
			if (pvmo->ObjType == PAGE_TYPE::PT_EXE)
				pvcd->clrTextBk = RGB(255, 220, 220);
			else if (pvmo->ObjType == PAGE_TYPE::PT_DLL)
				pvcd->clrTextBk = RGB(255, 255, 220);
			else if (pvmo->ObjType == PAGE_TYPE::PT_HEAP)
			{
				if (pvmo->Section.IsEmpty())
					pvcd->clrTextBk = RGB(220, 220, 255);
				else
					pvcd->clrTextBk = RGB(190, 190, 255);
			}
			else if (pvmo->ObjType == PAGE_TYPE::PT_STACK)
			{
				if (pvmo->Mbi.State & MEM_RESERVE)
					pvcd->clrTextBk = RGB(170, 255, 170);
				else if (pvmo->Mbi.Protect & PAGE_GUARD)
					pvcd->clrTextBk = RGB(190, 255, 190);
				else
					pvcd->clrTextBk = RGB(220, 255, 220);
			}
			else
			{
				if (pvmo->Mbi.State & MEM_FREE)
				{
					pvcd->clrTextBk = RGB(128, 128, 128);
					pvcd->clrText = RGB(210, 210, 210);
				}
				else if (pvmo->Mbi.State & MEM_RESERVE)
					pvcd->clrTextBk = RGB(210, 210, 210);
			}
			*pResult = CDRF_NEWFONT;
		break;
		default:
			*pResult = CDRF_DODEFAULT;
		break;
	}
}

void CVMemView::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pia = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CPoint point(pia->ptAction);
	int nItem = GetListCtrl().HitTest(point);
	if (nItem<0)
		return;

	PDBG_VMOBJ pvmo = (PDBG_VMOBJ)GetListCtrl().GetItemData(nItem);
	if (pvmo->Mbi.State != MEM_COMMIT)
	{
		AfxMessageBox(L"메모리 영역이 확정 영역이 아닙니다.");
		return;
	}

	DWORD dwSize = (DWORD)pvmo->Mbi.RegionSize;
	PVOID pMem = VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	if (pMem == NULL)
	{
		AfxMessageBox(PEPlus::GetErrMsg(GetLastError()));
		return;
	}

	MEM_IO_PRM mio;
	mio.Buff  = pMem;
	mio.Size  = dwSize;
	mio.Error = S_OK;
	GetDocument()->SendCommand(TM_READ_MEMORY, (WPARAM)pvmo->Mbi.BaseAddress, (LPARAM)&mio);
	if (FAILED(mio.Error))
		AfxMessageBox(PEPlus::GetErrMsg(mio.Error));
	else
	{
		CMemDump dlg(m_bIs32, pvmo->Mbi.BaseAddress, pMem, dwSize);
		dlg.DoModal();
	}
	VirtualFree(pMem, 0, MEM_RELEASE);
	*pResult = 0;
}

void CVMemView::OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult)
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
