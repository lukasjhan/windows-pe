#include "..\stdafx.h"
#include "..\PEExplorer.h"
#include "ResViewMenu.h"
#include "afxdialogex.h"


// CResViewMenu 대화 상자입니다.

IMPLEMENT_DYNAMIC(CResViewMenu, CDialogEx)
CResViewMenu::CResViewMenu(LPBYTE pImgStart, CWnd* pParent /*=NULL*/)
	: CDialogEx(CResViewMenu::IDD, pParent)
{
	m_pImgStart = pImgStart;
}

CResViewMenu::~CResViewMenu()
{
}

void CResViewMenu::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void CResViewMenu::ParseMenuItems()
{
	Tv()->DeleteAllItems();

	TV_INSERTSTRUCT tvis ;
	ZeroMemory(&(tvis.item), sizeof(TVITEM));
	tvis.item.mask		= TVIF_TEXT|TVIF_PARAM|TVIF_HANDLE;
	tvis.item.pszText	= LPSTR_TEXTCALLBACK;
	tvis.item.cChildren	= 1;
	tvis.item.lParam	= (LPARAM)0;
	tvis.hParent		= TVI_ROOT;
	tvis.hInsertAfter	= TVI_LAST;

	HTREEITEM hRoot = Tv()->InsertItem(&tvis);
	LPBYTE pIter = m_pImgStart + sizeof(DWORD);
	MakeMenuTree(pIter, hRoot);
}

void CResViewMenu::MakeMenuTree(LPBYTE& pIter, HTREEITEM hParent)
{
	try
	{
		bool bIsExit = false;
		do
		{
			PMENU_ITEM pMenu = new MENU_ITEM();

			pMenu->_wAttr = *((LPWORD)pIter);
			pIter += sizeof(WORD);

			if(pMenu->_wAttr & MF_END)
				bIsExit = true;

			UINT_PTR uidItem = 0;
			if(!(pMenu->_wAttr & MF_POPUP))
			{
				uidItem = (UINT_PTR)(*((LPWORD)pIter));
				pMenu->_wID = (WORD)uidItem;
				pIter += sizeof(WORD);
			}

			CString sz = (PWSTR)pIter;
			if(sz.GetLength() > 0)
			{
				wcscpy_s(pMenu->_szName, sz);
			}
			else
			{
				if(!pMenu->_wAttr && !uidItem)
					wcscpy_s(pMenu->_szName, L"------------------");
			}
			pIter += (sz.GetLength() + 1) * sizeof(WCHAR);

			TV_INSERTSTRUCT tvis ;
			ZeroMemory(&(tvis.item), sizeof(TVITEM));
			tvis.item.mask		= TVIF_TEXT|TVIF_PARAM|TVIF_HANDLE;
			tvis.item.pszText	= LPSTR_TEXTCALLBACK;
			tvis.item.cChildren	= 1;
			tvis.item.lParam	= (LPARAM)pMenu;
			tvis.hParent		= hParent;
			tvis.hInsertAfter	= TVI_LAST;

			HTREEITEM hItem = Tv()->InsertItem(&tvis);
			if(!hItem)
				throw GetLastError();

			if(pMenu->_wAttr & MF_POPUP)
				MakeMenuTree(pIter, hItem);
		}
		while(!bIsExit);
	}
	catch(HRESULT hr)
	{
		AfxMessageBox(PEPlus::GetErrMsg(hr));
	}
}

PMENU_ITEM CResViewMenu::FindItem(CTreeCtrl* pTree, HTREEITEM hItem, WORD wID)
{
	HTREEITEM hTemp = pTree->GetChildItem(hItem);
	while(hTemp)
	{
		PMENU_ITEM pMenu = (PMENU_ITEM)pTree->GetItemData(hTemp);
		if(pMenu->_wAttr & MF_POPUP)
		{
			pMenu = FindItem(pTree, hTemp, wID);
			if(pMenu)
				return pMenu;
		}
		else
		{
			if(pMenu->_wID == wID)
				return pMenu;
		}
		hTemp = pTree->GetNextSiblingItem(hTemp);
	}

	return NULL;
}

