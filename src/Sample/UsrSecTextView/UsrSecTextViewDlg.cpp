#include "stdafx.h"
#include "UsrSecTextView.h"
#include "UsrSecTextViewDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


#pragma data_seg(".YHD")
BYTE g_szOutTest[65536 * 2] = 
{
	0xFF, 0xFE, 'Y', 0x00, 'H', 0x00, 'D', 0x00, ' ', 0x00,
	'S', 0x00, 'e', 0x00, 'c', 0x00, 't', 0x00, 'i', 0x00, 'o', 0x00, 'n', 0x00,
	0x00, 0x00,
};
#pragma data_seg()

// CUsrSecTextViewDlg 대화 상자
CUsrSecTextViewDlg::CUsrSecTextViewDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CUsrSecTextViewDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUsrSecTextViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CUsrSecTextViewDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CUsrSecTextViewDlg 메시지 처리기
BOOL CUsrSecTextViewDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	// TODO: 
	CRect rc;
	GetClientRect(rc);
	CRect rcView;
	GetDlgItem(IDC_EDIT_VIEW)->GetWindowRect(rcView);
	ScreenToClient(rcView);
	m_siMargin.cx = rc.Width() - rcView.Width();
	m_siMargin.cy = rc.Height() - rcView.Height();

	if (*((PWORD)g_szOutTest) == 0xFEFF)
		SetDlgItemText(IDC_EDIT_VIEW, (PCWSTR)g_szOutTest);
	else
	{
		DWORD dwUtf8 = *((PDWORD)g_szOutTest);
		dwUtf8 &= 0xFFFFFF;
		if (dwUtf8 == 0xBFBBEF)
		{
			PSTR pszUtf8 = (PSTR)&g_szOutTest[3];
			int nLen = MultiByteToWideChar(CP_UTF8, 0, pszUtf8, -1, NULL, 0);
			if (nLen > 0)
			{
				PWSTR pszUniv = new WCHAR[nLen];
				MultiByteToWideChar(CP_UTF8, 0, (PSTR)g_szOutTest, -1, pszUniv, nLen);
				SetDlgItemText(IDC_EDIT_VIEW, pszUniv);
				delete[] pszUniv;
			}
		}
		else
		{
			USES_CONVERSION;
			SetDlgItemText(IDC_EDIT_VIEW, A2CW((PSTR)g_szOutTest));
		}
	}

	return TRUE;
}

void CUsrSecTextViewDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CUsrSecTextViewDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CUsrSecTextViewDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CUsrSecTextViewDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (GetDlgItem(IDC_EDIT_VIEW)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDIT_VIEW)->SetWindowPos
			(NULL, 0, 0, cx - m_siMargin.cx, cy - m_siMargin.cy, SWP_NOMOVE | SWP_NOZORDER);
	}
}
