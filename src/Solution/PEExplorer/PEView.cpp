#include "stdafx.h"
#include "PEExplorer.h"
#include "PEView.h"
#include "HexaView.h"
#include "DataView.h"
#include "CodeView.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif

PCWSTR CPEView::GSZ_COL_NAMES[COL_IDX_MAX] =
{
	L"필드", L"타입", L"오프셋:RVA", L"크기", L"값", L"상세"
};


// CPEView
CPEView::CPEView()
{
}

CPEView::~CPEView()
{
}
 
PPE_NODE CPEView::GetPENode(HTREEITEM htiCur)
{
	HTREEITEM htUpper = NULL;
	while (true)
	{
		htUpper = m_ctlPE.GetParentItem(htiCur);
		if (htUpper == NULL)
			return (PPE_NODE)m_ctlPE.GetItemData(htiCur);
		htiCur = htUpper;
	}
	return NULL;
}

int CALLBACK CPEView::NodeCompareProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	CTreeListCtrl* pCtlPE = (CTreeListCtrl*)lParamSort;
	PPE_NODE pn1 = PPE_NODE(lParam1);
	PPE_NODE pn2 = PPE_NODE(lParam2);

	return (int(pn1->Offset) - int(pn2->Offset));
}

void CPEView::SortChildNode(HTREEITEM hNode)
{
	TVSORTCB	tvs;
	tvs.hParent		= hNode;
	tvs.lpfnCompare = CPEView::NodeCompareProc;
	tvs.lParam		= (LPARAM)&m_ctlPE;

	m_ctlPE.Expand(hNode, TVE_EXPAND);
	m_ctlPE.SortChildrenCB(&tvs);
	m_ctlPE.Expand(hNode, TVE_COLLAPSE);
}


BEGIN_MESSAGE_MAP(CPEView, CWnd)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CONTEXTMENU()
	ON_WM_SETCURSOR()

	ON_COMMAND(ID_FILE_OPEN, &CPEView::OnFileOpen)
	ON_COMMAND(ID_FILE_CLOSE, &CPEView::OnFileClose)
	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, &CPEView::OnUpdateFileClose)
	ON_COMMAND(IDM_HEXA_VIEW, &CPEView::OnHexaView)
	ON_COMMAND(IDM_CODE_VIEW, &CPEView::OnCodeView)
	ON_COMMAND(IDM_DATA_VIEW, &CPEView::OnDataView)
	ON_COMMAND(IDM_RESOURCE_VIEW, &CPEView::OnResourceView)
	ON_COMMAND(IDM_CERT_INFO_VIEW, &CPEView::OnCertInfoView)
	ON_COMMAND(IDM_COPY_DUMP, &CPEView::OnCopyDump)
	ON_COMMAND(IDM_VIEW_DETAIL, &CPEView::OnViewDetail)
	ON_COMMAND_RANGE(IDM_LVI_COPY_ALL, IDM_LVI_COPY_INFO, OnLviCopyItem)
	ON_COMMAND(IDM_LVI_FIND_ITEM, &CPEView::OnLviFindItem)

	ON_NOTIFY(TVN_DELETEITEM, IDC_TV_PEVIEW, &CPEView::OnTvnDeleteitemPEView)
	ON_NOTIFY(NM_DBLCLK, IDC_TV_PEVIEW, &CPEView::OnNMDblClickPEView)
	ON_NOTIFY(NM_RCLICK, IDC_TV_PEVIEW, &CPEView::OnNMRClickPEView)
END_MESSAGE_MAP()

// CPEView 메시지 처리기
BOOL CPEView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

int CPEView::OnCreate(LPCREATESTRUCT lpCreateStruct)
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

	m_imgPE.Create(IDB_TREE_NODE, 16, 18, RGB(255, 255, 255));
	m_ctlPE.SetImageList(&m_imgPE, TVSIL_NORMAL);
	m_tvFont.CreatePointFont(100, _T("Courier New"));
	m_ctlPE.SetFont(&m_tvFont);

	int nColWs[COL_IDX_MAX] = { 220, 180, 150, 120, 150, 500	};
	for (int i = 0; i < COL_IDX_MAX; i++)
		m_ctlPE.InsertColumn(i, GSZ_COL_NAMES[i], 
		(i == 3) ? TVCFMT_RIGHT : TVCFMT_LEFT, nColWs[i]);

	return 0;
}

