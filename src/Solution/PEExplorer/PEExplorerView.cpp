#include "stdafx.h"
#include "PEExplorer.h"
#include "PEExplorerView.h"
#include "HexaView.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif

PCWSTR CPEExplorerView::GSZ_COL_NAMES[COL_IDX_MAX] =
{
	L"필드", L"타입", L"오프셋", L"크기", L"값", L"상세"
};


// CPEExplorerView
CPEExplorerView::CPEExplorerView()
{
	m_hOldCur = NULL;
	m_bHndCur = false;
}

CPEExplorerView::~CPEExplorerView()
{
}

PEAnals* CPEExplorerView::GetPEAnals(HTREEITEM htiCur)
{
	HTREEITEM htUpper = NULL;
	while (true)
	{
		htUpper = m_ctlPE.GetParentItem(htiCur);
		if (htUpper == NULL)
		{
			PEAnals* pchk = (PEAnals*)m_ctlPE.GetItemData(htiCur);
			return pchk;
		}
		htiCur = htUpper;
	}
	return NULL;
}

int CALLBACK CPEExplorerView::NodeCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CTreeListCtrl* pCtlPE = (CTreeListCtrl*)lParamSort;
	//CString strItem1 = pmyTreeCtrl->GetItemText((HTREEITEM)lParam1);
	//CString strItem2 = pmyTreeCtrl->GetItemText((HTREEITEM)lParam2);
	PPE_NODE pn1 = PPE_NODE(lParam1);
	PPE_NODE pn2 = PPE_NODE(lParam2);

	return (int(pn1->Offset) - int(pn2->Offset));
}

void CPEExplorerView::SortChildNode(HTREEITEM hNode)
{
	TVSORTCB	tvs;
	tvs.hParent		= hNode;
	tvs.lpfnCompare = CPEExplorerView::NodeCompareProc;
	tvs.lParam		= (LPARAM)&m_ctlPE;

	m_ctlPE.Expand(hNode, TVE_EXPAND);
	m_ctlPE.SortChildrenCB(&tvs);
	m_ctlPE.Expand(hNode, TVE_COLLAPSE);
}


BEGIN_MESSAGE_MAP(CPEExplorerView, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_WM_SETCURSOR()

	ON_COMMAND(ID_FILE_OPEN, &CPEExplorerView::OnFileOpen)
	ON_COMMAND(ID_FILE_CLOSE, &CPEExplorerView::OnFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, &CPEExplorerView::OnUpdateFileClose)
	ON_COMMAND(IDM_HEXA_DUMP, &CPEExplorerView::OnHexaDump)
	ON_COMMAND(IDM_COPY_DUMP, &CPEExplorerView::OnCopyDump)
	ON_COMMAND(IDM_VIEW_DETAIL, &CPEExplorerView::OnViewDetail)

	ON_NOTIFY(TVN_DELETEITEM, IDC_TV_PEVIEW, &CPEExplorerView::OnTvnDeleteitemPEView)
	ON_NOTIFY(NM_DBLCLK, IDC_TV_PEVIEW, &CPEExplorerView::OnNMDblClickPEView)
	ON_NOTIFY(NM_RCLICK, IDC_TV_PEVIEW, &CPEExplorerView::OnNMRClickPEView)
	ON_NOTIFY(NM_CLICK, IDC_TV_PEVIEW, &CPEExplorerView::OnNMLClickPEView)
	ON_NOTIFY(NM_HOVER, IDC_TV_PEVIEW, &CPEExplorerView::OnNMHoverPEView)
END_MESSAGE_MAP()

// CPEExplorerView 메시지 처리기
BOOL CPEExplorerView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

int CPEExplorerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rc;
	GetClientRect(rc);
	UINT uStyle = TVS_HASBUTTONS | TVS_HASBUTTONS | TVS_HASLINES | 
				  TVS_LINESATROOT | TVS_FULLROWSELECT;
	m_ctlPE.Create(uStyle | WS_CHILD | WS_VISIBLE , rc, this, IDC_TV_PEVIEW);
	UINT uExStyle = TVS_EX_ITEMLINES | TVS_EX_ITEMLINES | TVS_EX_SEND_HOVER |
					TVS_EX_ALTERNATECOLOR | TVS_EX_FULLROWMARK | TVS_EX_SUBSELECT;
	m_ctlPE.SetExtendedStyle(uExStyle);

	m_imgPE.Create(IDB_TREE_NODE, 16, 3, RGB(255, 255, 255));
	m_ctlPE.SetImageList(&m_imgPE, TVSIL_NORMAL);
	m_tvFont.CreatePointFont(90, _T("Courier New"));
	m_ctlPE.SetFont(&m_tvFont);

	int nColWs[COL_IDX_MAX] = { 220, 180, 80, 120, 150, 500	};
	for (int i = 0; i < COL_IDX_MAX; i++)
		m_ctlPE.InsertColumn(i, GSZ_COL_NAMES[i], 
		(i == 3) ? TVCFMT_RIGHT : TVCFMT_LEFT, nColWs[i]);

	return 0;
}

