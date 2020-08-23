#pragma once
#include "PEDoc.Types.h"
#include "PESchema.h"


class CPEDoc : public CDocument
{
public:	// 재정의입니다.
	virtual ~CPEDoc();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void OnCloseDocument();
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
	virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	afx_msg void OnDebugResume(UINT uCmdId);
	afx_msg void OnUpdateDebugResume(CCmdUI* pCmdUI);

protected:
	CPEDoc();
	DECLARE_DYNCREATE(CPEDoc)
	//afx_msg void OnFileSaveAs();
	afx_msg void OnDebugStart();
	afx_msg void OnUpdateDebugStart(CCmdUI* pCmdUI);
	afx_msg void OnDebugStop();
	afx_msg void OnUpdateDebugStop(CCmdUI* pCmdUI);
	afx_msg void OnDebugAllBreakPoint(UINT uCmdId);
	afx_msg void OnUpdateDebugAllBreakPoint(CCmdUI* pCmdUI);
	afx_msg void OnDebugCheckStartup(UINT uCmdId);
	afx_msg void OnUpdateDebugCheckStartup(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
	////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////////////////////
	//
protected:
#define DBGWAIT_NONE		0
#define DBGWAIT_TIMEOUT		1
#define DBGWAIT_WAITNEXT	2
#define DBGWAIT_CONTINUE	3
#define DBGWAIT_NOTHANDLE	4

	DBG_PROCESS		m_dp;
	DBG_THREAD_MAP	m_mapThrs;
	DBG_MODULE_MAP	m_mapMods;
	DBG_BREAK_MAP	m_mapBrks;
	DBG_NLBRK_MAP	m_mapNlBrks;

	bool			m_bBPOnceHit;
	DWORD			m_dwHaltThId;
	bool			m_bAtDebug;

	DBG_BKMARK_MAP	m_mapMarks;
	DBG_NLBM_MAP	m_mapNlBms;

	HANDLE			m_hevAck;
	HANDLE			m_hthDbg;
	DWORD			m_dwDbgThId;

	HWND			m_hNotiWnd;
	DOCK_MAP*		m_pNotiMap;

	CString			m_szPrjName;
	CString			m_szExeArgs;
	bool			m_bIsProc;

	bool			m_bBrkEntPos;
	bool			m_bBrkRtlInit;
	PBYTE			m_pfnUsrThrSt;
	bool			m_bBrkWinMain;
	PBYTE			m_pfnWinMain;

	PVOID			m_pInitPrm;

private:
	static DWORD WINAPI __DebuggerThread(PVOID pParam);
	HANDLE InitDebuger();
	void RunDebugger(HANDLE hevDbg);
	bool TermDebugger(HANDLE hevDbg);

	static PBYTE AllocBuff(bool bFromPE, HANDLE hRef, PBYTE pRefAddr, DWORD dwReadSize);
	static void FreeBuff(bool bFromPE, PBYTE pBuffAddr);

	bool HandleDebugEvent(DEBUG_EVENT& de);
	bool HandleBreakEvent(EXCEPTION_DEBUG_INFO& ei, PDBG_THREAD pdt);
	int HandleExceptEvent(EXCEPTION_DEBUG_INFO& ei, PDBG_THREAD pdt);
	DWORD HandleBreakMessage(MSG& msg);
	bool HandleUserMessage(MSG& msg, HANDLE hevDbg);

	PBYTE GetStackBase(HANDLE hThread, HANDLE hProcess, bool bWowProc);

#ifdef _WIN64
	PBYTE ParsePData(HANDLE hProcess, PDBG_MODULE pdm, PIMAGE_DATA_DIRECTORY pdd, DBG_CODES_MAP& codes);
#endif
	void ParseFuncPDB(HANDLE hProcess, PDBG_MODULE pdm, DBG_CODES_MAP& codes);
	void ParseFuncDirect(HANDLE hProcess, PDBG_MODULE pdm, DBG_CODES_MAP& codes);

	void GetCodeSections(HANDLE hProcess, PDBG_MODULE pdm);
	static CString GetImageName(HANDLE hProc, PVOID pImgName, HANDLE hFile, BOOL bUniCode);
	static PEPdb* LoadPdbRef(PBYTE pImgBase, DBG_PROCESS* pdp = NULL);
	static PBYTE GetPEHeader(HANDLE hProcess, PDBG_MODULE pdm);
	void BuildFunctoin(HANDLE hProcess, PDBG_MODULE pdm);
	static HRESULT GetProcHeapList(DBG_PROCESS* pdp, DBG_HEAP_MAP& mapHeap);
	HRESULT WalkProcess(DBG_HEAP_MAP& heaps, DBG_VMOBJ_MAP& vmos);

	void LoadProcessInfo(DWORD dwThreadId, CREATE_PROCESS_DEBUG_INFO& di);
	PDBG_THREAD LoadThreadInfo(DWORD dwThreadId, PDBG_PROCESS pdp, CREATE_THREAD_DEBUG_INFO& di);
	PDBG_MODULE LoadModuleInfo(PDBG_PROCESS pdp, LOAD_DLL_DEBUG_INFO& di);
	void MoveBreakPointFileToLoad(PCWSTR pszImgPath, PDBG_MODULE pdm);
	void MoveBookMarkFileToLoad(PCWSTR pszImgPath, PDBG_MODULE pdm);
	PBYTE GetRtlThreadStartFromNTDll(PBYTE pMapBase, PBYTE pImgBase);


	BYTE UpdateBreakPoint(int nOpt, PBYTE pBrkAddr, BYTE btCode);
	void UpdateThreadContext(HANDLE hThread, bool bIs32Bit, bool bInTrap, bool bIpBack);
	void SuspendDebuggeeProcess();
	void ResumeDebuggeeProcess();
	void SetBreakPoint(int nCodeIdx);
	PDBG_MODULE GetHaltDebugInfo(PBYTE pCtrlAddr, PDBG_THREAD pdt, bool bIsBreak = false);

	bool HandleDbgEXCEPTION(DWORD dwThreadId, EXCEPTION_DEBUG_INFO& ei);
	CString HandleAppEXCEPTION(DWORD dwThreadId, EXCEPTION_DEBUG_INFO& ei);

	bool ParseException(PIMAGE_SECTION_HEADER psh, PIMAGE_DATA_DIRECTORY pdd);
	void ParseTextSection(PIMAGE_SECTION_HEADER psh, HANDLE hProess);
	void DisassembleDummy(PDBG_DUMMY pdd);

	void SendNotiMsg(UINT uID, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		DOCK_MAP::iterator it = m_pNotiMap->find(uID);
		if (it == m_pNotiMap->end())
			return;
		SendMessage(it->second, uMsg, wParam, lParam);
	}

public:
	static PCWSTR GSZ_DBG_EVENTS[];
	static PE_SCHEMA SCHEMA;
	static CString CreateNewProject(PCTSTR pszPrjName,
		PCTSTR pszPrjPath, bool bIsProc, PCTSTR pszExePath, PCTSTR pszExeArgs = NULL);
	void SaveDebugProject(PCTSTR pszPrjPath);
	void LoadDebugProject(PCTSTR pszPrjPath);

	PBYTE GetReturnAddr32(PDBG_MODULE pdm, PBYTE pCtrlPC, HANDLE hThread);
#ifdef _WIN64
	static UCHAR G_UWND_OOP_SLOTS[];
	PUNWIND_CODE GetUnwindData(HANDLE hProc, PBYTE puiAddr, UNWIND_INFO& ui);
	PRUNTIME_FUNCTION UnwindPrologue(ULONG64 pCtrlPc, ULONG64 ulFrmBase, PRUNTIME_FUNCTION prtf,
		UNWIND_INFO& ui, PUNWIND_CODE pucs, ULONG64 ulImgBase, PCONTEXT pctx);
	PBYTE VirtualUnwind(PDBG_MODULE pdm, ULONG64 pCtrlPc, PRUNTIME_FUNCTION prtf, PCONTEXT pCtxRec);
	PUNWIND_INFO LookupPrimaryUnwindInfo(PRUNTIME_FUNCTION prtf, PBYTE pImgBase, PRUNTIME_FUNCTION* pPrimEnt);
	PRUNTIME_FUNCTION SameFunction(PRUNTIME_FUNCTION prtf, PBYTE pImgBase, ULONG64 pCtrlPc);
	PRUNTIME_FUNCTION LookupRTF(PBYTE pCtrlPC, PDBG_MODULE pdm);
	PBYTE GetReturnAddr64(PDBG_MODULE pdm, PBYTE pCtrlPC, HANDLE hThread);
#endif

	PCTSTR GetPrjName() { return m_szPrjName; }
	bool IsTypeProc()	{ return m_bIsProc; }
	bool UnderDebug()	{ return m_bAtDebug; }
	bool IsDbgHalt()	{ return (m_dwHaltThId > 0); }
	bool BreakEntPos()	{ return m_bBrkEntPos; }
	bool BreakThrStart(){ return m_bBrkRtlInit; }
	PBYTE GetRtlStartUp() { return m_pfnUsrThrSt; }

	PDBG_PROCESS	GetDbgProcess()	{ return &m_dp;		 };
	PDBG_THREAD_MAP	GetThreadMap()	{ return &m_mapThrs; }
	PDBG_MODULE_MAP GetModuleMap()	{ return &m_mapMods; }
	PDBG_BREAK_MAP	GetBreakMap()	{ return &m_mapBrks; }
	PDBG_NLBRK_MAP	GetNlBrkMap()	{ return &m_mapNlBrks; }
	PDBG_BKMARK_MAP	GetBMarkMap()	{ return &m_mapMarks; }
	PDBG_NLBM_MAP	GetNlBmMap()	{ return &m_mapNlBms; }

	PDBG_MODULE FindModule(PBYTE pAddr);
	PDBG_DASM FindDAsm(PDBG_MODULE pdm, DWORD dwCodeRva);
	PDBG_DASM FindDAsm(PDBG_DUMMY pdd, DWORD dwCodeRva);
	PDBG_CBASE FindCodes(PDBG_MODULE pdm, DWORD dwCodeRva);

	void SetNoitWnd(HWND hWnd, DOCK_MAP* pNotiMap)
	{
		m_hNotiWnd = hWnd;
		m_pNotiMap = pNotiMap;
	}
	void SendCommand(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		PostThreadMessage(m_dwDbgThId, uMsg, wParam, lParam);
		WaitForSingleObject(m_hevAck, INFINITE);
	}
	void SetInitPrm(PVOID pInitPrm)
	{
		m_pInitPrm = pInitPrm;
	}
	PVOID GetInitPrm() { return m_pInitPrm; }
};