void CPEView::OnDestroy()
{
	for (ANAL_MAP::iterator it = m_anals.begin(); it != m_anals.end(); it++)
		delete it->second;
	CWnd::OnDestroy();
}

void CPEView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	if (m_ctlPE.GetSafeHwnd())
		m_ctlPE.SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOMOVE | SWP_NOZORDER);
}

BOOL CPEView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

void CPEView::OnContextMenu(CWnd* pWnd, CPoint ptmPos)
{
	if (ptmPos.x == -1 && ptmPos.y == -1)
		ptmPos = (CPoint)GetMessagePos();
	ScreenToClient(&ptmPos);

	UINT uFlags;
	HTREEITEM htItem = m_ctlPE.HitTest(ptmPos, &uFlags);
	if (htItem == NULL)
		return;
	PPE_NODE ppn = (PPE_NODE)m_ctlPE.GetItemData(htItem);
	if (ppn == NULL)
		return;

	PEAnals* pPE = NULL;
	while (true)
	{
		HTREEITEM htiUp = m_ctlPE.GetParentItem(htItem);
		if (htiUp == NULL)
		{
			PPE_NODE pNod = (PPE_NODE)m_ctlPE.GetItemData(htItem);
			pPE = (PEAnals*)pNod->Tag;
			break;
		}
		htItem = htiUp;
	}
	if (pPE == NULL)
		return;

	CMenu menu;
	if (!menu.CreatePopupMenu())
		return;
	menu.AppendMenu(MF_STRING | MF_ENABLED, IDM_HEXA_VIEW, L"헥사 덤프");
	if (ppn->Kind == NK_FUNC || (ppn->Kind == NK_SECT && ppn->SubT & PE_KIND_CODE))
		menu.AppendMenu(MF_STRING | MF_ENABLED, IDM_CODE_VIEW, L"코드 보기");
	if (ppn->Kind == NK_SECT && ppn->SubT & PE_KIND_DATA)
		menu.AppendMenu(MF_STRING | MF_ENABLED, IDM_DATA_VIEW, L"데이터 보기");
	if ((ppn->Kind == IMAGE_DIRECTORY_ENTRY_RESOURCE) && (ppn->SubT > 0))
		menu.AppendMenu(MF_STRING | MF_ENABLED, IDM_RESOURCE_VIEW, L"리소스 보기");
	if (ppn->Kind == IMAGE_DIRECTORY_ENTRY_SECURITY)
		menu.AppendMenu(MF_STRING | MF_ENABLED, IDM_CERT_INFO_VIEW, L"인증서 정보 보기");
	menu.AppendMenu(MF_SEPARATOR, 0, L"");

	CMenu mucpy;
	if (!mucpy.CreatePopupMenu())
		return;
	for (int i = COL_IDX_INFO; i >= COL_IDX_NAME; i--)
		mucpy.AppendMenu(MF_STRING | MF_ENABLED, IDM_LVI_COPY_FIELD + i, GSZ_COL_NAMES[i]);
	menu.AppendMenu(MF_POPUP, (UINT_PTR)mucpy.GetSafeHmenu(), L"항목 복사");
	menu.AppendMenu(MF_STRING | MF_ENABLED, IDM_LVI_COPY_ALL, L"항목 모두 복사");
	menu.AppendMenu(MF_STRING | MF_ENABLED, IDM_LVI_FIND_ITEM, L"항목 찾기");
	menu.AppendMenu(MF_SEPARATOR, 0, L"");

	menu.AppendMenu(MF_STRING | MF_ENABLED, IDM_COPY_DUMP, L"데이터 복사");
	menu.AppendMenu(MF_STRING | MF_ENABLED, IDM_VIEW_DETAIL, L"상세 보기");
	if (ppn->Type == PE_TYPE::PEFormat)
	{
		menu.AppendMenu(MF_SEPARATOR, 0, L"");
		menu.AppendMenu(MF_STRING | MF_ENABLED, ID_FILE_CLOSE, L"PE 파일 닫기");
	}
	ClientToScreen(&ptmPos);
	menu.TrackPopupMenu(TPM_LEFTALIGN, ptmPos.x, ptmPos.y, this);
}

void CPEView::OnTvnDeleteitemPEView(NMHDR *pNMHDR, LRESULT *pResult)
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

