// APIHookTestDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "APIHookTest.h"
#include "APIHookTestDlg.h"
#include ".\apihooktestdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// ��ȭ ���� ������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ����

// ����
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CAPIHookTestDlg ��ȭ ����



CAPIHookTestDlg::CAPIHookTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAPIHookTestDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAPIHookTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAPIHookTestDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_OPEN, OnBnClickedBtnOpen)
	ON_BN_CLICKED(IDC_BTN_SAVE, OnBnClickedBtnSave)
END_MESSAGE_MAP()


// CAPIHookTestDlg �޽��� ó����

BOOL CAPIHookTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// �� ��ȭ ������ �������� �����մϴ�. ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	// �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	
	return TRUE;  // ��Ʈ�ѿ� ���� ��Ŀ���� �������� ���� ��� TRUE�� ��ȯ�մϴ�.
}

void CAPIHookTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸����� 
// �Ʒ� �ڵ尡 �ʿ��մϴ�. ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
// �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.
void CAPIHookTestDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�. 
HCURSOR CAPIHookTestDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAPIHookTestDlg::OnBnClickedBtnOpen()
{
	CString szFile;
	GetDlgItemText(IDC_EDT_FILE, szFile);
	HANDLE hFile = CreateFile(szFile, GENERIC_READ|GENERIC_WRITE, 
		FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(L"���� ���� ����");
		return;
	}

	DWORD dwFileSize = GetFileSize(hFile, NULL);
	if((INT)dwFileSize < 0)
	{
		DWORD dwErrCode = GetLastError();
		dwErrCode = 0;
	}
	PBYTE pszData = new BYTE[dwFileSize + 2];
	DWORD dwReadBytes = 0;
	BOOL bIsOK =  ReadFile(hFile, 
		pszData, dwFileSize, &dwReadBytes, NULL);
	if(!bIsOK)
	{
		DWORD dwErrCode = GetLastError();
		dwErrCode = 0;
	}
	pszData[dwReadBytes] = pszData[dwReadBytes + 1] = 0;
	SetDlgItemText(IDC_EDT_VIEW, (PCTSTR)pszData);
	delete pszData;
	CloseHandle(hFile);
}

void CAPIHookTestDlg::OnBnClickedBtnSave()
{
	CString szFile;
	GetDlgItemText(IDC_EDT_FILE, szFile);
	HANDLE hFile = CreateFile(szFile, GENERIC_ALL, 0, NULL, OPEN_ALWAYS, 0, NULL);
	if(hFile == INVALID_HANDLE_VALUE)
	{
		AfxMessageBox(L"���� ���� ����");
		return;
	}

	CString szData;
	GetDlgItemText(IDC_EDT_VIEW, szData);

	DWORD dwFileSize = (DWORD)szData.GetLength() * sizeof(TCHAR);
	DWORD dwWroteBytes = NULL;
	BOOL bIsOK =  WriteFile(hFile, (PCTSTR)szData, 
		dwFileSize, &dwWroteBytes, NULL);
	CloseHandle(hFile);
}
