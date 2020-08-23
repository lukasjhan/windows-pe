#pragma once
#include "PEDock.Project.h"
#include "PEDock.Module.h"
#include "PEDock.Thread.h"
#include "PEDock.Output.h"
#include "PEDock.Regstr.h"
#include "PEDock.BreakPnt.h"
#include "PEDock.BookMark.h"
#include "PEDock.CallStack.h"


class CPEDoc;
class CPEFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CPEFrame)

public:
	CPEFrame();
	virtual ~CPEFrame();

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CMFCMenuBar		m_mbMain;
	CMFCStatusBar	m_sbMain;
	CMFCToolBar		m_tbMain;
	CMFCToolBar		m_tbDebug;
	CMFCToolBar		m_tbExplr;

	DOCK_MAP		m_mapDock;
	CModuleDock		m_dockModule;
	CThreadDock		m_dockThread;
	COutputDock		m_dockOutput;
	CRegstrDock		m_dockRegstr;
	CBreakDock		m_dockBreak;
	CBookMarkDock	m_dockBookMark;
	CCallStackDock	m_dockCallStack;

	CProjectMgr		m_wndPrjMgr;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg void OnClose();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnEnvConfig();
	afx_msg LRESULT OnProjectEvent(WPARAM, LPARAM);
	afx_msg LRESULT OnDebugMessage(WPARAM, LPARAM);
	afx_msg LRESULT OnDebugHalt(WPARAM, LPARAM);
	afx_msg LRESULT OnDebugExcept(WPARAM, LPARAM);
	afx_msg void OnExplrEditGoto();
	afx_msg void OnUpdateExplrEditGoto(CCmdUI* pCmdUI);
	afx_msg void OnExplrGotoAddr();
	afx_msg void OnUpdateExplrGotoAddr(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();

public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void OnUpdateFrameMenu(HMENU hMenuAlt);

	CPEDoc* GetPEDoc() { return m_wndPrjMgr.GetPEDoc(); }
	HWND GetDockablePane(UINT nViewId)
	{
		DOCK_MAP::iterator it = m_mapDock.find(nViewId);
		if (it != m_mapDock.end())
			return it->second;
		return NULL;
	}
	DOCK_MAP* GetPaneMap() { return &m_mapDock; }
	void UpdateDockPanes();

	CProjectMgr* GetProjectMgr() { return &m_wndPrjMgr; }
	CModuleDock* GetModuleView() { return &m_dockModule; }
	CThreadDock* GetThreadView() { return &m_dockThread; }
	COutputDock* GetOutputView() { return &m_dockOutput; }
	CRegstrDock* GetRegstrView() { return &m_dockRegstr; }
	CBreakDock*	 GetBreakView () { return &m_dockBreak ; }
	CBookMarkDock* GetBookMarkView() { return &m_dockBookMark; }
	CCallStackDock* GetCallStackkView() { return &m_dockCallStack; }

	void UpdateDAsmView(PBYTE pUpdtAddr, DWORD dwThrId, PVOID pctx);
	void UpdateDAsmView(PDBG_MODULE pdm, DWORD dwUptRVA);
	void UpdateDAsmItem(PDBG_MODULE pdm, DWORD dwUptRVA);

};


