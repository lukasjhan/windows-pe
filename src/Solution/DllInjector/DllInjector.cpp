#include "stdafx.h"
#include "DllInjector.h"
#include "DllInjectorDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDllInjectorApp
BEGIN_MESSAGE_MAP(CDllInjectorApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CDllInjectorApp 생성
CDllInjectorApp::CDllInjectorApp()
{
}
CDllInjectorApp theApp;


// CDllInjectorApp 초기화
BOOL CDllInjectorApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	CWinApp::InitInstance();

	CShellManager *pShellManager = new CShellManager;
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	SetRegistryKey(_T("YHD Works Co."));

	CDllInjectorDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
	}
	else if (nResponse == IDCANCEL)
	{
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "경고: 대화 상자를 만들지 못했으므로 응용 프로그램이 예기치 않게 종료됩니다.\n");
		TRACE(traceAppMsg, 0, "경고: 대화 상자에서 MFC 컨트롤을 사용하는 경우 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS를 수행할 수 없습니다.\n");
	}
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}
	return FALSE;
}

