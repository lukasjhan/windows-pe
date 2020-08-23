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
	// 표준 파일을 기초로 하는 문서 명령입니다.
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
END_MESSAGE_MAP()


// CPdbApp 생성
CPdbApp::CPdbApp()
{
	SetAppID(_T("YHD_Works.PDBParser.Release_1"));
	// TODO: 
}
CPdbApp theApp;


// CPdbApp 초기화
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
	LoadStdProfileSettings(4);  // MRU를 포함하여 표준 INI 파일 옵션을 로드합니다.

	CSingleDocTemplate* pDocTmpl = new CSingleDocTemplate
	(
		IDR_MAINFRAME, RUNTIME_CLASS(CPdbDoc), 
		RUNTIME_CLASS(CMainFrm), RUNTIME_CLASS(CPdbTreeView));
	if (!pDocTmpl)
		return FALSE;
	AddDocTemplate(pDocTmpl);

	// 표준 셸 명령, DDE, 파일 열기에 대한 명령줄을 구문 분석합니다.
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);
	// DDE Execute 열기를 활성화합니다.
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


// CPdbApp 메시지 처리기
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

// 대화 상자를 실행하기 위한 응용 프로그램 명령입니다.
void CPdbApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CPdbApp 메시지 처리기
