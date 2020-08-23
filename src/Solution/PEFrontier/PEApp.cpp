#include "stdafx.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "PEApp.h"
#include "PEFrame.h"
#include "PEChild.h"
#include "PEDoc.h"
#include "PEView.Html.h"
#include "PEView.DAsm.h"
#include "PEView.Src.h"
#include "PEView.VMem.h"
#include "PEDlg.NewDbgPrj.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


#define GSZ_APP_TMP_CACHE	_T("%s\\Temp\\SymbolCache")

bool CPEApp::_STATIC_FIRST = true;
CPEApp G_PE_APP;

// CPEApp 생성
CPEApp::CPEApp()
{
	m_bHiColorIcons = TRUE;
	// CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("YHDWorks.PEFrontier.PEProbe.VersionDef"));
}

bool CPEApp::LoadSchema()
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

		CPEDoc::SCHEMA.Load(szXmlRes);
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

void CPEApp::LoadSymbolPath(PCTSTR pszSymPath)
{
	if (m_pszRegistryKey == NULL)
		return;

	HKEY hSecKey = GetSectionKey(_T("Settings"));
	if (hSecKey == NULL)
		return;

	if (pszSymPath != NULL)
	{
		int nLen = (int)_tcslen(pszSymPath);
		if (RegSetValueEx(hSecKey, _T("SymbolPath"), 0, RRF_RT_REG_SZ,
			(CONST PBYTE)pszSymPath, nLen * sizeof(TCHAR)) == ERROR_SUCCESS)
			m_szSymPath = pszSymPath;
	}
	else
	{
		TCHAR szSymPath[MAX_PATH];
		DWORD dwType = 0;
		DWORD dwCount = MAX_PATH * sizeof(TCHAR);
		LONG lResult = RegQueryValueEx(hSecKey, _T("SymbolPath"), NULL, &dwType, (LPBYTE)&szSymPath, &dwCount);
		if (lResult != ERROR_SUCCESS)
		{
			PWSTR pszAppData = NULL;
			SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &pszAppData);
			int nLen = (int)wsprintf(szSymPath, GSZ_APP_TMP_CACHE, pszAppData);
			RegSetValueEx(hSecKey, _T("SymbolPath"), 0, RRF_RT_REG_SZ,
				(CONST PBYTE)szSymPath, nLen * sizeof(TCHAR));
		}
		m_szSymPath = szSymPath;
	}
	RegCloseKey(hSecKey);
}

CMDIChildWndEx* CPEApp::AppendView(CDocument* pDoc, UINT nViewID, CRuntimeClass* pRCV)
{
	if (m_pDocManager == NULL)
		return NULL;

	CString szViewT, szViewT2;
	szViewT2.LoadString(nViewID);
	AfxExtractSubString(szViewT, szViewT2, CDocTemplate::fileNewName);

	CDocTemplate* pDocT = NULL;
	POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();
	while (pos)
	{
		CDocTemplate* pDocTtr = m_pDocManager->GetNextDocTemplate(pos);
		CString szTypeN;
		if (pDocTtr->GetDocString(szTypeN, CDocTemplate::fileNewName))
		{
			if (szViewT == szTypeN)
			{
				pDocT = pDocTtr;
				break;
			}
		}
	}
	if (pDocT == NULL)
		return NULL;

	CMultiDocTemplate* pMDocT = (CMultiDocTemplate*)pDocT;
	CFrameWnd* pFrame = ((CPEFrame*)m_pMainWnd)->GetActiveFrame();

	CCreateContext cc;
	cc.m_pCurrentDoc	 = pDoc;
	cc.m_pNewViewClass	 = pRCV;
	cc.m_pNewDocTemplate = pDocT;
	cc.m_pLastView		 = (pFrame != NULL) ? pFrame->GetActiveView() : NULL;
	cc.m_pCurrentFrame	 = pFrame->GetActiveFrame();

	CPEChild* pChildFrm = new CPEChild();
	if (!pChildFrm->LoadFrame(nViewID, WS_OVERLAPPEDWINDOW | FWS_PREFIXTITLE, m_pMainWnd, &cc))
	{
		AfxMessageBox(L"자식 윈도우 생성에 실패했습니다.");
		delete pChildFrm;
		return NULL;
	}
	pChildFrm->InitialUpdateFrame(pDoc, TRUE);
	return pChildFrm;
}

// CPEApp 메시지 처리기
BEGIN_MESSAGE_MAP(CPEApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CPEApp::OnAppAbout)
	// 표준 파일을 기초로 하는 문서 명령입니다.
	ON_COMMAND(ID_FILE_NEW, &CPEApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CPEApp::OnFileOpen)
