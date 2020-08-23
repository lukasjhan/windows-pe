#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "PEExplorer.h"
#include "MainFrm.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CPEExplorerApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CPEExplorerApp::OnAppAbout)
END_MESSAGE_MAP()
CPEExplorerApp::CPEExplorerApp()
{
	SetAppID(_T("PEExplorer.AppID.NoVersion"));
}
CPEExplorerApp theApp;

BOOL CPEExplorerApp::InitInstance()
{
	INITCOMMONCONTROLSEX ic;
	ic.dwSize = sizeof(ic);
	ic.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&ic);

	CWinApp::InitInstance();
	if (!AfxOleInit())
	{
		AfxMessageBox(L"OLE 초기화 실패");
		return FALSE;
	}
	if (!LoadSchema())
		return FALSE;

	EnableTaskbarInteraction(FALSE);
	SetRegistryKey(_T("YHD Works Co."));

	CMainFrame* pFrame = new CMainFrame();
	if (!pFrame)
		return FALSE;
	m_pMainWnd = pFrame;
	pFrame->LoadFrame(IDR_MAINFRAME, 
		WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);
	pFrame->SetIcon(LoadIcon(IDR_MAINFRAME), FALSE);
	pFrame->ShowWindow(SW_SHOW);
	pFrame->UpdateWindow();
	return TRUE;
}

int CPEExplorerApp::ExitInstance()
{
	AfxOleTerm(FALSE);
	return CWinApp::ExitInstance();
}

bool CPEExplorerApp::LoadSchema()
{
	HGLOBAL	hXdfXml = NULL;
	try
	{
		HRSRC hXdfRes = ::FindResource
		(
			AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_PE_FORMAT), _T("XML")
		);
		if (hXdfRes == NULL)
			throw HRESULT_FROM_WIN32(GetLastError());
		hXdfXml = ::LoadResource(AfxGetInstanceHandle(), hXdfRes);
		if (hXdfXml == NULL)
			throw HRESULT_FROM_WIN32(GetLastError());
		DWORD dwSize = ::SizeofResource(AfxGetApp()->m_hInstance, hXdfRes);
		PSTR pData = (PSTR)::LockResource(hXdfXml);

		USES_CONVERSION;
		CString szXmlRes;
		PTSTR pszBuff = szXmlRes.GetBufferSetLength(dwSize * 2);
		memcpy(pszBuff, A2T(pData), dwSize * 2);
		szXmlRes.ReleaseBufferSetLength(dwSize * 2);
		::UnlockResource(hXdfXml);

		PEAnals::SCHEMA.Load(szXmlRes);
		return true;
	}
	catch (HRESULT e)
	{
		CString szMsg;
		szMsg.Format(_T("Error occurred, code=0x%08X"), e);
		AfxMessageBox(szMsg);
	}
	catch (PCTSTR e)
	{
		AfxMessageBox(e);
	}
	if (hXdfXml != NULL)
		::FreeResource(hXdfXml);
	return false;
}




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

void CPEExplorerApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CPEExplorerApp 메시지 처리기