void CPEView::OnNMDblClickPEView(NMHDR *pNMHDR, LRESULT *pResult)
{
	CPoint pt = (CPoint)GetMessagePos();
	ScreenToClient(&pt);
	UINT uFlags;
	HTREEITEM htItem = m_ctlPE.HitTest(pt, &uFlags);
	if (htItem == NULL || ((TVHT_ONITEM | TVHT_ONSUBLABEL) & uFlags) == 0)
		return;

	*pResult = 0;
}

void CPEView::OnNMRClickPEView(NMHDR *pNMHDR, LRESULT *pResult)
{
	SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, GetMessagePos());
	*pResult = 1;
}

void CPEView::OnFileOpen()
{
	TCHAR szDefExt[] = _T("exe");
	TCHAR szFilter[] = _T("PE 파일(*.exe, *.dll, *.ocx, *.sys)|*.exe;*.dll;*.ocx;*.sys|")
		_T("실행 어플리케이션(*.exe)|*.exe|")
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
		m_ctlPE.SelectItem(pnRoot->Node);

		SortChildNode(pnRoot->Node);
		m_ctlPE.Expand(pnRoot->Node, TVE_EXPAND);
		m_ctlPE.EnsureVisible(pnRoot->Node);
		for (SORT_SET::iterator it = sorts.begin(); it != sorts.end(); it++)
		{
			SortChildNode(*it);
		}
	}
}

void CPEView::OnFileClose()
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
		PEAnals* pPE = (PEAnals*)pn->Tag;
		ANAL_MAP::iterator it = m_anals.find(pPE->GetPEPath());
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

#include "FindItemDlg.h"
void CPEView::OnLviFindItem()
{
	HTREEITEM hCurSel = m_ctlPE.GetSelectedItem();
	if (!m_ctlPE.ItemHasChildren(hCurSel))
		return;

	CFindItemDlg dlg;
	if (dlg.DoModal() != IDOK)
		return;

	HTREEITEM hTrg = NULL;
	HTREEITEM hItr = m_ctlPE.GetChildItem(hCurSel);
	while (hItr)
	{
		CString sz = m_ctlPE.GetItemText(hItr);
		if (!sz.IsEmpty())
		{
			if (sz.Find(dlg.m_szVal) >= 0)
			{
				hTrg = hItr;
				break;
			}
		}
		hItr = m_ctlPE.GetNextSiblingItem(hItr);
	}
	if (hTrg)
	{
		m_ctlPE.SelectSetFirstVisible(hTrg);
	}
}

void CPEView::OnUpdateFileClose(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_ctlPE.GetChildItem(TVI_ROOT) != NULL);
}

void CPEView::OnLviCopyItem(UINT uCmdId)
{
	HTREEITEM hti = m_ctlPE.GetSelectedItem();
	if (hti == NULL)
		return;

	CString sz;
	if (uCmdId == IDM_LVI_COPY_ALL)
	{
		for (int i = 0; i < COL_IDX_MAX; i++)
		{
			if (!sz.IsEmpty())
				sz.Append(L"\t");
			sz.Append(m_ctlPE.GetItemText(hti, i));
		}
	}
	else
	{
		int nColIdx = (int)(uCmdId - IDM_LVI_COPY_FIELD);
		sz = m_ctlPE.GetItemText(hti, nColIdx);
	}
	if (!sz.IsEmpty())
	{
		int nSize = (sz.GetLength() + 1) * sizeof(TCHAR);
		HANDLE hData = ::GlobalAlloc(GMEM_MOVEABLE, nSize);
		PBYTE pData = (PBYTE)::GlobalLock(hData);
		memcpy(pData, (PBYTE)(PCTSTR)sz, nSize);
		::GlobalUnlock(hData);

		COleDataSource* pDS = new COleDataSource;
		pDS->CacheGlobalData(CF_UNICODETEXT, hData);
		pDS->SetClipboard();
	}
}

void CPEView::OnHexaView()
{
	HTREEITEM hti = m_ctlPE.GetSelectedItem();
	PPE_NODE pnpe = GetPENode(hti);
	if (pnpe == NULL || pnpe->Node == hti)
		return;

	PPE_NODE pn = (PPE_NODE)m_ctlPE.GetItemData(hti);
	CHexaView cv(pn, (PEAnals*)pnpe->Tag);
	cv.DoModal();
}

void CPEView::OnCodeView()
{
	HTREEITEM hti = m_ctlPE.GetSelectedItem();
	PPE_NODE pnpe = GetPENode(hti);
	if (pnpe == NULL || pnpe->Node == hti)
		return;

	PPE_NODE pn = (PPE_NODE)m_ctlPE.GetItemData(hti);
	CCodeView cv(pn, (PEAnals*)pnpe->Tag);
	cv.DoModal();
}