END_MESSAGE_MAP()

CDocument* CPEApp::OpenDocumentFile(LPCTSTR lpszFileName)
{
	CPEDoc* pDoc = (CPEDoc*)CWinAppEx::OpenDocumentFile(lpszFileName);
	((CPEFrame*)m_pMainWnd)->GetProjectMgr()->SetPEDoc(pDoc);
	return pDoc;
}

void CPEApp::OnFileNew()
{
	if (_STATIC_FIRST)
	{
		_STATIC_FIRST = false;
		return;
	}

	CNewDbgPrj ndp;
	if (ndp.DoModal() != IDOK)
		return;

	CString szPrjPath = CPEDoc::CreateNewProject(ndp.m_szPrjName, 
		ndp.m_szPrjPath, (ndp.m_bIsProc == TRUE), ndp.m_szExePath);
	if (m_pDocManager != NULL)
	{
		POSITION pos = m_pDocManager->GetFirstDocTemplatePosition();
		CDocTemplate* pDocT = m_pDocManager->GetNextDocTemplate(pos);
		ASSERT(pDocT != NULL);
		ASSERT_KINDOF(CDocTemplate, pDocT);
		pDocT->OpenDocumentFile(szPrjPath);
	}
}

// CPEApp 초기화
BOOL CPEApp::InitInstance()
{
	INITCOMMONCONTROLSEX ic;
	ic.dwSize = sizeof(ic);
	ic.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&ic);
	CWinAppEx::InitInstance();

	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	if (!LoadSchema())
		return FALSE;

	AfxEnableControlContainer();
	EnableTaskbarInteraction();

	SetRegistryKey(_T("YHD Works Co."));
	LoadStdProfileSettings(4);  // MRU를 포함하여 표준 INI 파일 옵션을 로드합니다.
	InitContextMenuManager();
	InitKeyboardManager();
	InitTooltipManager();

	m_fnt.CreateFont(15, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET,
		0, 0, 0, DEFAULT_PITCH | FF_DONTCARE, _T("Courier New"));;
	LoadSymbolPath();

	CMFCToolTipInfo ttPrm;
	ttPrm.m_bVislManagerTheme = TRUE;
	G_PE_APP.GetTooltipManager()->SetTooltipParams
		(AFX_TOOLTIP_TYPE_ALL, RUNTIME_CLASS(CMFCToolTipCtrl), &ttPrm);

	CRuntimeClass* arRTC[] =
	{
		RUNTIME_CLASS(CFrontierView), RUNTIME_CLASS(CDAsmView), 
		RUNTIME_CLASS(CSrcView), RUNTIME_CLASS(CVMemView)
	};
	UINT arIds[] = { IDR_PEFrontierTYPE, IDR_PEDASM_TYPE, IDR_PESOURCE_TYPE, IDR_PETYPE_VIEW_VMEM };
	for (int i = 0; i < sizeof(arIds) / sizeof(UINT); i++)
	{
		CMultiDocTemplate* pDocT = new CMultiDocTemplate
		(
			arIds[i], RUNTIME_CLASS(CPEDoc), RUNTIME_CLASS(CPEChild), arRTC[i]
		);
		if (!pDocT) return FALSE;
		AddDocTemplate(pDocT);
	}

	CPEFrame* pMainFrm = new CPEFrame();
	if (!pMainFrm || !pMainFrm->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrm;
		return FALSE;
	}
	m_pMainWnd = pMainFrm;

	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

	if (!ProcessShellCommand(cmdInfo))
		return FALSE;
	pMainFrm->ShowWindow(m_nCmdShow);
	pMainFrm->UpdateWindow();

	return TRUE;
}

int CPEApp::ExitInstance()
{
	AfxOleTerm(FALSE);
	return CWinAppEx::ExitInstance();
}

BOOL CPEApp::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_SYSCHAR && pMsg->wParam == VK_F10)
		pMsg->message = WM_CHAR;
	return CWinAppEx::PreTranslateMessage(pMsg);
}
////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////
// CAboutDlg 대화 상자.
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
void CPEApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CPEApp 사용자 지정 로드/저장 메서드
void CPEApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_PRJMGR_DEFAULT);
	GetContextMenuManager()->AddMenu(L"프로젝트 바탕 팝업", IDR_POPUP_PRJMGR_BACKBROUND);
	GetContextMenuManager()->AddMenu(L"프로젝트 모듈 팝업", IDR_POPUP_PRJMGR_MODULE);
}

void CPEApp::LoadCustomState()
{
}

void CPEApp::SaveCustomState()
{
}
// CPEApp 메시지 처리기
////////////////////////////////////////////////////////////////////////////////////////////////////