void CPEExplorerView::OnDestroy()
{
	for (ANAL_MAP::iterator it = m_anals.begin(); it != m_anals.end(); it++)
		delete it->second;
	CWnd::OnDestroy();
}

void CPEExplorerView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	if (m_ctlPE.GetSafeHwnd())
		m_ctlPE.SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER);
}

BOOL CPEExplorerView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	if (m_bHndCur)
		m_hOldCur = SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
	else
	{
		SetCursor(m_hOldCur);
		m_hOldCur = NULL;
		return CWnd::OnSetCursor(pWnd, nHitTest, message);
	}
	return TRUE;
}

void CPEExplorerView::OnContextMenu(CWnd* pWnd, CPoint ptMousePos)
{
	if (ptMousePos.x == -1 && ptMousePos.y == -1)
		ptMousePos = (CPoint)GetMessagePos();

	ScreenToClient(&ptMousePos);

	UINT uFlags;
	HTREEITEM htItem = m_ctlPE.HitTest(ptMousePos, &uFlags);
	if (htItem == NULL)
		return;

	//m_hActiveItem = htItem;
	CMenu menu;
	CMenu* pPopup;

	// the font popup is stored in a resource
	menu.LoadMenu(IDR_MENU_POPUP);
	pPopup = menu.GetSubMenu(0);
	ClientToScreen(&ptMousePos);
	pPopup->TrackPopupMenu(TPM_LEFTALIGN, ptMousePos.x, ptMousePos.y, this);
}

void CPEExplorerView::OnTvnDeleteitemPEView(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNmtv = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	if (pNmtv->itemOld.mask & TVIF_PARAM && pNmtv->itemOld.lParam != 0)
	{
		PPE_NODE pDel = (PPE_NODE)pNmtv->itemOld.lParam;
		delete pDel;
		m_ctlPE.SetItemData(pNmtv->itemOld.hItem, 0);
	}
	*pResult = 0;
}

void CPEExplorerView::OnNMDblClickPEView(NMHDR *pNMHDR, LRESULT *pResult)
{
	CPoint pt = (CPoint)GetMessagePos();
	ScreenToClient(&pt);
	UINT uFlags;
	HTREEITEM htItem = m_ctlPE.HitTest(pt, &uFlags);
	if (htItem == NULL || ((TVHT_ONITEM | TVHT_ONSUBLABEL) & uFlags) == 0)
		return;

	*pResult = 0;
}

void CPEExplorerView::OnNMRClickPEView(NMHDR *pNMHDR, LRESULT *pResult)
{
	SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, GetMessagePos());
	*pResult = 1;
}

void CPEExplorerView::OnNMHoverPEView(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	LPNMTREEVIEW pNmtv = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	if (pNmtv->itemOld.mask & TVIF_PARAM && pNmtv->itemOld.lParam != 0)
	{
		PPE_NODE pn = (PPE_NODE)pNmtv->itemOld.lParam;
		if (pn->Field->IsRva && pn->Field->Ref.Rva != NULL)
		{
			PPE_NODE pnr = (PPE_NODE)pn->Field->Ref.Rva;
			m_ctlPE.SetItemTextColor(pn->Node, 5, RGB(0, 0, 0));
			m_ctlPE.SetItemTextColor(pnr->Node, 0, RGB(0, 0, 0));
		}
	}

	if (pNmtv->itemNew.mask & TVIF_PARAM && pNmtv->itemNew.lParam != 0)
	{
		if (pNmtv->itemNew.cChildren == COL_IDX_INFO)
		{
			PPE_NODE pn = (PPE_NODE)pNmtv->itemNew.lParam;
			if (pn->Field->IsRva && pn->Field->Ref.Rva != NULL)
			{
				*pResult = 1;
				PPE_NODE pnr = (PPE_NODE)pn->Field->Ref.Rva;
				m_ctlPE.SetItemTextColor(pn->Node, 5, RGB(0, 0, 255));
				m_ctlPE.SetItemTextColor(pnr->Node, 0, RGB(255, 0, 0));
				m_bHndCur = true;
			}
		}
	}
	if (*pResult == 0)
		m_bHndCur = false;
	*pResult = 1;
}

