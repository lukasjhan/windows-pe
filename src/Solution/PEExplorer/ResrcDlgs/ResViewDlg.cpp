#include "..\stdafx.h"
#include "..\PEExplorer.h"
#include "ResViewDlg.h"
#include "afxdialogex.h"


#pragma pack(push, 1)
struct DLGINITSTRUCT
{
	WORD nIDC;
	WORD message;
	DWORD dwSize;
};
struct DLGTEMPLATEEX
{
	WORD dlgVer;
	WORD signature;
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	WORD cDlgItems;
	short x;
	short y;
	short cx;
	short cy;

	// Everything else in this structure is variable length,
	// and therefore must be determined dynamically

	// sz_Or_Ord menu;			// name or ordinal of a menu resource
	// sz_Or_Ord windowClass;	// name or ordinal of a window class
	// WCHAR title[titleLen];	// title string of the dialog box
	// short pointsize;			// only if DS_SETFONT is set
	// short weight;			// only if DS_SETFONT is set
	// short bItalic;			// only if DS_SETFONT is set
	// WCHAR font[fontLen];		// typeface name, if DS_SETFONT is set
};
typedef DLGTEMPLATEEX* LPDLGTEMPLATEEX;

struct DLGITEMTEMPLATEEX
{
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	short x;
	short y;
	short cx;
	short cy;
	DWORD id;

	// Everything else in this structure is variable length,
	// and therefore must be determined dynamically

	// sz_Or_Ord windowClass;	// name or ordinal of a window class
	// sz_Or_Ord title;			// title string or ordinal of a resource
	// WORD extraCount;			// bytes following creation data
};
typedef DLGITEMTEMPLATEEX* LPDLGITEMTEMPLATEEX;
#pragma pack(pop)



IMPLEMENT_DYNAMIC(CResViewDlg, CDialogEx)

CResViewDlg::CResViewDlg(PBYTE pImgStart, CWnd* pParent /*=NULL*/)
	: CDialogEx(CResViewDlg::IDD, pParent)
{
	m_pImgStart = pImgStart;
	m_bIsExt = false;
}

CResViewDlg::~CResViewDlg()
{
}

void CResViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CResViewDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_SHOW_PARSE, OnBnClickedBtnShowParse)
	ON_BN_CLICKED(IDC_BTN_SHOW_DIALOG, OnBnClickedBtnShowDialog)
END_MESSAGE_MAP()

// CResViewDlg 메시지 처리기입니다.
BOOL CResViewDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CListCtrl* pLv = (CListCtrl*)GetDlgItem(IDC_LV_CTRL);
	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	pLv->SetExtendedStyle(pLv->GetExtendedStyle() | dwExStyle);

	pLv->InsertColumn(0, L"ID", LVCFMT_LEFT, 30);
	pLv->InsertColumn(1, L"클래스", LVCFMT_LEFT, 100);
	pLv->InsertColumn(2, L"타이틀", LVCFMT_LEFT, 120);
	pLv->InsertColumn(3, L"X, Y", LVCFMT_CENTER, 60);
	pLv->InsertColumn(4, L"W, H", LVCFMT_CENTER, 60);
	pLv->InsertColumn(5, L"스타일", LVCFMT_CENTER, 75);
	pLv->InsertColumn(6, L"스타일2", LVCFMT_CENTER, 75);
	pLv->InsertColumn(7, L"생성인자", LVCFMT_LEFT, 40);
	pLv->InsertColumn(8, L"헬프ID", LVCFMT_LEFT, 40);

	ParseDialogTemplate(pLv);

	return TRUE;
}