LRESULT CALLBACK MenuWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HMENU		s_hMenu;
	static CTreeCtrl*	s_pTree;

	switch(uMsg)
	{
		case WM_CREATE :
		{
			LPCREATESTRUCT pCS = (LPCREATESTRUCT)lParam;
			PCREATE_PRM pPrm = (PCREATE_PRM)pCS->lpCreateParams;
			s_hMenu = pPrm->_hMenu;
			s_pTree = pPrm->_pTree;
		}
		return TRUE;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			RECT        rc;

			CString szOut;
			if(s_hMenu)
				szOut = L"마우스 오른쪽 버튼을 클릭해 주세요!!!";
			else
				szOut = L"주메뉴를 선택해 주세요!!!";
			HDC hDC = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rc);
			DrawText(hDC, 
					 szOut, 
					 szOut.GetLength(), 
					 &rc, 
					 DT_SINGLELINE|DT_CENTER|DT_VCENTER);
			EndPaint (hWnd, &ps) ;
		}
		return 0;

		case WM_CONTEXTMENU	:
		{
			if(s_hMenu)
			{
				TrackPopupMenu(s_hMenu, TPM_LEFTALIGN|TPM_LEFTBUTTON , 
					LOWORD(lParam), HIWORD(lParam), 0, hWnd, NULL);
				return 0;
			}
		}
		break;

		case WM_DESTROY:
			//PostQuitMessage(0);
		return 0;

		case WM_COMMAND :
		{
			if(!HIWORD(wParam))
			{
				WORD id = LOWORD(wParam);
				HTREEITEM hRoot = s_pTree->GetRootItem();
				PMENU_ITEM pMenu = CResViewMenu::FindItem(s_pTree, hRoot, id);

				CString szOut;
				if(pMenu)
					szOut.Format(L"%s [ID : %d] 메뉴가 선택되었습니다.",
						pMenu->_szName, pMenu->_wID);
				else
					szOut.Format(L"ID %d 메뉴 항목이 존재하지 않습니다", id);
				AfxMessageBox(szOut);
				return 0;
			}
		}
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void CResViewMenu::ShowParsedMenu(bool bIsMain)
{
	PCWSTR pcszClsName = L"TEST_MENU";
	PBYTE pIter = m_pImgStart;
	WORD wVersion = *((PWORD)pIter); pIter += sizeof(WORD);
	WORD wcbSize  = *((PWORD)pIter); pIter += sizeof(WORD);

	if(wVersion || wcbSize)
	{
		AfxMessageBox(L"메뉴 리소스 포맷이 아닙니다.");
		return;
	}

	HMENU hMenu = MakeMenuItem(pIter, bIsMain);
	if(hMenu)
	{
		HWND         hWnd;
		WNDCLASS     wndclass;

		////////////////////////////////////////////////////////////////////////////
		//윈도우즈 클래스 등록
		////////////////////////////////////////////////////////////////////////////
		wndclass.style         = CS_HREDRAW|CS_VREDRAW;
		wndclass.lpfnWndProc   = MenuWndProc;
		wndclass.cbClsExtra    = 0;
		wndclass.cbWndExtra    = 0;
		wndclass.hInstance     = AfxGetInstanceHandle();
		wndclass.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
		wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
		wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wndclass.lpszMenuName  = NULL;
		wndclass.lpszClassName = pcszClsName;
		if(!RegisterClass(&wndclass))
		{
			//::MessageBox(NULL, TEXT("Window Class Registeration Failure!!!"),
			//		"TEST_MENU", MB_ICONERROR) ;
			//return;
		}
		////////////////////////////////////////////////////////////////////////////


		////////////////////////////////////////////////////////////////////////////
		//윈도우즈 생성& 갱신
		////////////////////////////////////////////////////////////////////////////
		CREATE_PRM cp;
		cp._hMenu = (bIsMain) ? NULL : hMenu;
		cp._pTree = (CTreeCtrl*)GetDlgItem(IDC_TV_MENU);
		hWnd = CreateWindow(pcszClsName, pcszClsName, WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT, CW_USEDEFAULT, 400, 150, 
							AfxGetMainWnd()->GetSafeHwnd(),	
							(bIsMain) ? hMenu : NULL, 
							AfxGetInstanceHandle(),	&cp);
		if(!hWnd)
		{
			::MessageBox(NULL, TEXT("Window Creation Failure!!!"),
				L"TEST_MENU", MB_ICONERROR);
			return;
		}
		::ShowWindow(hWnd, SW_NORMAL);
		::UpdateWindow(hWnd);
		////////////////////////////////////////////////////////////////////////////

		//MSG msg;
		//for(;;)
		//{
		//	if(PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
		//	{
		//		if(msg.message == WM_QUIT)
		//			break;
		//		TranslateMessage(&msg);
		//		DispatchMessage(&msg);
		//	}
		//}
	}
}