void CPEExplorerView::OnNMLClickPEView(NMHDR *pNMHDR, LRESULT *pResult)
{
	//CPoint pt = (CPoint)GetMessagePos();
	//ScreenToClient(&pt);

	//TVHITTESTINFO tvhi;
	//UINT uFlags;
	//HTREEITEM htItem = m_ctlPE.HitTest(pt, &uFlags);
	//if (htItem == NULL || ((/*TVHT_ONITEM | */TVHT_ONSUBLABEL) & uFlags) == 0)
	//	return;

	//CRect rc; 
	//if (!m_ctlPE.GetItemRect(htItem, COL_IDX_INFO, &rc, TVIR_GETCOLUMN))
	//	return;
	//if (!rc.PtInRect(pt))
	//	return;
	//PPE_NODE pn = (PPE_NODE)m_ctlPE.GetItemData(htItem);
	//if (!pn->Field->IsRva)
	//	return;
	//PPE_NODE pr = (PPE_NODE)pn->Field->Ref.Rva;
	//if (pr == NULL)
	//	return;
	//m_ctlPE.SetItemTextColor(pr->Node, 0, RGB(255, 0, 0));
	//*pResult = 1;
}

void CPEExplorerView::OnFileOpen()
{
	TCHAR szDefExt[] = _T("exe");
	TCHAR szFilter[] = _T("실행 어플리케이션(*.exe)|*.exe|")
		_T("동적 라이브러리(*.dll)|*.dll|ActiveX 컨트롤(*.ocx)|*.ocx|")
		_T("디바이스 드라이버(*.sys)|*.sys|모든 파일(*.*)|*.*||");

	CFileDialog dlg(TRUE, szDefExt, _T(""),
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter);
	if (dlg.DoModal() != IDOK)
		return;

	CString szErr;
	CString szPEPath = dlg.GetPathName().MakeLower();
	ANAL_MAP::iterator it = m_anals.find(szPEPath);
	if (it != m_anals.end())
	{
		szErr.Format(_T("%s\xd\xa해당 PE 파일이 이미 열려 있습니다."), dlg.GetPathName());
		AfxMessageBox(szErr);
		return;
	}

	SORT_SET sorts;
	PPE_NODE pnRoot = NULL;
	PEAnals* ppe = NULL;
	try
	{
		CString szItem;
		ppe = new PEAnals(szPEPath, &m_ctlPE);
		pnRoot = ppe->ParsePEImage(sorts);
	}
	catch (HRESULT hr)
	{
		szErr.Format(_T("Error occurred : 0x%08X"), hr);
	}
	catch (PCTSTR pszErr)
	{
		szErr = pszErr;
	}

	if (!szErr.IsEmpty())
	{
		AfxMessageBox(szErr);
		if (pnRoot != NULL)
		{
			m_ctlPE.DeleteItem(pnRoot->Node);
			m_ctlPE.Invalidate();
			delete pnRoot;
		}
	}
	else
	{
		m_anals.insert(std::make_pair(szPEPath, ppe));
		m_ctlPE.Expand(pnRoot->Node, TVE_EXPAND);
		m_ctlPE.SelectItem(pnRoot->Node);
		m_ctlPE.EnsureVisible(pnRoot->Node);

		for (SORT_SET::iterator it = sorts.begin(); it != sorts.end(); it++)
		{
			SortChildNode(*it);
		}
		//PIMAGE_SECTION_HEADER pshs = ppe->GetSectHdrs();
		//for (int i = 0; i < ppe->GetSectCount(); i++)
		//{
		//	PPE_NODE pn = ppe->FindNode(pnRoot->Node, pshs[i].PointerToRawData);
		//	if (pn != NULL)
		//		SortChildNode(pn->Node);
		//}
	}
}

