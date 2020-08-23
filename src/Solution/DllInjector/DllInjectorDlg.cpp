#include "stdafx.h"
#include "DllInjector.h"
#include "DllInjectorDlg.h"
#include "afxdialogex.h"
#include "ProcList.h"
#include "..\PEPlus\PEPlus.h"
#include <TlHelp32.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg 대화 상자
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	enum { IDD = IDD_ABOUTBOX };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);
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


// CDllInjectorDlg 대화 상자
CDllInjectorDlg::CDllInjectorDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDllInjectorDlg::IDD, pParent)
	, m_dwProcID(0)
	, m_szDllPath(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDllInjectorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDT_PROC_ID, m_dwProcID);
	DDX_Text(pDX, IDC_MEB_DLL_FILE, m_szDllPath);
}

HMODULE CDllInjectorDlg::FindModule(PCWSTR pszDllName, DWORD dwProcId)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcId);
	if (hSnapshot == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(PEPlus::GetErrMsg(GetLastError()));
		return NULL;
	}

	HMODULE hModule = NULL;
	MODULEENTRY32 me = { sizeof(me) };
	BOOL bMoreMods = Module32First(hSnapshot, &me);
	for (; bMoreMods; bMoreMods = Module32Next(hSnapshot, &me))
	{
		if (_wcsicmp(me.szModule, pszDllName) == 0)
		{
			hModule = (HMODULE)me.modBaseAddr;
			break;
		}
	}
	CloseHandle(hSnapshot);
	return hModule;
}


BEGIN_MESSAGE_MAP(CDllInjectorDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_PROC_LIST, &CDllInjectorDlg::OnClickedBtnProcList)
	ON_BN_CLICKED(IDC_BTN_INJECT, &CDllInjectorDlg::OnBnClickedBtnInject)
	ON_BN_CLICKED(IDC_BTN_EJECT, &CDllInjectorDlg::OnBnClickedBtnEject)
END_MESSAGE_MAP()

BOOL CDllInjectorDlg::OnInitDialog()
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

	return TRUE;
}

void CDllInjectorDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CDllInjectorDlg::OnPaint()
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

HCURSOR CDllInjectorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDllInjectorDlg::OnClickedBtnProcList()
{
	// TODO: 
	CProcList dlg;
	if (dlg.DoModal() != IDOK)
		return;

	m_dwProcID = dlg.GetProcID();
	UpdateData(FALSE);
}

void CDllInjectorDlg::OnBnClickedBtnInject()
{
	UpdateData();
	
	HRESULT hr = PEPlus::InjectModule(m_dwProcID, m_szDllPath);
	if (FAILED(hr))
		AfxMessageBox(PEPlus::GetErrMsg(hr));
	else
		AfxMessageBox(L"Module Injection SUCCESS!!!", MB_ICONINFORMATION);
}

void CDllInjectorDlg::OnBnClickedBtnEject()
{
	UpdateData();
	CString szDllName;
	int nPos = m_szDllPath.ReverseFind(L'\\');
	if (nPos > 0)
		szDllName = m_szDllPath.Mid(nPos + 1);
	else
		szDllName = m_szDllPath;

	HRESULT hr = PEPlus::EjectModule(m_dwProcID, szDllName);
	if (FAILED(hr))
		AfxMessageBox(PEPlus::GetErrMsg(hr));
	else
		AfxMessageBox(L"Module Ejection SUCCESS!!!", MB_ICONINFORMATION);
}