HMENU CResViewMenu::MakeMenuItem(LPBYTE& pIter, bool bIsRoot)
{
	HMENU hMenu = (bIsRoot) ? CreateMenu() : CreatePopupMenu();

	try
	{
		bool bIsExit = false;
		do
		{
			WORD wAttr = *((LPWORD)pIter);
			pIter += sizeof(WORD);

			if(wAttr & MF_END)
			{
				wAttr &= ~MF_END;
				bIsExit = true;
			}

			UINT_PTR uidItem = 0;
			if(!(wAttr & MF_POPUP))
			{
				uidItem = (UINT_PTR)(*((LPWORD)pIter));
				pIter += sizeof(WORD);
			}

			CString sz = (PWSTR)pIter;
			if (sz.GetLength() == 0)
			{
				if(!wAttr && !uidItem)
					wAttr = MF_SEPARATOR;
			}
			pIter += (sz.GetLength() + 1) * sizeof(WCHAR);

			if(wAttr & MF_POPUP)
				uidItem = (UINT_PTR)MakeMenuItem(pIter, false);

			BOOL bIsOK;
			if(wAttr == MF_SEPARATOR)
				bIsOK = AppendMenu(hMenu, wAttr, uidItem, L"");
			else
				bIsOK = AppendMenu(hMenu, wAttr, uidItem, sz);
			if(!bIsOK)
				throw GetLastError();
		}
		while(!bIsExit);
	}
	catch(HRESULT hr)
	{
		if(hMenu)
		{
			DestroyMenu(hMenu);
			hMenu = NULL;
		}
		AfxMessageBox(PEPlus::GetErrMsg(hr));
	}

	return hMenu;
}

BEGIN_MESSAGE_MAP(CResViewMenu, CDialogEx)
	ON_NOTIFY(TVN_GETDISPINFO, IDC_TV_MENU, OnTvnGetdispinfoTvMenu)
	ON_BN_CLICKED(IDC_BTN_SHOW_MAIN, OnBnClickedBtnShowMain)
	ON_BN_CLICKED(IDC_BTN_SHOW_POPUP, OnBnClickedBtnShowPopup)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TV_MENU, OnTvnSelchangedTvMenu)
	ON_NOTIFY(TVN_DELETEITEM, IDC_TV_MENU, OnTvnDeleteitemTvMenu)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CResViewMenu 메시지 처리기입니다.

void CResViewMenu::OnTvnGetdispinfoTvMenu(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVDISPINFO ptvdi = reinterpret_cast<LPNMTVDISPINFO>(pNMHDR);
	PMENU_ITEM pMenu = (PMENU_ITEM)(ptvdi->item.lParam);

	CString szFormat;
	if(!pMenu)
		szFormat = L"메뉴";
	else
		szFormat = pMenu->_szName;
	wsprintf(ptvdi->item.pszText, szFormat);

	*pResult = 0;
}

void CResViewMenu::OnBnClickedBtnShowMain()
{
	ShowParsedMenu(true);
}

void CResViewMenu::OnBnClickedBtnShowPopup()
{
	ShowParsedMenu(false);
}

BOOL CResViewMenu::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ParseMenuItems();

	return TRUE;  // 컨트롤에 대한 포커스를 설정하지 않을 경우 TRUE를 반환합니다.
}

void CResViewMenu::OnTvnSelchangedTvMenu(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pnmtv = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	PMENU_ITEM pMenu = (PMENU_ITEM)pnmtv->itemNew.lParam;

	CString szOut;
	if(pMenu)
	{
		if(pMenu->_wAttr & MF_POPUP)
			szOut.Format(L"팝업메뉴\xd\xa"
				L"=====================\xd\xa%s\xd\xa", pMenu->_szName);
		else
			szOut.Format(L"일반메뉴\xd\xa=====================\xd\xa"
			L"%s\xd\xaID : %d (0x%08X)\xd\xa", pMenu->_szName,
			pMenu->_wID, pMenu->_wID);

		if(pMenu->_wAttr & MF_CHECKED)
			szOut += L"체크됨\xd\xa";
		if(pMenu->_wAttr & MF_DISABLED)
			szOut += L"비활성\xd\xa";
		if(pMenu->_wAttr & MF_GRAYED)
			szOut += L"그레이\xd\xa";
	}
	GetDlgItem(IDC_EDT_INFO)->SetWindowText(szOut);
		
	*pResult = 0;
}

void CResViewMenu::OnTvnDeleteitemTvMenu(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if(pNMTreeView->itemOld.lParam)
	{
		delete ((PMENU_ITEM)pNMTreeView->itemOld.lParam);
		pNMTreeView->itemOld.lParam = 0;
	}
	*pResult = 0;
}

void CResViewMenu::OnDestroy()
{
	CDialogEx::OnDestroy();
	Tv()->DeleteAllItems();
}