void CPEExplorerView::OnFileClose()
{
	HTREEITEM hCurSel = m_ctlPE.GetSelectedItem();
	while (hCurSel != NULL)
	{
		HTREEITEM hParent = m_ctlPE.GetParentItem(hCurSel);
		if (hParent == NULL)
			break;
		hCurSel = hParent;
	}
	if (hCurSel == NULL)
		AfxMessageBox(_T("닫을 PE를 선택하세요."));
	else
	{
		PPE_NODE pn = (PPE_NODE)m_ctlPE.GetItemData(hCurSel);
		ANAL_MAP::iterator it = m_anals.find(pn->PE->GetPEPath());
		if (it == m_anals.end())
		{
			AfxMessageBox(_T("닫을 PE가 존재하지 않습니다."));
			return;
		}
		delete it->second;
		m_anals.erase(it);

		m_ctlPE.DeleteItem(hCurSel);
		m_ctlPE.Invalidate();
	}
}

void CPEExplorerView::OnUpdateFileClose(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_ctlPE.GetChildItem(TVI_ROOT) != NULL);
}

void CPEExplorerView::OnHexaDump()
{
	HTREEITEM hti = m_ctlPE.GetSelectedItem();
	int nImgIdx = -1;
	m_ctlPE.GetItemImage(hti, nImgIdx, nImgIdx);
	if (nImgIdx == IMG_IDX_PE)
		return;

	SHORT nPrs = GetKeyState(VK_LCONTROL);
	PPE_NODE pNod = (PPE_NODE)m_ctlPE.GetItemData(hti);
	if (nPrs < 0)
	{
		CHexaView cv(pNod, &pNod->PE->CODE_LIST, pNod->PE->CODE_MAP.find(pNod->Offset)->second);
		cv.DoModal();
	}
	else
	{
		
		CHexaView cv(pNod);// , true);
		cv.DoModal();
	}
}

void CPEExplorerView::OnCopyDump()
{
	HTREEITEM hti = m_ctlPE.GetSelectedItem();
	int nImgIdx = -1;
	m_ctlPE.GetItemImage(hti, nImgIdx, nImgIdx);
	if (nImgIdx == IMG_IDX_PE)
		return;

	PPE_NODE pNod = (PPE_NODE)m_ctlPE.GetItemData(hti);
	int nSrt = pNod->Offset / 16 * 16;
	int nEnd = (pNod->Offset + pNod->Size) / 16 * 16;
	if ((pNod->Offset + pNod->Size) % 16 > 0)
		nEnd += 16;

	int nLen = nEnd - nSrt;
	PBYTE pRaw = pNod->PE->GetImgView() + nSrt;
	CString szFrm;
	for (int i = 0; i < 16; i++)
		szFrm.AppendFormat(_T("\t+%X"), i);
	szFrm.AppendFormat(_T("\xd\xa"));
	int nRow = nLen / 16;
	int nMod = nLen % 16;
	for (int i = 0; i < nRow; i++)
	{
		int nBx = i * 16;
		szFrm.AppendFormat(_T("%08X"), nSrt + nBx);
		for (int j = 0; j < 16; j++, nBx++)
		{
			szFrm.AppendFormat(_T("\t%02X"), pRaw[i * 16 + j]);
		}
		szFrm.AppendFormat(_T("\xd\xa"));
	}

	HANDLE hData = ::GlobalAlloc(GMEM_MOVEABLE, szFrm.GetLength() * sizeof(TCHAR));
	PBYTE pData = (PBYTE)::GlobalLock(hData);
	memcpy(pData, (PBYTE)(PCTSTR)szFrm, szFrm.GetLength() * sizeof(TCHAR));
	::GlobalUnlock(hData);

	COleDataSource* pDS = new COleDataSource;
	pDS->CacheGlobalData(CF_UNICODETEXT, hData);
	pDS->SetClipboard();
}

