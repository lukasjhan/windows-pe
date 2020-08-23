#include "stdafx.h"
#ifndef SHARED_HANDLERS
#include "PEApp.h"
#endif
#include "PEDoc.h"
#include <propkey.h>
#include "PEFrame.h"
#include "PEDlg.NewDbgPrj.h"
#include "PEView.DAsm.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif

PE_SCHEMA CPEDoc::SCHEMA;


// CPEDoc
IMPLEMENT_DYNCREATE(CPEDoc, CDocument)
BEGIN_MESSAGE_MAP(CPEDoc, CDocument)
	ON_COMMAND(IDM_DEBUG_START, &CPEDoc::OnDebugStart)
	ON_UPDATE_COMMAND_UI(IDM_DEBUG_START, &CPEDoc::OnUpdateDebugStart)
	ON_COMMAND(IDM_DEBUG_STOP, &CPEDoc::OnDebugStop)
	ON_UPDATE_COMMAND_UI(IDM_DEBUG_STOP, &CPEDoc::OnUpdateDebugStop)
	ON_COMMAND_RANGE(IDM_DEBUG_CONTINUE, IDM_DEBUG_STEP_OUT, &CPEDoc::OnDebugResume)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_DEBUG_CONTINUE, IDM_DEBUG_STEP_OUT, &CPEDoc::OnUpdateDebugResume)
	ON_COMMAND(ID_FILE_SAVE_AS, &CPEDoc::OnFileSaveAs)
	ON_COMMAND_RANGE(IDM_DEBUG_DELETE_ALL_BREAK, IDM_DEBUG_DISABLE_ALL_BREAK, &CPEDoc::OnDebugAllBreakPoint)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_DEBUG_DELETE_ALL_BREAK, IDM_DEBUG_DISABLE_ALL_BREAK, &CPEDoc::OnUpdateDebugAllBreakPoint)
	ON_COMMAND_RANGE(IDM_DEBUG_BREAK_ENTRY, IDM_DEBUG_BREAK_RTLSTARTUP, &CPEDoc::OnDebugCheckStartup)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_DEBUG_BREAK_ENTRY, IDM_DEBUG_BREAK_RTLSTARTUP, &CPEDoc::OnUpdateDebugCheckStartup)
END_MESSAGE_MAP()

// CPEDoc 생성/소멸
CPEDoc::CPEDoc()
{
	m_bAutoDelete = FALSE;

	//m_dp.Parent	 = this;
	m_bAtDebug	  = false;
	m_bBrkEntPos  = true;
	m_bBrkRtlInit = true;
	m_pfnUsrThrSt = NULL;
	m_bBrkWinMain = true;
	m_pfnWinMain  = NULL;

	m_bBPOnceHit = false;
	m_dwHaltThId = 0;

	m_pNotiMap	= NULL;
	m_hNotiWnd	= NULL;

}

CPEDoc::~CPEDoc()
{
	for (DBG_MODULE_MAP::iterator it = m_mapMods.begin(); it != m_mapMods.end(); it++)
	{
		if (m_dp.ImageBase != it->second->ImageBase)
			delete it->second;
	}
	for (DBG_THREAD_MAP::iterator it = m_mapThrs.begin(); it != m_mapThrs.end(); it++)
		delete it->second;
	for (DBG_BREAK_MAP::iterator it = m_mapBrks.begin(); it != m_mapBrks.end(); it++)
		delete it->second;
}

BOOL CPEDoc::OnOpenDocument(PCTSTR pszPathName)
{
	try
	{
		LoadDebugProject(pszPathName);

		CPEFrame* pMainWnd = (CPEFrame*)AfxGetMainWnd();
		SetNoitWnd(pMainWnd->GetSafeHwnd(), pMainWnd->GetPaneMap());
		pMainWnd->SendMessage(WM_PROJECT_EVENT, PRJ_NOTI_OPEN, (LPARAM)this);
		return TRUE;
	}
	catch (HRESULT hr)
	{
		AfxMessageBox(PEPlus::GetErrMsg(hr));
		return FALSE;
	}
	catch (PCWSTR psz)
	{
		AfxMessageBox(psz);
		return FALSE;
	}
}

BOOL CPEDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	try
	{
		SaveDebugProject(lpszPathName);
		m_bModified = FALSE;
		return TRUE;
	}
	catch (HRESULT hr)
	{
		AfxMessageBox(PEPlus::GetErrMsg(hr));
		return FALSE;
	}
	catch (PCWSTR psz)
	{
		AfxMessageBox(psz);
		return FALSE;
	}
}

void CPEDoc::OnCloseDocument()
{
	OnDebugStop();
	m_bAutoDelete = TRUE;
	CPEFrame* pMainWnd = (CPEFrame*)AfxGetMainWnd();
	pMainWnd->SendMessage(WM_PROJECT_EVENT, PRJ_NOTI_CLOSED, (LPARAM)this);
	CDocument::OnCloseDocument();
}

void CPEDoc::Serialize(CArchive& ar)
{
}

#ifdef SHARED_HANDLERS	// 축소판 그림을 지원합니다.
void CPEDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 문서의 데이터를 그리려면 이 코드를 수정하십시오.
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// 검색 처리기를 지원합니다.
void CPEDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 문서의 데이터에서 검색 콘텐츠를 설정합니다.
	// 콘텐츠 부분은 ";"로 구분되어야 합니다.

	// 예: strSearchContent = _T("point;rectangle;circle;ole object;");
	SetSearchContent(strSearchContent);
}

void CPEDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}
#endif // SHARED_HANDLERS

#ifdef _DEBUG
void CPEDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CPEDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// CPEDoc 명령
void CPEDoc::OnDebugStart()
{
	if (m_bAtDebug)
		return;

	try
	{
		m_hevAck = CreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hevAck == NULL)
			throw HRESULT_FROM_WIN32(GetLastError());

		m_hthDbg = CreateThread(0, 0, __DebuggerThread, this, 0, &m_dwDbgThId);
		if (m_hthDbg == NULL)
			throw HRESULT_FROM_WIN32(GetLastError());
		WaitForSingleObject(m_hevAck, INFINITE);

		m_bAtDebug = true;
		SendMessage(m_hNotiWnd, WM_PROJECT_EVENT, PRJ_DBG_BEGUN, (LPARAM)this);
	}
	catch (HRESULT hr)
	{
		AfxMessageBox(PEPlus::GetErrMsg(hr));
	}
}

void CPEDoc::OnUpdateDebugStart(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(!m_bAtDebug);
}

void CPEDoc::OnDebugStop()
{
	if (!m_bAtDebug)
		return;

	PostThreadMessage(m_dwDbgThId, TM_CLOSE_DEBUGGER, 0, TRUE);
	WaitForSingleObject(m_hthDbg, INFINITE);
	DWORD dwExitCode = 0;
	GetExitCodeThread(m_hthDbg, &dwExitCode);
	if (dwExitCode)
		SendMessage(m_hNotiWnd, WM_DEBUG_MESSAGE, EXIT_PROCESS_DEBUG_EVENT, (LPARAM)&m_dp);

	for (DBG_THREAD_MAP::iterator it = m_mapThrs.begin(); it != m_mapThrs.end(); it++)
		delete it->second;
	m_mapThrs.clear();

	SendMessage(m_hNotiWnd, WM_DEBUG_MESSAGE, EXIT_THREAD_DEBUG_EVENT, 0);
	SendMessage(m_hNotiWnd, WM_DEBUG_MESSAGE, UNLOAD_DLL_DEBUG_EVENT, 0);

	if (m_hthDbg != NULL)
	{
		CloseHandle(m_hthDbg);
		m_hthDbg = NULL;
	}
	if (m_hevAck != NULL)
	{
		CloseHandle(m_hevAck);
		m_hevAck = NULL;
	}

	m_bAtDebug = false;
	SendMessage(m_hNotiWnd, WM_PROJECT_EVENT, PRJ_DBG_ENDED, (LPARAM)this);
}

void CPEDoc::OnUpdateDebugStop(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bAtDebug);
}

void CPEDoc::OnDebugResume(UINT uCmdId)
{
	DBG_THREAD_MAP::iterator it = m_mapThrs.find(m_dwHaltThId);
	if (it == m_mapThrs.end())
	{
		AfxMessageBox(L"해당 스레드가 없습니다.");
		return;
	}

	PBYTE pHaltAddr = it->second->InstPtr;
	PDBG_MODULE pdm = FindModule(pHaltAddr);
	if (pdm == NULL)
	{
		AfxMessageBox(L"해당 모듈이 없습니다.");
		return;
	}

	DWORD dwHaltRVA = (DWORD)(pHaltAddr - pdm->ImageBase);
	PVOID arPrms[3] = { (PVOID)dwHaltRVA, (PVOID)pdm, (PVOID)m_dwHaltThId };
	SendCommand(TM_BREAK_RESUME, (WPARAM)uCmdId, (LPARAM)arPrms);

	((CPEFrame*)AfxGetMainWnd())->UpdateDockPanes();
	UpdateAllViews(NULL);
	//nWantItem = m_nBrkItem;
	//m_nBrkItem = -1;
	//GetListCtrl().Update(nWantItem);
	//m_dwDbgThId = 0;
}

void CPEDoc::OnUpdateDebugResume(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_dwHaltThId > 0);
}

void CPEDoc::OnDebugAllBreakPoint(UINT uCmdId)
{
	for (DBG_BREAK_MAP::iterator it = m_mapBrks.begin(); it != m_mapBrks.end(); it++)
	{
		PDBG_BRKPNT pbp = it->second;
		if (pbp->IsTemp)
			continue;

		if (uCmdId == IDM_DEBUG_DELETE_ALL_BREAK)
		{

		}
		else
		{
			pbp->Active = false;
		}
	}
}

void CPEDoc::OnUpdateDebugAllBreakPoint(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_mapBrks.size() > 0);
}

void CPEDoc::OnDebugCheckStartup(UINT uCmdId)
{
	if (uCmdId == IDM_DEBUG_BREAK_ENTRY)
		m_bBrkEntPos = !m_bBrkEntPos;
	else
		m_bBrkRtlInit = !m_bBrkRtlInit;
	SetModifiedFlag();
}

void CPEDoc::OnUpdateDebugCheckStartup(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!m_bAtDebug);
	if (!m_bAtDebug)
	{
		if (pCmdUI->m_nID == IDM_DEBUG_BREAK_ENTRY)
			pCmdUI->SetCheck(m_bBrkEntPos);
		else
			pCmdUI->SetCheck(m_bBrkRtlInit);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////