void CPEView::OnDataView()
{
	HTREEITEM hti = m_ctlPE.GetSelectedItem();
	PPE_NODE pnpe = GetPENode(hti);
	if (pnpe == NULL || pnpe->Node == hti)
		return;

	PPE_NODE pn = (PPE_NODE)m_ctlPE.GetItemData(hti);
	CDataView cv(pn, (PEAnals*)pnpe->Tag);
	cv.DoModal();
}

void CPEView::OnResourceView()
{
	HTREEITEM hti = m_ctlPE.GetSelectedItem();
	PPE_NODE pnpe = GetPENode(hti);
	if (pnpe == NULL || pnpe->Node == hti)
		return;

	PPE_NODE pn = (PPE_NODE)m_ctlPE.GetItemData(hti);
	PEAnals* pPE = (PEAnals*)pnpe->Tag;
	pPE->ViewResDataEntry(pn);
}

#include <WinTrust.h>
#include "SecuView.h"
void CPEView::OnCertInfoView()
{
	HTREEITEM hti = m_ctlPE.GetSelectedItem();
	PPE_NODE pnpe = GetPENode(hti);
	if (pnpe == NULL || pnpe->Node == hti)
		return;

	PPE_NODE pn = (PPE_NODE)m_ctlPE.GetItemData(hti);
	PEAnals* pPE = (PEAnals*)pnpe->Tag;
	CSecuView cv(pPE->GetImgView() + pn->Offset, pn->Size);
	cv.DoModal();
}




void CPEView::OnCopyDump()
{
	HTREEITEM hti = m_ctlPE.GetSelectedItem();
	int nImgIdx = -1;
	m_ctlPE.GetItemImage(hti, nImgIdx, nImgIdx);
	//if (nImgIdx == IMG_IDX_PE)
	//	return;

	PPE_NODE pNod = (PPE_NODE)m_ctlPE.GetItemData(hti);
	int nSrt = pNod->Offset / 16 * 16;
	int nEnd = (pNod->Offset + pNod->Size) / 16 * 16;
	if ((pNod->Offset + pNod->Size) % 16 > 0)
		nEnd += 16;

	PEAnals* pPE = NULL;
	while (true)
	{
		HTREEITEM htiUp = m_ctlPE.GetParentItem(hti);
		if (htiUp == NULL)
		{
			pNod = (PPE_NODE)m_ctlPE.GetItemData(hti);
			pPE = (PEAnals*)pNod->Tag;
			break;
		}
		hti = htiUp;
	}
	if (pPE == NULL)
		return;

	int nLen = nEnd - nSrt;
	PBYTE pRaw = pPE->GetImgView() + nSrt;
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

void CPEView::OnViewDetail()
{
	HTREEITEM hti = m_ctlPE.GetSelectedItem();
	int nImgIdx = -1;
	m_ctlPE.GetItemImage(hti, nImgIdx, nImgIdx);
	if (nImgIdx == IMG_IDX_PE)
		return;

	PPE_NODE pn = (PPE_NODE)m_ctlPE.GetItemData(hti);
	if (pn->Type != PE_TYPE::Struct)
		return;

	CString sz;
	if (pn->IsArr)
	{
		SHORT nPrs = GetKeyState(VK_LCONTROL);
		sz = _T("Index");
		PPE_STRUCT pst = pn->Ref.Struct;
		for (MBR_LIST::iterator it = pst->Members.begin(); it < pst->Members.end(); it++)
			sz.AppendFormat(L"\t%s", (*it)->Name);
		sz.Append(L"\xd\xa");

		HTREEITEM hIter = m_ctlPE.GetChildItem(pn->Node);
		while (hIter != NULL)
		{
			PPE_NODE pitm = (PPE_NODE)m_ctlPE.GetItemData(hIter);
			sz.Append(pitm->Name);
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
			CString** pps = new CString*[pn->More.ArrLen + 1];
			for (int i = 0; i < pn->More.ArrLen + 1; i++)
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
			for (int j = 0; j < (int)pst->Members.size() + 1; j++)
			{
				for (int i = 0; i < pn->More.ArrLen + 1; i++)
				{
					if (i > 0)
						sz.Append(L"\t");
					sz.Append(pps[i][j]);
				}
				sz.Append(L"\xd\xa");
			}
			for (int i = 0; i < pn->More.ArrLen; i++)
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