void CPEExplorerView::OnViewDetail()
{
	HTREEITEM hti = m_ctlPE.GetSelectedItem();
	int nImgIdx = -1;
	m_ctlPE.GetItemImage(hti, nImgIdx, nImgIdx);
	if (nImgIdx == IMG_IDX_PE)
		return;

	PPE_NODE pn = (PPE_NODE)m_ctlPE.GetItemData(hti);
	if (pn->Field->Type != PE_TYPE::Struct)
		return;

	CString sz;
	if (pn->Field->IsArr)
	{
		SHORT nPrs = GetKeyState(VK_LCONTROL);
		sz = _T("Index");
		PPE_STRUCT pst = pn->Field->Struct;
		for (MBR_LIST::iterator it = pst->Members.begin(); it < pst->Members.end(); it++)
			sz.AppendFormat(L"\t%s", (*it)->Name);
		sz.Append(L"\xd\xa");

		HTREEITEM hIter = m_ctlPE.GetChildItem(pn->Node);
		while (hIter != NULL)
		{
			PPE_NODE pitm = (PPE_NODE)m_ctlPE.GetItemData(hIter);
			sz.Append(pitm->Field->Name);
			HTREEITEM hSub = m_ctlPE.GetChildItem(hIter);
			while (hSub != NULL)
			{
				sz.AppendFormat(L"\t%s, %s", 
					m_ctlPE.GetItemText(hSub, COL_IDX_VALUE),
					m_ctlPE.GetItemText(hSub, COL_IDX_INFO));
				hSub = m_ctlPE.GetNextSiblingItem(hSub);
			}
			sz.Append(L"\xd\xa");
			hIter = m_ctlPE.GetNextSiblingItem(hIter);
		}
		if (nPrs < 0)
		{
			//typedef CString* PMFCSTR;
			CString** pps = new CString*[pn->Field->ArrLen + 1];
			for (int i = 0; i < pn->Field->ArrLen + 1; i++)
				pps[i] = new CString[pst->Members.size() + 1];

			int nRowPos = 0, nRowIdx = 0;
			while (true)
			{
				int nwpi = sz.Find(L"\xd\xa", nRowPos);
				int nRowLen = nwpi - nRowPos;
				if (nwpi == 0)
					break;
				if (nwpi < 0)
					nRowLen = sz.GetLength() - nRowPos;
				if (nRowLen <= 0)
					break;

				int nColPos = nRowPos, nColIdx = 0;
				while (true)
				{
					int ncpi = sz.Find(L'\t', nColPos);
					int nColLen = ncpi - nColPos;
					if (ncpi > nwpi || ncpi > nwpi)
						nColLen = nwpi - nColPos;

					pps[nRowIdx][nColIdx] = sz.Mid(nColPos, nColLen);
					//wcscpy_s(pps[nRowIdx][nColIdx], szv);
					if (ncpi < 0 || ncpi > nwpi)
						break;
					nColPos = ncpi + 1;
					nColIdx++;
				}
				if (nwpi < 0)
					break;
				nRowPos = nwpi + 2;
				nRowIdx++;
			}
			sz.Empty();
			for (int j = 0; j < pst->Members.size() + 1; j++)
			{
				for (int i = 0; i < pn->Field->ArrLen + 1; i++)
				{
					if (i > 0)
						sz.Append(L"\t");
					sz.Append(pps[i][j]);
				}
				sz.Append(L"\xd\xa");
			}
			for (int i = 0; i < pn->Field->ArrLen; i++)
				delete[] pps[i];
			delete[] pps;
		}
	}
	else
	{
		for (int i = 0; i < COL_IDX_MAX; i++)
		{
			if (i == COL_IDX_SIZE)
				continue;
			if (i > 0)
				sz.Append(L"\t");
			sz.Append(GSZ_COL_NAMES[i]);
		}
		sz.Append(L"\xd\xa");
		HTREEITEM hIter = m_ctlPE.GetChildItem(pn->Node);
		while (hIter != NULL)
		{
			for (int i = 0; i < COL_IDX_MAX; i++)
			{
				if (i == COL_IDX_SIZE)
					continue;
				if (i > 0)
					sz.Append(L"\t");
				sz.Append(m_ctlPE.GetItemText(hIter, i));
			}
			sz.Append(L"\xd\xa");
			hIter = m_ctlPE.GetNextSiblingItem(hIter);
		}
	}
	if (sz.IsEmpty())
		return;

	HANDLE hData = ::GlobalAlloc(GMEM_MOVEABLE, sz.GetLength() * sizeof(TCHAR));
	PBYTE pData = (PBYTE)::GlobalLock(hData);
	memcpy(pData, (PBYTE)(PCTSTR)sz, sz.GetLength() * sizeof(TCHAR));
	::GlobalUnlock(hData);

	COleDataSource* pDS = new COleDataSource;
	pDS->CacheGlobalData(CF_UNICODETEXT, hData);
	pDS->SetClipboard();
}