void CResViewDlg::OnBnClickedBtnShowParse()
{
	PBYTE	pImgDlg = m_pImgStart;

	CString sz;
	bool	bHasFont = false;
	DWORD	dwHelpID = 0;
	DWORD	dwStyle = 0, dwStyleEx = 0;
	INT		nCtrlCnt = 0;
	INT		x = 0, y = 0, w = 0, h = 0;
	TCHAR	szWndMenu[MAX_PATH] = { 0 };
	TCHAR	szWndClass[MAX_PATH] = { 0 };
	TCHAR	szCaption[MAX_PATH] = { 0 };

	if (m_bIsExt)
	{
		LPDLGTEMPLATEEX pDlg = (LPDLGTEMPLATEEX)pImgDlg;
		pImgDlg += sizeof(DLGTEMPLATEEX);

		bHasFont = ((pDlg->style & DS_SETFONT) != 0);
		nCtrlCnt = pDlg->cDlgItems;
		dwHelpID = pDlg->helpID;
		dwStyle = pDlg->style;
		dwStyleEx = pDlg->exStyle;
		x = pDlg->x;
		y = pDlg->y;
		w = pDlg->cx;
		h = pDlg->cy;
	}
	else
	{
		LPDLGTEMPLATE pDlg = (LPDLGTEMPLATE)pImgDlg;
		pImgDlg += sizeof(DLGTEMPLATE);

		nCtrlCnt = pDlg->cdit;
		dwStyle = pDlg->style;
		dwStyleEx = pDlg->dwExtendedStyle;
		x = pDlg->x;
		y = pDlg->y;
		w = pDlg->cx;
		h = pDlg->cy;
	}

	PWORD pwIter = (PWORD)pImgDlg;
	// 대화상자 메뉴
	if (*pwIter == 0)
		pwIter++;
	else if (*pwIter == 0xFFFF)
	{
		pwIter++;
		*((PWORD)szWndMenu) = *pwIter;
		pwIter++;
	}
	else
	{
		INT nUniLen = (INT)wcslen((PWSTR)pwIter);
		wcscpy_s(szWndMenu, (PWSTR)pwIter);
		pwIter += (nUniLen + 1);
	}

	// 대화상자 클래스
	if (*pwIter)
	{
		INT nUniLen = (INT)wcslen((PWSTR)pwIter);
		wcscpy_s(szWndClass, (PWSTR)pwIter);
		pwIter += (nUniLen + 1);
	}
	else
	{
		pwIter++;
	}

	// 대화상자 캡션
	if (*pwIter)
	{
		INT nUniLen = (INT)wcslen((PWSTR)pwIter);
		wcscpy_s(szCaption, (PWSTR)pwIter);
		pwIter += (nUniLen + 1);
	}
	else
	{
		pwIter++;
	}

	// 폰트
	HFONT hFont;
	if (bHasFont)
	{
		INT nPointSize = *pwIter;
		pwIter++;
		INT nWeight = *pwIter;
		pwIter++;

		BYTE byItalic = (BYTE)((*pwIter) & 0xFF);
		BYTE byChCode = (BYTE)(((*pwIter) & 0xFF00) >> 8);
		pwIter++;

		TCHAR szFont[MAX_PATH];
		INT nUniLen = (INT)wcslen((PWSTR)pwIter);
		wcscpy_s(szFont, (PWSTR)pwIter);
		pwIter += (nUniLen + 1);

		LOGFONT lf;
		memset(&lf, 0x00, sizeof(LOGFONT));
		lf.lfItalic = byItalic;
		lf.lfCharSet = byChCode;
		lf.lfWeight = nWeight;
		lf.lfHeight = nPointSize;
		hFont = ::CreateFontIndirect(&lf);
	}

	LONG lBaseUnit = GetDialogBaseUnits();
	LONG lXUnit = (lBaseUnit & 0x0000FFFF);
	LONG lYUnit = (lBaseUnit & 0xFFFF0000) >> 16;
	x = MulDiv(x, lXUnit, 4);
	w = MulDiv(w, lXUnit, 4);
	y = MulDiv(y, lYUnit, 8);
	h = MulDiv(h, lYUnit, 8);
	if (dwStyle & WS_CAPTION)
		h += GetSystemMetrics(SM_CYCAPTION);

	if (szWndClass[0] == 0)
		wcscpy_s(szWndClass, L"TEST_DLG");

	WNDCLASS wndclass;
	wcscpy_s(szWndClass, L"TEST_DLG");


	////////////////////////////////////////////////////////////////////////////
	//윈도우즈 클래스 등록
	////////////////////////////////////////////////////////////////////////////
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = AfxGetInstanceHandle();
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szWndClass;
	if (!RegisterClass(&wndclass))
	{
		//::MessageBox(NULL, TEXT("Window Class Registeration Failure!!!"),
		//		"TEST_MENU", MB_ICONERROR) ;
		//return;
	}

	HWND hWnd = CreateWindowEx(dwStyleEx, szWndClass, szCaption,
		dwStyle, x, y, w, h, GetSafeHwnd(),
		NULL, AfxGetInstanceHandle(), NULL);

	WORD	wItemID = 0;
	PBYTE	pParam = NULL;
	for (INT i = 0; i<nCtrlCnt; i++)
	{
		PBYTE pCtrlIter = (PBYTE)ROUND_UP((DWORD_PTR)pwIter, 4);

		if (m_bIsExt)
		{
			LPDLGITEMTEMPLATEEX pItem = (LPDLGITEMTEMPLATEEX)pCtrlIter;
			pCtrlIter += sizeof(DLGITEMTEMPLATEEX);

			dwHelpID = pItem->helpID;
			dwStyle = pItem->style;
			dwStyleEx = pItem->exStyle;
			x = pItem->x;
			y = pItem->y;
			w = pItem->cx;
			h = pItem->cy;
			wItemID = (WORD)pItem->id;
		}
		else
		{
			LPDLGITEMTEMPLATE	 pItem = (LPDLGITEMTEMPLATE)pCtrlIter;
			pCtrlIter += sizeof(DLGITEMTEMPLATE);

			dwHelpID = 0;
			dwStyle = pItem->style;
			dwStyleEx = pItem->dwExtendedStyle;
			x = pItem->x;
			y = pItem->y;
			w = pItem->cx;
			h = pItem->cy;
			wItemID = (WORD)pItem->id;
		}

		if (m_bIsExt)
			pwIter = (PWORD)ROUND_UP((DWORD_PTR)pCtrlIter, 4);
		else
			pwIter = (PWORD)pCtrlIter;

		// 컨트롤 클래스
		if (*pwIter == 0)
		{
			szWndClass[0] = 0x00;
			pwIter++;
		}
		else if (*pwIter == 0xFFFF)
		{
			pwIter++;
			switch (*pwIter)
			{
			case 0x0080: wcscpy_s(szWndClass, L"BUTTON"); break;
			case 0x0081: wcscpy_s(szWndClass, L"EDIT"); break;
			case 0x0082: wcscpy_s(szWndClass, L"STATIC"); break;
			case 0x0083: wcscpy_s(szWndClass, L"LISTBOX"); break;
			case 0x0084: wcscpy_s(szWndClass, L"SCROLLBAR"); break;
			case 0x0085: wcscpy_s(szWndClass, L"COMBOBOX"); break;
			default: *((PWORD)szWndClass) = *pwIter; break;
				TOOLTIPS_CLASS;
			}
			pwIter++;
		}
		else
		{
			INT nUniLen = (INT)wcslen((PWSTR)pwIter);
			wcscpy_s(szWndClass, (PWSTR)pwIter);
			pwIter += (nUniLen + 1);
		}

		// 컨트롤 타이틀
		if (*pwIter == 0)
		{
			szCaption[0] = 0x00;
			pwIter++;
		}
		else if (*pwIter == 0xFFFF)
		{
			pwIter++;
			*((PWORD)szCaption) = *pwIter;
			pwIter++;
		}
		else
		{
			INT nUniLen = (INT)wcslen((PWSTR)pwIter);
			wcscpy_s(szCaption, (PWSTR)pwIter);
			pwIter += (nUniLen + 1);
		}

		// 생성 데이터
		if (*pwIter == 0)
		{
			pParam = NULL;
			pwIter++;
		}
		else
		{
			WORD wItemLen = *pwIter;
			pParam = (PBYTE)pwIter;
			PBYTE pTemp = pParam;
			pTemp += wItemLen;
			pwIter = (PWORD)pTemp;
		}

		x = MulDiv(x, lXUnit, 4);
		w = MulDiv(w, lXUnit, 4);
		y = MulDiv(y, lYUnit, 8);
		h = MulDiv(h, lYUnit, 8);

		HWND hCtrl = CreateWindowEx(dwStyleEx, szWndClass, szCaption,
			dwStyle, x, y, w, h, hWnd, (HMENU)wItemID,
			AfxGetInstanceHandle(), pParam);
	}
	::ShowWindow(hWnd, SW_NORMAL);
	::UpdateWindow(hWnd);
}

