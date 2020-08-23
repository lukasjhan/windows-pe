#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "PdbParser.h"
#include "MainFrm.h"
#include "PdbDoc.h"
#include "PdbTreeView.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif

// CPdbApp
BEGIN_MESSAGE_MAP(CPdbApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CPdbApp::OnAppAbout)
	// ǥ�� ������ ���ʷ� �ϴ� ���� ����Դϴ�.
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CPdbApp ����
CPdbApp::CPdbApp()
{
	SetAppID(_T("YHD_Works.PDBParser.Release_1"));
	// TODO: 
}
CPdbApp theApp;


// CPdbApp �ʱ�ȭ
BOOL CPdbApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();
	::CoInitialize(NULL);

	m_fnt.CreateFont(14, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET,
		0, 0, 0, DEFAULT_PITCH | FF_DONTCARE, _T("Courier New"));;

	EnableTaskbarInteraction(FALSE);
	SetRegistryKey(_T("YHD Works Co."));
	LoadStdProfileSettings(4);  // MRU�� �����Ͽ� ǥ�� INI ���� �ɼ��� �ε��մϴ�.

	CSingleDocTemplate* pDocTmpl = new CSingleDocTemplate
	(
		IDR_MAINFRAME, RUNTIME_CLASS(CPdbDoc), 
		RUNTIME_CLASS(CMainFrm), RUNTIME_CLASS(CPdbTreeView));
	if (!pDocTmpl)
		return FALSE;
	AddDocTemplate(pDocTmpl);

	// ǥ�� �� ���, DDE, ���� ���⿡ ���� ������� ���� �м��մϴ�.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// DDE Execute ���⸦ Ȱ��ȭ�մϴ�.
	EnableShellOpen();
	//RegisterShellFileTypes(TRUE);

	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	m_pMainWnd->DragAcceptFiles();

	return TRUE;
}

int CPdbApp::ExitInstance()
{
	::CoUninitialize();
	return CWinApp::ExitInstance();
}


// CPdbApp �޽��� ó����
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
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

// ��ȭ ���ڸ� �����ϱ� ���� ���� ���α׷� ����Դϴ�.
void CPdbApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CPdbApp �޽��� ó����
