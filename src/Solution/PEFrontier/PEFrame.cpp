#include "stdafx.h"
#include "PEApp.h"
#include "PEFrame.h"
#include "PEDoc.h"
#include "PEView.DAsm.h"
#include "PEDlg.EnvConfig.h"
#include "PEDlg.ExceptInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static UINT indicators[] =
{
	ID_SEPARATOR,		// 상태 줄 표시기
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};


// CPEFrame
IMPLEMENT_DYNAMIC(CPEFrame, CMDIFrameWndEx)
BEGIN_MESSAGE_MAP(CPEFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_WM_SETTINGCHANGE()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_DEBUG_MESSAGE, OnDebugMessage)
	ON_MESSAGE(WM_DEBUG_HALT, OnDebugHalt)
	ON_MESSAGE(WM_DEBUG_EXCEPT, OnDebugExcept)
	ON_MESSAGE(WM_PROJECT_EVENT, OnProjectEvent)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, OnToolbarCreateNew)
	ON_COMMAND(ID_WINDOW_MANAGER, OnWindowManager)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, OnViewCustomize)
	ON_COMMAND(IDM_ENV_CONFIG, OnEnvConfig)
	ON_COMMAND(IDM_EXPLR_EDIT_GOTO, OnExplrEditGoto)
	ON_UPDATE_COMMAND_UI(IDM_EXPLR_EDIT_GOTO, OnUpdateExplrEditGoto)
	ON_COMMAND(IDM_EXPLR_GO_ADDR, OnExplrGotoAddr)
	ON_UPDATE_COMMAND_UI(IDM_EXPLR_GO_ADDR, OnUpdateExplrGotoAddr)
END_MESSAGE_MAP()


// CPEFrame 생성/소멸
CPEFrame::CPEFrame()
{
}

CPEFrame::~CPEFrame()
{
}

void CPEFrame::UpdateDAsmView(PBYTE pUpdtAddr, DWORD dwThrId, PVOID pctx)
{
	PDBG_MODULE pdm = GetPEDoc()->FindModule(pUpdtAddr);
	if (pdm == NULL)
	{
		return;
	}
	m_dockRegstr.UpdateThreadCtx(dwThrId, pctx);
	UpdateDAsmView(pdm, (DWORD)(pUpdtAddr - pdm->ImageBase));
}

void CPEFrame::UpdateDAsmView(PDBG_MODULE pdm, DWORD dwUptRVA)
{
	CDAsmView* pView = (CDAsmView*)m_wndPrjMgr.FindViewInst(pdm->ImageBase);
	if (pView != NULL)
	{
		pView->ScrollDAsmLine(dwUptRVA, DASM_SETPOS);
		pView->GetParentFrame()->ActivateFrame();
	}
	else
	{
		m_wndPrjMgr.AppenDAsmView(pdm, pdm->ImageBase + dwUptRVA);
	}
}

void CPEFrame::UpdateDAsmItem(PDBG_MODULE pdm, DWORD dwUptRVA)
{
	CDAsmView* pView = (CDAsmView*)m_wndPrjMgr.FindViewInst(pdm->ImageBase);
	if (pView != NULL)
		pView->UpdateDAsmItem(dwUptRVA);
}

void CPEFrame::UpdateDockPanes()
{
	m_dockRegstr.UpdateThreadCtx(0, NULL);
	m_dockCallStack.UpdateCallStack(false, 0, NULL);
	m_dockThread.EnableControls(3);
}

BOOL CPEFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWndEx::PreCreateWindow(cs) )
		return FALSE;

	return TRUE;
}

BOOL CPEFrame::CreateDockingWindows()
{
	BOOL bNameValid = FALSE;
	CString strClassView;

	if (!m_dockModule.Create(_T("모듈"), this, CRect(0, 0, 200, 200), TRUE, ID_DOCK_MODULE,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("모듈 뷰 창을 만들지 못했습니다.\n");
		return FALSE; // 만들지 못했습니다.
	}
	m_mapDock.insert(std::make_pair(ID_DOCK_MODULE, m_dockModule.GetSafeHwnd()));

	if (!m_dockThread.Create(_T("스레드"), this, CRect(0, 0, 200, 200), TRUE, ID_DOCK_THREAD,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("스레드 뷰 창을 만들지 못했습니다.\n");
		return FALSE; // 만들지 못했습니다.
	}
	m_mapDock.insert(std::make_pair(ID_DOCK_THREAD, m_dockThread.GetSafeHwnd()));

	if (!m_dockCallStack.Create(_T("호출 스택"), this, CRect(0, 0, 200, 200), TRUE, ID_DOCK_CALLSTACK,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("호출 스택 창을 만들지 못했습니다.\n");
		return FALSE; // 만들지 못했습니다.
	}
	m_mapDock.insert(std::make_pair(ID_DOCK_CALLSTACK, m_dockCallStack.GetSafeHwnd()));

	// 클래스 뷰를 만듭니다.
	//bNameValid = strClassView.LoadString(IDS_CLASS_VIEW);
	//ASSERT(bNameValid);
	//if (!m_wndClassView.Create(strClassView, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_CLASSVIEW, 
	//	WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	//{
	//	TRACE0("클래스 뷰 창을 만들지 못했습니다.\n");
	//	return FALSE; // 만들지 못했습니다.
	//}
	//m_mapDock.insert(std::make_pair(ID_VIEW_CLASSVIEW, m_wndClassView.GetSafeHwnd()));

	// 디버그 관리창을 만듭니다.
	CString strFileView;
	bNameValid = strFileView.LoadString(IDS_DOCK_PRJMGR);
	ASSERT(bNameValid);
	if (!m_wndPrjMgr.Create(strFileView, this, CRect(0, 0, 200, 200), TRUE, ID_DOCK_PRJMGR, 
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT| CBRS_FLOAT_MULTI))
	{
		TRACE0("디버그 관리창을 만들지 못했습니다.\n");
		return FALSE; // 만들지 못했습니다.
	}
	m_mapDock.insert(std::make_pair(ID_DOCK_PRJMGR, m_wndPrjMgr.GetSafeHwnd()));
	//m_wndPrjMgr.DockToFrameWindow(CBRS_ALIGN_RIGHT);

	// 출력 창을 만듭니다.
	CString strOutputWnd;
	bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
	ASSERT(bNameValid);
	if (!m_dockOutput.Create(strOutputWnd, this, CRect(0, 0, 100, 100), TRUE, ID_DOCK_OUTPUT,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
	{
		TRACE0("출력 창을 만들지 못했습니다.\n");
		return FALSE; // 만들지 못했습니다.
	}
	m_mapDock.insert(std::make_pair(ID_DOCK_OUTPUT, m_dockOutput.GetSafeHwnd()));

	if (!m_dockRegstr.Create(_T("레지스터"), this, CRect(0, 0, 100, 100), TRUE, ID_DOCK_REGISTER,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("스레드 뷰 창을 만들지 못했습니다.\n");
		return FALSE; // 만들지 못했습니다.
	}
	m_mapDock.insert(std::make_pair(ID_DOCK_REGISTER, m_dockRegstr.GetSafeHwnd()));

	if (!m_dockBreak.Create(_T("중단점"), this, CRect(0, 0, 100, 100), TRUE, ID_DOCK_BREAK,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("스레드 뷰 창을 만들지 못했습니다.\n");
		return FALSE; // 만들지 못했습니다.
	}
	m_mapDock.insert(std::make_pair(ID_DOCK_BREAK, m_dockBreak.GetSafeHwnd()));

	if (!m_dockBookMark.Create(_T("책갈피"), this, CRect(0, 0, 100, 100), TRUE, IDR_DOCK_BOOKMARK,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
	{
		TRACE0("스레드 뷰 창을 만들지 못했습니다.\n");
		return FALSE; // 만들지 못했습니다.
	}
	m_mapDock.insert(std::make_pair(IDR_DOCK_BOOKMARK, m_dockBookMark.GetSafeHwnd()));

	return TRUE;
}

// CPEFrame 진단
#ifdef _DEBUG
void CPEFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CPEFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG


// CPEFrame 메시지 처리기
int CPEFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	CMDITabInfo mip;
	mip.m_style = CMFCTabCtrl::STYLE_3D_VS2005;	// 사용할 수 있는 다른 스타일...
	mip.m_bActiveTabCloseButton = TRUE;			// FALSE로 설정하여 탭 영역 오른쪽에 닫기 단추를 배치합니다.
	mip.m_bTabIcons				= FALSE;		// TRUE로 설정하여 MDI 탭의 문서 아이콘을 활성화합니다.
	mip.m_bAutoColor			= TRUE;			// FALSE로 설정하여 MDI 탭의 자동 색 지정을 비활성화합니다.
	mip.m_bDocumentMenu			= TRUE;			// 탭 영역의 오른쪽 가장자리에 문서 메뉴를 활성화합니다.
	EnableMDITabbedGroups(TRUE, mip);

	if (!m_mbMain.Create(this))
	{
		TRACE0("메뉴 모음을 만들지 못했습니다.\n");
		return -1; 
	}
	m_mbMain.SetPaneStyle(m_mbMain.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	// 메뉴 모음을 활성화해도 포커스가 이동하지 않게 합니다.
	CMFCPopupMenu::SetForceMenuFocus(FALSE);
	if (!m_tbMain.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE |
		CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_tbMain.LoadToolBar(IDR_MAINFRAME_256))
	{
		TRACE0("표준 도구 모음을 만들지 못했습니다.\n");
		return -1;
	}
	CString szName;
	ASSERT(szName.LoadString(IDS_TOOLBAR_STANDARD));
	m_tbMain.SetWindowText(szName);
	ASSERT(szName.LoadString(IDS_TOOLBAR_CUSTOMIZE));
	m_tbMain.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, szName);


	if (!m_tbDebug.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE |
		CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_tbDebug.LoadToolBar(IDR_TOOLBAR_DEBUG))
	{
		TRACE0("디버그 도구 모음을 만들지 못했습니다.\n");
		return -1;
	}
	m_tbDebug.SetWindowText(L"디버그");
	//m_tbDebug.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, L"디버그");


	if (!m_tbExplr.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE |
		CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_tbExplr.LoadToolBar(IDR_TOOLBAR_EXPLORE))
	{
		TRACE0("탐색 도구 모음을 만들지 못했습니다.\n");
		return -1;
	}
	m_tbExplr.SetWindowText(L"탐색");
	//m_tbExplr.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, L"탐색");
	int nImgIdx = GetCmdMgr()->GetCmdImage(IDM_EXPLR_EDIT_GOTO, FALSE);
	CMFCToolBarEditBoxButton eb(IDM_EXPLR_EDIT_GOTO, nImgIdx);
	m_tbExplr.ReplaceButton(IDM_EXPLR_EDIT_GOTO, eb);

	if (!m_sbMain.Create(this))
	{
		TRACE0("상태 표시줄을 만들지 못했습니다.\n");
		return -1;      // 만들지 못했습니다.
	}
	m_sbMain.SetIndicators(indicators, sizeof(indicators) / sizeof(UINT));

	// TODO: 도구 모음 및 메뉴 모음을 도킹할 수 없게 하려면 이 다섯 줄을 삭제하십시오.
	m_mbMain.EnableDocking(CBRS_ALIGN_ANY);
	m_tbMain.EnableDocking(CBRS_ALIGN_ANY);
	m_tbDebug.EnableDocking(CBRS_ALIGN_ANY);
	m_tbExplr.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_mbMain);
	DockPane(&m_tbExplr);
	m_dockManager.DockPaneLeftOf(&m_tbDebug, &m_tbExplr);
	m_dockManager.DockPaneLeftOf(&m_tbMain, &m_tbDebug);

	// Visual Studio 2005 스타일 도킹 창 동작을 활성화합니다.
	CDockingManager::SetDockingMode(DT_SMART);
	// Visual Studio 2005 스타일 도킹 창 자동 숨김 동작을 활성화합니다.
	EnableAutoHidePanes(CBRS_ALIGN_ANY);

	// 메뉴 항목 이미지를 로드합니다(표준 도구 모음에 없음).
	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, IDB_MENU_IMAGES_24);

	// 도킹 창을 만듭니다.
	if (!CreateDockingWindows())
	{
		TRACE0("도킹 창을 만들지 못했습니다.\n");
		return -1;
	}

	m_wndPrjMgr.EnableDocking(CBRS_ALIGN_ANY);
	//m_wndClassView.EnableDocking(CBRS_ALIGN_ANY);
	m_dockModule.EnableDocking(CBRS_ALIGN_ANY);
	m_dockThread.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndPrjMgr);

	CDockablePane* pTabbedBar = NULL;
	//m_wndClassView.AttachToTabWnd(&m_wndPrjMgr, DM_SHOW, TRUE, &pTabbedBar);
	m_dockModule.AttachToTabWnd(&m_wndPrjMgr, DM_SHOW, TRUE, &pTabbedBar);
	m_dockThread.AttachToTabWnd(&m_wndPrjMgr, DM_SHOW, TRUE, &pTabbedBar);

	m_dockOutput.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_dockOutput);
	m_dockRegstr.EnableDocking(CBRS_ALIGN_ANY);
	m_dockBreak.EnableDocking(CBRS_ALIGN_ANY);

	pTabbedBar = NULL;
	m_dockRegstr.AttachToTabWnd(&m_dockOutput, DM_SHOW, TRUE, &pTabbedBar);
	m_dockBreak.AttachToTabWnd(&m_dockOutput, DM_SHOW, TRUE, &pTabbedBar);

	m_dockBookMark.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_dockBookMark);

	m_dockCallStack.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_dockCallStack);

	// 모든 사용자 인터페이스 요소를 그리는 데 사용하는 비주얼 관리자를 설정합니다.
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
	// 향상된 창 관리 대화 상자를 활성화합니다.
	EnableWindowsDialog(ID_WINDOW_MANAGER, ID_WINDOW_MANAGER, TRUE);
	// 도구 모음 및 도킹 창 메뉴 바꾸기를 활성화합니다.
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, szName, ID_VIEW_TOOLBAR);
	// 빠른(<Alt> 키를 누른 채 끌기) 도구 모음 사용자 지정을 활성화합니다.
	CMFCToolBar::EnableQuickCustomization();
	// 창 제목 표시줄에서 문서 이름 및 응용 프로그램 이름의 순서를 전환합니다.
	// 문서 이름이 축소판 그림과 함께 표시되므로 작업 표시줄의 기능성이 개선됩니다.
	ModifyStyle(0, FWS_PREFIXTITLE);
	return 0;
}

void CPEFrame::OnClose()
{
	CPEDoc* pDoc = GetPEDoc();
	if (pDoc != NULL && pDoc->UnderDebug())
	{
		if (AfxMessageBox(_T("디버깅을 종료하시겠습니까?"), MB_YESNO) != IDYES)
			return;
	}
	CMDIFrameWndEx::OnClose();
}

BOOL CPEFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (GetActiveFrame() != this)
		return CMDIFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	CPEDoc* pDoc = GetPEDoc();
	if (pDoc == NULL)
		return CMDIFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	return pDoc->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CPEFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

void CPEFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = 
		new CMFCToolBarsCustomizeDialog(this, TRUE);
	pDlgCust->Create();
}

LRESULT CPEFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	BOOL bNameValid;
	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void CPEFrame::OnUpdateFrameMenu(HMENU hMenuAlt)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CMDIFrameWndEx::OnUpdateFrameMenu(hMenuAlt);
}

void CPEFrame::OnEnvConfig()
{
	// TODO:
	CEnvConfig cfg(((CPEApp*)AfxGetApp())->GetSymbolPath());
	if (cfg.DoModal() == IDOK)
	{
		((CPEApp*)AfxGetApp())->SetSymbolPath(cfg.GetSymbolPath());
	}
}


LRESULT CPEFrame::OnDebugMessage(WPARAM wParam, LPARAM lParam)
{
	DWORD dwDbgCode = (DWORD)wParam;
	CString szEvMsg;
	switch (dwDbgCode)
	{
		case CREATE_PROCESS_DEBUG_EVENT:
		case EXIT_PROCESS_DEBUG_EVENT:
		{
			PDBG_PROCESS pdp = (PDBG_PROCESS)lParam;
			int nOpt = -1;
			if (dwDbgCode == CREATE_PROCESS_DEBUG_EVENT)
			{
				nOpt = DBG_NOTI_APPEND;
				szEvMsg.Format(L"Process %s(%d) created at: 0x%I64x, base:0x%I64x",
					pdp->ModuleName, pdp->ProcessId, pdp->MainThread->StartAddr, pdp->ImageBase);
				m_dockBreak.UpdateBrkPntList();
				m_dockBookMark.UpdateBookMarkList();
			}
			else
			{
				nOpt = (pdp != NULL) ? DBG_NOTI_REMOVE : DBG_NOTI_CLEAR;
				szEvMsg.Format(L"Process %s(0x%x) terminated with exit code=0x%X",
					pdp->ModuleName, pdp->ProcessId, pdp->ExitCode);
			}
			m_dockModule.UpdateItem(nOpt, pdp);
			if (pdp->MainThread != NULL)
				m_dockThread.UpdateItem(nOpt, pdp->MainThread);
			m_wndPrjMgr.UpdateProcess(nOpt, pdp);
		}
		break;

		case CREATE_THREAD_DEBUG_EVENT:
		case EXIT_THREAD_DEBUG_EVENT:
		{
			PDBG_THREAD pdt = (PDBG_THREAD)lParam;
			int nOpt = -1;
			if (dwDbgCode == CREATE_THREAD_DEBUG_EVENT)
			{
				nOpt = DBG_NOTI_APPEND;
				szEvMsg.Format(L"Thread 0x%x (Id: %d) created at: 0x%x", 
					pdt->Thread, pdt->ThreadId, pdt->StartAddr);
			}
			else
			{
				nOpt = (pdt != NULL) ? DBG_NOTI_REMOVE : DBG_NOTI_CLEAR;
				if (nOpt != DBG_NOTI_CLEAR)
					szEvMsg.Format(L"Thread %d exited with code: 0x%X", pdt->ThreadId, pdt->ExitCode);
			}
			m_dockThread.UpdateItem(nOpt, pdt);
		}
		break;

		case LOAD_DLL_DEBUG_EVENT:
		case UNLOAD_DLL_DEBUG_EVENT:
		{
			PDBG_MODULE pdm = (PDBG_MODULE)lParam;
			int nOpt = -1;
			if (dwDbgCode == LOAD_DLL_DEBUG_EVENT)
			{
				nOpt = DBG_NOTI_APPEND;
				szEvMsg.Format(L"%s loaded at 0x%I64x", pdm->ModuleName, pdm->ImageBase);
				m_dockBreak.UpdateBrkPntList();
				m_dockBookMark.UpdateBookMarkList();
			}
			else
			{
				nOpt = (pdm != NULL) ? DBG_NOTI_REMOVE : DBG_NOTI_CLEAR;
				if (nOpt != DBG_NOTI_CLEAR)
					szEvMsg.Format(L"Module %s unloaded", pdm->ModuleName);
			}
			m_dockModule.UpdateItem(nOpt, pdm);
			if (nOpt != DBG_NOTI_CLEAR)
				m_wndPrjMgr.UpdateModule(nOpt, pdm);
		}
		break;

		case OUTPUT_DEBUG_STRING_EVENT:
		{
			PBYTE pBuff = (PBYTE)lParam;
			WORD fUnicode = *((PWORD)pBuff);
			pBuff += sizeof(WORD);
			if (fUnicode)
				szEvMsg = (PCWSTR)pBuff;
			else
			{
				USES_CONVERSION;
				szEvMsg = A2CW((PSTR)pBuff);
			}
		}
		break;

		case RIP_EVENT:
		{
			LPRIP_INFO pri = (LPRIP_INFO)lParam;
			szEvMsg.Format(szEvMsg, L"Error: %d, Type: %d\xd\xa", pri->dwError, pri->dwType);
		}
		break;

		case EXCEPTION_DEBUG_EVENT:
		{
			LPEXCEPTION_DEBUG_INFO pei = (LPEXCEPTION_DEBUG_INFO)lParam;
			DWORD dwExpCode = pei->ExceptionRecord.ExceptionCode;
			PBYTE pExptAddr = (PBYTE)pei->ExceptionRecord.ExceptionAddress;

			CPEDoc* pDoc = (CPEDoc*)GetActiveFrame()->GetActiveDocument();
			PDBG_MODULE pdm = pDoc->FindModule(pExptAddr);
			bool bIs32Bit = (pdm != NULL) ? pdm->Is32Bit : pDoc->GetDbgProcess()->Is32Bit;

			szEvMsg.Format(L"0x%s에 %s의 %s 예외가 있습니다. %s",
							PEPlus::GetAddrForm(bIs32Bit, pExptAddr),
							(pdm != NULL) ? pdm->ModuleName : L"알 수 없는 모듈",
							(pei->dwFirstChance) ? L"첫 번째" : L"처리되지 않은",
							PEPlus::GetErrMsg(dwExpCode));
		}
		break;
	}
	if (!szEvMsg.IsEmpty())
		m_dockOutput.PrintOutputStr(dwDbgCode, szEvMsg);

	return 0;
}

LRESULT CPEFrame::OnDebugHalt(WPARAM wParam, LPARAM lParam)
{
	DWORD dwHaltCode = (DWORD)wParam;
	DWORD dwThreadId = (DWORD)lParam;

	CPEDoc* pDoc = GetPEDoc();
	DBG_THREAD_MAP::iterator it = pDoc->GetThreadMap()->find(dwThreadId);
	if (it == pDoc->GetThreadMap()->end())
	{
		AfxMessageBox(L"해당 스레드가 없습니다.");
		return S_FALSE;
	}

	PBYTE pHaltAddr = it->second->InstPtr;
	PDBG_MODULE pdm = pDoc->FindModule(pHaltAddr);
	if (pdm == NULL)
	{
		AfxMessageBox(L"해당 모듈이 없습니다.");
		return S_FALSE;
	}

	CDAsmView* pView = (CDAsmView*)m_wndPrjMgr.FindViewInst(pdm->ImageBase);
	if (pView != NULL)
	{
		pView->UpdateBreakPoint(dwHaltCode, pHaltAddr, dwThreadId);
		pView->GetParentFrame()->ActivateFrame();
	}
	else
	{
		m_wndPrjMgr.AppenDAsmView(pdm, pHaltAddr, dwThreadId);
	}
	if (dwHaltCode == EXCEPTION_BREAKPOINT || dwHaltCode == EXCEPTION_SINGLE_STEP)
		m_dockBreak.UpdateBreakItem(pHaltAddr);
	m_dockCallStack.UpdateCallStack(pdm->Is32Bit, it->second->ThreadId, &it->second->CallStack);
	DBG_CALL_STACK::reverse_iterator ics = it->second->CallStack.rbegin();
	if (ics != it->second->CallStack.rend())
	{
		m_dockRegstr.UpdateThreadCtx(it->second->ThreadId, &(*ics)->Context);
	}
	m_dockThread.EnableControls(2);
	return S_OK;
}

LRESULT CPEFrame::OnDebugExcept(WPARAM wParam, LPARAM lParam)
{
	LPEXCEPTION_DEBUG_INFO pei = (LPEXCEPTION_DEBUG_INFO)wParam;
	DWORD dwExpCode = pei->ExceptionRecord.ExceptionCode;
	if (OnDebugHalt(dwExpCode, lParam) == S_FALSE)
		return IDIGNORE;

	CExceptInfo dlg(pei);
	int nDlgRet = (int)dlg.DoModal();
	if (nDlgRet != IDABORT)
		UpdateDockPanes();
	return (LRESULT)nDlgRet;
}

LRESULT CPEFrame::OnProjectEvent(WPARAM wParam, LPARAM lParam)
{
	CPEDoc* pDoc = (CPEDoc*)lParam;
	switch (wParam)
	{
		case PRJ_NOTI_OPEN:
			m_wndPrjMgr.InitView(pDoc);
			m_dockBreak.InitBreakPoint(pDoc->GetNlBrkMap());
			m_dockBookMark.InitBookMarks(pDoc->GetNlBmMap());
			break;
		case PRJ_NOTI_CLOSED:
			m_wndPrjMgr.UninitView(pDoc);
		break;

		case PRJ_DBG_BEGUN:
		case PRJ_DBG_ENDED:
		{
			BOOL bEnable;
			COLORREF clrBkg, clrTxt;
			if (wParam == PRJ_DBG_BEGUN)
			{
				bEnable = TRUE;
				clrBkg = RGB(255, 100, 0), clrTxt = RGB(255, 255, 255);
				m_dockRegstr.InitThreadCtx(pDoc->GetDbgProcess()->Is32Bit);
			}
			else
			{
				bEnable = FALSE;
				clrBkg = GetSysColor(COLOR_3DLIGHT), clrTxt = RGB(0, 0, 0);
				m_dockRegstr.UninitThreadCtx();
			}
			m_dockCallStack.EnableControls(bEnable);
			m_dockModule.EnableControls(bEnable);
			m_dockThread.EnableControls(bEnable);
			for (int i = 0; i < m_sbMain.GetCount(); i++)
			{
				m_sbMain.SetPaneBackgroundColor(i, clrBkg);
				m_sbMain.SetPaneTextColor(i, clrTxt);
			}
			if (wParam == PRJ_DBG_BEGUN)
				FlashWindow(TRUE);
		}
		break;
	}

	return 0;
}

void CPEFrame::OnExplrEditGoto()
{
	CMFCToolBarEditBoxButton* pEdit = dynamic_cast<CMFCToolBarEditBoxButton*>
		(m_tbExplr.GetButton(m_tbExplr.CommandToIndex(IDM_EXPLR_EDIT_GOTO)));
	if (pEdit == NULL)
		return;

	CString sz;
	pEdit->GetEditBox()->GetWindowText(sz);
	sz.Remove(_T(' '));
	if (sz.IsEmpty())
		return;

	CPEDoc* pDoc = (CPEDoc*)GetActiveFrame()->GetActiveDocument();
	bool bIsRva = false;
	int nPos = sz.Find(_T(':'));
	if (nPos >= 0)
	{
		bIsRva = (_tcsnicmp(sz, _T("RVA"), nPos) == 0);
		sz = sz.Mid(nPos + 1);
	}

	bool bIs32 = (bIsRva) ? true : pDoc->GetDbgProcess()->Is32Bit;
	PBYTE pGotoAddr = 0;
	try
	{
		pGotoAddr = PEPlus::AddrStr2Value(bIs32, sz);
	}
	catch(HRESULT /*hr*/)
	{
		AfxMessageBox(L"주소 형식이 잘못되었습니다.");
		pEdit->GetEditBox()->SetFocus();
		return;
	}

	if (bIsRva)
	{
		CView* pTmp = GetActiveFrame()->GetActiveView();
		if (pTmp == NULL || !pTmp->IsKindOf(RUNTIME_CLASS(CDAsmView)))
		{
			AfxMessageBox(L"활성화된 디스어셈블 모듈이 없습니다.");
			return;
		}
		CDAsmView* pView = (CDAsmView*)pTmp;
		pView->UpdateBreakPoint(0, (DWORD)pGotoAddr);
	}
	else
	{
		PDBG_MODULE pdm = pDoc->FindModule(pGotoAddr);
		if (pdm == NULL)
		{
			sz.Format(L"번지 0x%p에 해당하는 모듈이 없습니다.", pGotoAddr);
			AfxMessageBox(sz);
			return;
		}
		CDAsmView* pView = (CDAsmView*)m_wndPrjMgr.FindViewInst(pdm->ImageBase);
		if (pView != NULL)
		{
			pView->UpdateBreakPoint(0, (DWORD)(pGotoAddr - pdm->ImageBase));
			pView->GetParentFrame()->ActivateFrame();
		}
		else
		{
			m_wndPrjMgr.AppenDAsmView(pdm, pGotoAddr);
		}
	}
}

void CPEFrame::OnUpdateExplrEditGoto(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_wndPrjMgr.GetViewInstCount() > 0);
}

void CPEFrame::OnExplrGotoAddr()
{
	OnExplrEditGoto();
}

void CPEFrame::OnUpdateExplrGotoAddr(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;
	if (m_wndPrjMgr.GetViewInstCount() > 0)
	{
		CMFCToolBarEditBoxButton* pEdit = dynamic_cast<CMFCToolBarEditBoxButton*>
			(m_tbExplr.GetButton(m_tbExplr.CommandToIndex(IDM_EXPLR_EDIT_GOTO)));
		if (pEdit != NULL)
		{
			if (pEdit->GetEditBox()->GetWindowTextLength() > 0)
				bEnable = TRUE;
		}
	}
	pCmdUI->Enable(bEnable);
}