void CResViewDlg::OnBnClickedBtnShowDialog()
{
	LPDLGTEMPLATE pResDlg = (LPDLGTEMPLATE)m_pImgStart;
	DialogBoxIndirect(AfxGetInstanceHandle(),
		pResDlg, GetSafeHwnd(), (DLGPROC)DlgProc);
}

LRESULT CALLBACK CResViewDlg::WndProc(HWND hWnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			::DestroyWindow(hWnd);
			return 0;
		}
		break;
	}

	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK CResViewDlg::DlgProc(HWND hDlg, UINT uMsg,
	WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			::EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}

	return FALSE;
}

void CResViewDlg::ParseDialogTemplate(CListCtrl* pLv)
{
	WORD wVer = *((PWORD)m_pImgStart);
	WORD wSig = *((PWORD)(m_pImgStart + sizeof(WORD)));
	if (wSig == 0xFFFF && wVer == 1)
		m_bIsExt = true;

	CString sz;
	DWORD	dwHelpID = 0;
	INT		nCtrlCnt = 0;
	bool	bHasFont = false;
	PBYTE	pImgDlg = m_pImgStart;
	if (m_bIsExt)
	{
		LPDLGTEMPLATEEX pDlg = (LPDLGTEMPLATEEX)pImgDlg;
		pImgDlg += sizeof(DLGTEMPLATEEX);

		nCtrlCnt = pDlg->cDlgItems;
		dwHelpID = pDlg->helpID;
		bHasFont = ((pDlg->style & DS_SETFONT) != 0);
		sz.Format(L"(%d, %d)", pDlg->x, pDlg->y);
		SetDlgItemText(IDC_STC_XY, sz);
		sz.Format(L"(%d, %d)", pDlg->cx, pDlg->cy);
		SetDlgItemText(IDC_STC_WH, sz);
		sz.Format(L"0x%08X", pDlg->style);
		SetDlgItemText(IDC_STC_STYLE, sz);
		sz.Format(L"0x%08X", pDlg->exStyle);
		SetDlgItemText(IDC_STC_STYLEEX, sz);
	}
	else
	{
		LPDLGTEMPLATE pDlg = (LPDLGTEMPLATE)pImgDlg;
		pImgDlg += sizeof(DLGTEMPLATE);

		bHasFont = ((pDlg->style & DS_SETFONT) != 0);
		nCtrlCnt = pDlg->cdit;
		sz.Format(L"(%d, %d)", pDlg->x, pDlg->y);
		SetDlgItemText(IDC_STC_XY, sz);
		sz.Format(L"(%d, %d)", pDlg->cx, pDlg->cy);
		SetDlgItemText(IDC_STC_WH, sz);
		sz.Format(L"0x%08X", pDlg->style);
		SetDlgItemText(IDC_STC_STYLE, sz);
		sz.Format(L"0x%08X", pDlg->dwExtendedStyle);
		SetDlgItemText(IDC_STC_STYLEEX, sz);
	}

	PWORD pwIter = (PWORD)pImgDlg;
	for (int i = 0; i < 3; i++)
	{
		if (*pwIter > 0)
		{
			if (i < 2 && *pwIter == 0xFFFF)
			{
				pwIter++;
				SetDlgItemInt(IDC_STC_MENU + i, *pwIter);
				pwIter++;
			}
			else
			{
				sz = (PCWSTR)pwIter;
				SetDlgItemText(IDC_STC_MENU + i, sz);
				pwIter += (sz.GetLength() + 1);
			}
		}
		else
			pwIter++;
	}

	if (m_bIsExt)
		sz.Format(L"확장 대화상자 정보 (헬프ID : %d)", dwHelpID);
	else
		sz.Format(L"표준 대화상자 정보");
	SetDlgItemText(IDC_GRP_DLG, sz);

	if (bHasFont)
	{
		SetDlgItemInt(IDC_STC_POINT, *pwIter);
		pwIter++;
		if (m_bIsExt)
		{
			SetDlgItemInt(IDC_STC_WEIGHT, *pwIter);
			pwIter++;
			SetDlgItemInt(IDC_STC_ITALYC, (*pwIter) & 0xFF);
			sz.Format(L"0x%02X", ((*pwIter) & 0xFF00) >> 8);
			SetDlgItemText(IDC_STC_CHAR, sz);
			pwIter++;
		}

		CString szFont = (PCWSTR)pwIter;
		pwIter += (szFont.GetLength() + 1);
		sz.Format(L"폰트 (%s)", szFont);
		SetDlgItemText(IDC_GRP_FONT, sz);
	}
	else
		SetDlgItemText(IDC_GRP_FONT, L"폰트 (정보 없음)");

	CString szID, szCls, szTitle, szXY, szWH;
	CString szStyle, szStyleEx, szParam, szHelpID;
	for (INT i = 0; i<nCtrlCnt; i++)
	{
		PBYTE pCtrlIter = (PBYTE)ROUND_UP((DWORD_PTR)pwIter, 4);

		if (m_bIsExt)
		{
			LPDLGITEMTEMPLATEEX pItem = (LPDLGITEMTEMPLATEEX)pCtrlIter;
			pCtrlIter += sizeof(DLGITEMTEMPLATEEX);
			szID.Format(L"%d", pItem->id);					pLv->InsertItem(i, szID);
			szXY.Format(L"(%d, %d)", pItem->x, pItem->y);	pLv->SetItemText(i, 3, szXY);
			szWH.Format(L"(%d, %d)", pItem->cx, pItem->cy);	pLv->SetItemText(i, 4, szWH);
			szStyle.Format(L"0x%08X", pItem->style);		pLv->SetItemText(i, 5, szStyle);
			szStyleEx.Format(L"0x%08X", pItem->exStyle);	pLv->SetItemText(i, 6, szStyleEx);
			szHelpID.Format(L"%d", pItem->helpID);			pLv->SetItemText(i, 8, szHelpID);
		}
		else
		{
			LPDLGITEMTEMPLATE pItem = (LPDLGITEMTEMPLATE)pCtrlIter;
			pCtrlIter += sizeof(DLGITEMTEMPLATE);
			szID.Format(L"%d", pItem->id);							pLv->InsertItem(i, szID);
			szXY.Format(L"(%d, %d)", pItem->x, pItem->y);			pLv	->SetItemText(i, 3, szXY);
			szWH.Format(L"(%d, %d)", pItem->cx, pItem->cy);			pLv->SetItemText(i, 4, szWH);
			szStyle.Format(L"0x%08X", pItem->style);				pLv->SetItemText(i, 5, szStyle);
			szStyleEx.Format(L"0x%08X", pItem->dwExtendedStyle);	pLv->SetItemText(i, 6, szStyleEx);
			szHelpID = L"-";										pLv->SetItemText(i, 8, szHelpID);
		}

		if (m_bIsExt)
			pwIter = (PWORD)ROUND_UP((DWORD_PTR)pCtrlIter, 4);
		else
			pwIter = (PWORD)pCtrlIter;
		// 컨트롤 클래스
		if (*pwIter == 0)
		{
			szCls = L"";
			pwIter++;
		}
		else if (*pwIter == 0xFFFF)
		{
			pwIter++;
			switch (*pwIter)
			{
				case 0x0080: szCls = L"BUTTON"; break;
				case 0x0081: szCls = L"EDIT"; break;
				case 0x0082: szCls = L"STATIC"; break;
				case 0x0083: szCls = L"LISTBOX"; break;
				case 0x0084: szCls = L"SCROLLBAR"; break;
				case 0x0085: szCls = L"COMBOBOX"; break;
				default: szCls.Format(L"0x%04X", *pwIter); break;
			}
			pwIter++;
		}
		else
		{
			szCls = (PWSTR)pwIter;
			pwIter += (szCls.GetLength() + 1);
		}
		pLv->SetItemText(i, 1, szCls);

		// 컨트롤 타이틀
		if (*pwIter == 0)
		{
			szTitle = L"";
			pwIter++;
		}
		else if (*pwIter == 0xFFFF)
		{
			pwIter++;
			szTitle.Format(L"ID : %d (0x%04X)", *pwIter, *pwIter);
			pwIter++;
		}
		else
		{
			szTitle = (PWSTR)pwIter;
			pwIter += (szTitle.GetLength() + 1);
		}
		pLv->SetItemText(i, 2, szTitle);

		// 생성 데이터
		if (*pwIter == 0)
		{
			szParam = L"없음";
			pwIter++;
		}
		else
		{
			WORD wItemLen = *pwIter;
			szParam.Format(L"%d 바이트", *pwIter);
			PBYTE pTemp = (PBYTE)pwIter;
			pTemp += wItemLen;
			pwIter = (PWORD)pTemp;
		}
		pLv->SetItemText(i, 7, szParam);
	}
}
