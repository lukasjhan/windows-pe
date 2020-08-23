#include "stdafx.h"
#include "PEApp.h"
#include "PEFrame.h"
#include "PEDoc.h"
#include "PEDlg.ExceptInfo.h"
#include <TlHelp32.h>

#define DBG_WAIT_TIMEOUT	100
#ifndef STATUS_WX86_BREAKPOINT
#define STATUS_WX86_BREAKPOINT	0x4000001F
#endif
#ifndef STATUS_WX86_SINGLE_STEP
#define STATUS_WX86_SINGLE_STEP	0x4000001E
#endif

PCWSTR CPEDoc::GSZ_DBG_EVENTS[] =
{
	L"UNKNOWN",
	L"EXCEPTION",
	L"CREATE_THREAD",
	L"CREATE_PROCESS",
	L"EXIT_THREAD",
	L"EXIT_PROCESS",
	L"LOAD_DLL",
	L"UNLOAD_DLL",
	L"DEBUG_STRING",
	L"RIP_EVENT"
};


DWORD WINAPI CPEDoc::__DebuggerThread(PVOID pParam)
{
	DWORD dwExitCode = EXIT_SUCCESS;
	CPEDoc* pThis = static_cast<CPEDoc*>(pParam);
	try
	{
		HANDLE hevDbg = pThis->InitDebuger();
		pThis->RunDebugger(hevDbg);
		if (pThis->TermDebugger(hevDbg))
			dwExitCode = EXIT_FAILURE;
	}
	catch (HRESULT e)
	{
		AfxMessageBox(PEPlus::GetErrMsg(e));
		dwExitCode = (DWORD)e;
	}
	return dwExitCode;
}

HANDLE CPEDoc::InitDebuger()
{
	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_FORCEONFEEDBACK | STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOWNORMAL;

	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));
	WCHAR szCmdLine[MAX_PATH * 2];
	int nLen = wsprintf(szCmdLine, L"%s\\%s", m_dp.ModulePath, m_dp.ModuleName);
	if (!m_szExeArgs.IsEmpty())
		wsprintf(szCmdLine + nLen, L" %s", m_szExeArgs);
	BOOL bIsOK = CreateProcess
	(
		NULL, szCmdLine, NULL, NULL, FALSE,
		DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi
	);
	if (!bIsOK)
		throw HRESULT_FROM_WIN32(GetLastError());

	HANDLE hprChild = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);
	if (hprChild == NULL)
	{
		TerminateProcess(pi.hProcess, 0);
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		throw HRESULT_FROM_WIN32(GetLastError());
	}
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	m_dp.Process	= hprChild;
	m_dp.Priority	= GetPriorityClass(hprChild);
	m_dp.ProcessId	= pi.dwProcessId;
	m_dp.Active		= true;
	HANDLE hevDbg = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (hevDbg == NULL)
		throw HRESULT_FROM_WIN32(GetLastError());
	SetEvent(m_hevAck);

	return hevDbg;
}

bool CPEDoc::TermDebugger(HANDLE hevDbg)
{
	CloseHandle(hevDbg);
	bool bProcTerm = (WaitForSingleObject(m_dp.Process, 0) == WAIT_TIMEOUT);
	if (bProcTerm)
		TerminateProcess(m_dp.Process, 0);	//ERROR_DEBUGGER_INACTIVE);

	CloseHandle(m_dp.Process);
	m_dp.Process = NULL;

	return bProcTerm;
}

void CPEDoc::RunDebugger(HANDLE hevDbg)
{
	bool bOnDebug = true;
	while (bOnDebug)
	{
		DWORD dwWaitRet = MsgWaitForMultipleObjectsEx
		(
			1, &hevDbg, INFINITE, QS_POSTMESSAGE, MWMO_INPUTAVAILABLE
		);
		if (dwWaitRet == WAIT_FAILED)
			break;

		if (dwWaitRet == WAIT_OBJECT_0)
		{
			DEBUG_EVENT de = { 0 };
			if (!WaitForDebugEvent(&de, DBG_WAIT_TIMEOUT))
				continue;

			bool bCallNext = true;
			DWORD dwNextStatus = DBG_EXCEPTION_NOT_HANDLED;
			if (de.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
			{
				DBG_THREAD_MAP::iterator it = m_mapThrs.find(de.dwThreadId);
				if (it == m_mapThrs.end())
				{
					ContinueDebugEvent(de.dwProcessId, de.dwThreadId, dwNextStatus);
					continue;
				}

				EXCEPTION_DEBUG_INFO& di = de.u.Exception;
				if (di.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT ||
#ifdef _M_X64
					di.ExceptionRecord.ExceptionCode == STATUS_WX86_BREAKPOINT  || //WOW64의 경우 STATUS_WX86_BREAKPOINT
					di.ExceptionRecord.ExceptionCode == STATUS_WX86_SINGLE_STEP || //WOW64의 경우 STATUS_WX86_SINGLE_STEP
#endif // _M_X64
					di.ExceptionRecord.ExceptionCode == EXCEPTION_SINGLE_STEP)
				{
					bCallNext = HandleBreakEvent(di, it->second);
					dwNextStatus = DBG_CONTINUE;
				}
				else
				{
					int nDlgRlt = HandleExceptEvent(di, it->second);
					if (nDlgRlt == IDABORT)
						bCallNext = false;
					else
					{
						if (nDlgRlt == IDCONTINUE)
							dwNextStatus = DBG_CONTINUE;
						bCallNext = true;
					}
				}
			}
			else
			{
				if (HandleDebugEvent(de))
					SendNotifyMessage(m_hNotiWnd, WM_COMMAND, IDM_DEBUG_STOP, 0);
			}
			if (bCallNext)
				ContinueDebugEvent(de.dwProcessId, de.dwThreadId, dwNextStatus);
		}
		else
		{
			MSG msg;
			PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
			if (msg.message == TM_BREAK_RESUME)
			{
				DWORD dwThrId = HandleBreakMessage(msg);
				ContinueDebugEvent(m_dp.ProcessId, dwThrId, DBG_CONTINUE);
			}
			else
			{
				bOnDebug = HandleUserMessage(msg, hevDbg);
			}
		}
	}
}



bool CPEDoc::HandleBreakEvent(EXCEPTION_DEBUG_INFO& ei, PDBG_THREAD pdt)
{
	bool bCallNext = true;
	PBYTE pExptAddr  = (PBYTE)ei.ExceptionRecord.ExceptionAddress;
	DWORD dwExptCode = ei.ExceptionRecord.ExceptionCode;

#ifdef _M_X64
	if (dwExptCode == EXCEPTION_BREAKPOINT || 
		dwExptCode == STATUS_WX86_BREAKPOINT)	//WOW64의 경우 EXCEPTION_BREAKPOINT 대신 STATUS_WX86_BREAKPOINT
#else
	if (dwExptCode == EXCEPTION_BREAKPOINT)
#endif
	{
		PDBG_BRKPNT pdb = NULL;
		if (!m_bBPOnceHit)
		{
			if (m_bBrkEntPos)
			{
				PBYTE pEntPtr = m_dp.ImageBase + m_dp.EntryPoint;
				DBG_BREAK_MAP::iterator id = m_mapBrks.find(pEntPtr);
				if (id == m_mapBrks.end())
				{
					PDBG_BRKPNT pdb = new DBG_BRKPNT(m_dp.EntryPoint, &m_dp, true);
					pdb->OrgCode = UpdateBreakPoint(DBG_BP_CREATE, pEntPtr, OP_INT3);
					m_mapBrks.insert(std::make_pair(pEntPtr, pdb));
				}
			}
			if (m_pfnUsrThrSt != NULL)
			{
				DBG_BREAK_MAP::iterator id = m_mapBrks.find(m_pfnUsrThrSt);
				if (id == m_mapBrks.end())
				{
					DBG_MODULE_MAP::reverse_iterator it = m_mapMods.rbegin();
					PDBG_MODULE pdm = NULL;
					BOOL bIsWow64 = FALSE;
					IsWow64Process(m_dp.Process, &bIsWow64);
					if (bIsWow64)
					{
						for (; it != m_mapMods.rend(); it++)
						{
							if (it->second->Is32Bit)
							{
								pdm = it->second;
								break;
							}
						}
					}
					else
						pdm = it->second;
					DWORD dwIniRva = (DWORD)(m_pfnUsrThrSt - pdm->ImageBase);
					PDBG_BRKPNT pdb = new DBG_BRKPNT(dwIniRva, pdm, true);
					pdb->OrgCode = UpdateBreakPoint(DBG_BP_CREATE, m_pfnUsrThrSt, OP_INT3);
					m_mapBrks.insert(std::make_pair(m_pfnUsrThrSt, pdb));
				}
			}
			m_bBPOnceHit = true;

			return bCallNext;
		}

		DBG_BREAK_MAP::iterator id = m_mapBrks.find(pExptAddr);
		if (id == m_mapBrks.end())
			return bCallNext;
		pdb = id->second;
		pdt->InTrap = false;
		pdt->PrevBP = pdb;
		bCallNext = false;
	}
	else	//EXCEPTION_SINGLE_STEP, WOW64의 경우 STATUS_WX86_SINGLE_STEP
	{
		if (pdt->PrevBP != NULL)
		{
			if (pdt->PrevBP->IsTemp)
			{
				m_mapBrks.erase(pdt->PrevBP->HaltAddr());
				delete pdt->PrevBP;
			}
			else
				UpdateBreakPoint(DBG_BP_SET, pdt->PrevBP->HaltAddr(), OP_INT3);
			pdt->PrevBP = NULL;
		}
		if (pdt->InTrap)
		{
			UpdateThreadContext(pdt->Thread, m_dp.Is32Bit, true, false);
			bCallNext = false;
		}
	}
	if (!bCallNext)
	{
		m_dwHaltThId = pdt->ThreadId;
		GetHaltDebugInfo(pExptAddr, pdt, (dwExptCode == EXCEPTION_BREAKPOINT));
		PostMessage(m_hNotiWnd, WM_DEBUG_HALT, (WPARAM)dwExptCode, (LPARAM)pdt->ThreadId);
	}
	else
		m_dwHaltThId = 0;
	return bCallNext;
}

int CPEDoc::HandleExceptEvent(EXCEPTION_DEBUG_INFO& ei, PDBG_THREAD pdt)
{
	SendMessage(m_hNotiWnd, WM_DEBUG_MESSAGE, EXCEPTION_DEBUG_EVENT, (LPARAM)&ei);
	if (ei.dwFirstChance)
		return IDIGNORE;

	DWORD dwExptCode = ei.ExceptionRecord.ExceptionCode;
	PBYTE pExptAddr = (PBYTE)ei.ExceptionRecord.ExceptionAddress;
	PDBG_MODULE pdm = GetHaltDebugInfo(pExptAddr, pdt);
	if (pdm == NULL)
	{
		AfxMessageBox(L"해당 모듈이 없습니다.");
		return IDABORT;
	}

	if (pdm->CBaseCnt == 0)
		BuildFunctoin(m_dp.Process, pdm);

	PDBG_CBASE pdc = FindCodes(pdm, (DWORD)(pExptAddr - pdm->ImageBase));
	if (pdc->CodeType > CT_THUNK)
	{
		PDBG_DUMMY pdd = (PDBG_DUMMY)pdc;
		if (pdd->CodeCount == 0)
			DisassembleDummy(pdd);
	}

	ULONG_PTR dwRetVal = 0;
	SendMessageTimeout(m_hNotiWnd, WM_DEBUG_EXCEPT, (WPARAM)&ei,
		(LPARAM)pdt->ThreadId, SMTO_NOTIMEOUTIFNOTHUNG, 0, &dwRetVal);
	return (int)dwRetVal;
}

DWORD CPEDoc::HandleBreakMessage(MSG& msg)
{
	UINT uResume	= (UINT)msg.wParam;
	PVOID* pPrms	= (PVOID*)msg.lParam;
	DWORD dwCtrlPC	= (DWORD)pPrms[0];
	PDBG_MODULE pdm = (PDBG_MODULE)pPrms[1];
	DWORD	dwThrId = (DWORD)pPrms[2];

	DBG_THREAD_MAP::iterator it = m_mapThrs.find(dwThrId);
	if (it == m_mapThrs.end())
	{
		SetEvent(m_hevAck);
		return dwThrId;
	}

	bool bReqBP = false, bUseTrap = false;;
	PBYTE pNextStop = NULL;
	switch (uResume)
	{
		case IDM_DEBUG_STEP_OVER:
			bReqBP = false, bUseTrap = true;
		break;

		case IDM_DEBUG_STEP_IN:
		{
			PDBG_DASM pasm = FindDAsm(pdm, dwCtrlPC);
			if (pasm->Code._flags & (FLAG_REPNZ | FLAG_REP) ||
				pasm->Code._opCode == OP_IID::OI_CALL)
			{
				pNextStop = pdm->ImageBase + pasm->Code._offset + pasm->Code._count;
				bReqBP = true, bUseTrap = false;
			}
			else
			{
				bReqBP = false, bUseTrap = true;
			}
		}
		break;

		case IDM_DEBUG_STEP_OUT :
		{
			if (m_dp.Is32Bit)
				pNextStop = GetReturnAddr32(pdm, pdm->ImageBase + dwCtrlPC, it->second->Thread);
#ifdef _WIN64
			else
				pNextStop = GetReturnAddr64(pdm, pdm->ImageBase + dwCtrlPC, it->second->Thread);
#endif
			bReqBP = true, bUseTrap = false;
		}
		break;

		case IDM_DEBUG_RUN2CURSOR :
		{
			pNextStop = pdm->ImageBase + dwCtrlPC;
			bReqBP = true, bUseTrap = false;
		}
		break;

		default :	// IDM_DEBUG_CONTINUE
			bReqBP = false, bUseTrap = false;
		break;
	}

	PDBG_THREAD pth = it->second;
	pth->InTrap = bUseTrap;
	if (pth->PrevBP != NULL)
	{
		UpdateBreakPoint(DBG_BP_CLEAR,  pth->PrevBP->HaltAddr(), pth->PrevBP->OrgCode);
	}
	if (bUseTrap || pth->PrevBP != NULL)
	{
		UpdateThreadContext(pth->Thread, m_dp.Is32Bit, bUseTrap, (pth->PrevBP != NULL));
	}
	if (bReqBP && m_mapBrks.count(pNextStop) == 0)
	{
		PDBG_BRKPNT pdb = new DBG_BRKPNT(pNextStop, FindModule(pNextStop), true);
		pdb->OrgCode = UpdateBreakPoint(DBG_BP_CREATE, pdb->HaltAddr(), OP_INT3);
		m_mapBrks.insert(std::make_pair(pdb->HaltAddr(), pdb));
	}
	m_dwHaltThId = 0;
	SetEvent(m_hevAck);

	return dwThrId;
}

bool CPEDoc::HandleUserMessage(MSG& msg, HANDLE hevDbg)
{
	switch (msg.message)
	{
		case TM_CLOSE_DEBUGGER:
		return false;

		case TM_BREAK_EVENT:
		{
			PDBG_BRKPNT pdb = NULL;
			int nBrkOpt = (int)msg.wParam;

			SuspendDebuggeeProcess();
			if (nBrkOpt == DBG_BP_CREATE)
			{
				PVOID* pParams = (PVOID*)msg.lParam;
				PBYTE pHaltAddr = (PBYTE)pParams[0];
				PDBG_MODULE pdm = (PDBG_MODULE)pParams[1];
				DBG_BREAK_MAP::iterator id = m_mapBrks.find(pHaltAddr);
				if (id == m_mapBrks.end())
				{
					pdb = new DBG_BRKPNT(pHaltAddr, pdm);
					pdb->OrgCode = UpdateBreakPoint(DBG_BP_CREATE, pdb->HaltAddr(), OP_INT3);
					m_mapBrks.insert(std::make_pair(pHaltAddr, pdb));
				}
				else
				{
					pdb = id->second;
					if (pdb->IsTemp)
						pdb->IsTemp = false;
				}
				pParams[2] = pdb;
			}
			else
			{
				pdb = (PDBG_BRKPNT)msg.lParam;
				BYTE code = (nBrkOpt > DBG_BP_SET) ? pdb->OrgCode : OP_INT3;
				UpdateBreakPoint(nBrkOpt, pdb->HaltAddr(), code);
				if (nBrkOpt == DBG_BP_REMOVE)
				{
					m_mapBrks.erase(pdb->HaltAddr());
					delete pdb;
				}
			}
			ResumeDebuggeeProcess();
			SetEvent(m_hevAck);
		}
		break;

		case TM_SUSPEND_DEBUGGEE:
		{
			SuspendDebuggeeProcess();
			ResetEvent(hevDbg);
		}
		break;
		case TM_RESUME_DEBUGGEE:
		{
			ResumeDebuggeeProcess();
			SetEvent(hevDbg);
		}
		break;

		case TM_WALK_PROCMEM:
		{
			DBG_HEAP_MAP*  pheaps = (DBG_HEAP_MAP*)msg.wParam;
			DBG_VMOBJ_MAP* pvmos = (DBG_VMOBJ_MAP*)msg.lParam;
			WalkProcess(*pheaps, *pvmos);
			SetEvent(m_hevAck);
		}
		break;

		case TM_DISASSEMBLE:
		{
			if (msg.wParam == DASM_MODULE)
			{
				PDBG_MODULE pdm = (PDBG_MODULE)msg.lParam;
				if (pdm->Header == NULL)
					pdm->Header = GetPEHeader(m_dp.Process, pdm);
				BuildFunctoin(m_dp.Process, pdm);
			}
			else
			{
				PDBG_DUMMY pdd = (PDBG_DUMMY)msg.lParam;
				DisassembleDummy(pdd);
			}
			SetEvent(m_hevAck);
		}
		break;

		case TM_READ_MEMORY:
		{
			PVOID pReadAddr  = (PVOID)msg.wParam;
			PMEM_IO_PRM pmio = (PMEM_IO_PRM)msg.lParam;

			MEMORY_BASIC_INFORMATION mbi;
			if (!VirtualQueryEx(m_dp.Process, pReadAddr, &mbi,
				sizeof(mbi)) || mbi.State != MEM_COMMIT)
			{
				pmio->Size = 0;
				pmio->Error = HRESULT_FROM_WIN32(GetLastError());
				SetEvent(m_hevAck);
				break;
			}

			DWORD dwProtect = 0;
			if (!(mbi.Protect & PAGE_READONLY) && !(mbi.Protect & PAGE_READWRITE))
				VirtualProtectEx(m_dp.Process, pReadAddr, pmio->Size, PAGE_READONLY, &dwProtect);

			SIZE_T dwReadBytes = 0;
			if (!ReadProcessMemory(m_dp.Process, pReadAddr, pmio->Buff, pmio->Size, &dwReadBytes))
			{
				pmio->Size = 0;
				pmio->Error = HRESULT_FROM_WIN32(GetLastError());
			}
			else
			{
				pmio->Size = (DWORD)dwReadBytes;
				pmio->Error = S_OK;
			}
			if (dwProtect)
				VirtualProtectEx(m_dp.Process, pReadAddr, pmio->Size, dwProtect, &dwProtect);
			SetEvent(m_hevAck);
		}
		break;

		case TM_WRITE_MEMORY:
		{
			PVOID		pWriteAddr = (PVOID)msg.wParam;
			PMEM_IO_PRM pmio = (PMEM_IO_PRM)msg.lParam;
			SIZE_T		dwWroteBytes = 0;
			if (!WriteProcessMemory(m_dp.Process, pWriteAddr, pmio->Buff, pmio->Size, &dwWroteBytes))
			{
				pmio->Size = 0;
				pmio->Error = HRESULT_FROM_WIN32(GetLastError());
			}
			else
			{
				pmio->Size = (DWORD)dwWroteBytes;
				pmio->Error = S_OK;
			}
			SetEvent(m_hevAck);
		}
		break;
	}
	return true;
}

bool CPEDoc::HandleDebugEvent(DEBUG_EVENT& de)
{
	bool bNoitExit = false;
	LPARAM lParam = 0;
	switch (de.dwDebugEventCode)
	{
		case CREATE_PROCESS_DEBUG_EVENT:
		{
			LoadProcessInfo(de.dwThreadId, de.u.CreateProcessInfo);
			lParam = (LPARAM)&m_dp;
		}
		break;

		case EXIT_PROCESS_DEBUG_EVENT:
		{
			EXIT_PROCESS_DEBUG_INFO& di = de.u.ExitProcess;
			m_dp.ExitCode = di.dwExitCode;
			lParam = (LPARAM)&m_dp;
			bNoitExit = true;
		}
		break;

		case CREATE_THREAD_DEBUG_EVENT:
		{
			PDBG_THREAD pdt = LoadThreadInfo(de.dwThreadId, &m_dp, de.u.CreateThread);
			m_mapThrs.insert(std::make_pair(de.dwThreadId, pdt));
			lParam = (LPARAM)pdt;
		}
		break;

		case EXIT_THREAD_DEBUG_EVENT:
		{
			EXIT_THREAD_DEBUG_INFO& di = de.u.ExitThread;
			DBG_THREAD_MAP::iterator it = m_mapThrs.find(de.dwThreadId);
			if (it != m_mapThrs.end())
			{
				PDBG_THREAD pdt = it->second;
				m_mapThrs.erase(it);
				pdt->ExitCode = di.dwExitCode;
				lParam = (LPARAM)pdt;
			}
		}
		break;

		case LOAD_DLL_DEBUG_EVENT:
		{
			PDBG_MODULE pdm = LoadModuleInfo(&m_dp, de.u.LoadDll);
			m_mapMods.insert(std::make_pair(pdm->ImageBase, pdm));
			lParam = (LPARAM)pdm;
		}
		break;

		case UNLOAD_DLL_DEBUG_EVENT:
		{
			UNLOAD_DLL_DEBUG_INFO& di = de.u.UnloadDll;
			DBG_MODULE_MAP::iterator it = m_mapMods.find((PBYTE)di.lpBaseOfDll);
			if (it != m_mapMods.end())
			{
				PDBG_MODULE pdm = it->second;
				m_mapMods.erase(it);
				lParam = (LPARAM)pdm;
			}
		}
		break;

		case OUTPUT_DEBUG_STRING_EVENT:
		{
			OUTPUT_DEBUG_STRING_INFO& di = de.u.DebugString;
			int nSize = di.nDebugStringLength;
			if (di.fUnicode) nSize *= sizeof(WCHAR);

			PBYTE pBuff = new BYTE[nSize + 4];
			*((PWORD)pBuff) = di.fUnicode;
			PBYTE pszMsg = pBuff + sizeof(WORD);
			ReadProcessMemory(m_dp.Process, di.lpDebugStringData, pszMsg, nSize, NULL);
			pszMsg[nSize] = 0, pszMsg[nSize + 1] = 0;
			lParam = (LPARAM)pBuff;
		}
		break;

		case RIP_EVENT:
		{
			lParam = (LPARAM)&de.u.RipInfo;
		}
		break;
	}
	SendMessage(m_hNotiWnd, WM_DEBUG_MESSAGE, de.dwDebugEventCode, lParam);

	switch (de.dwDebugEventCode)
	{
		case EXIT_PROCESS_DEBUG_EVENT:
		{
			DBG_THREAD_MAP::iterator it = m_mapThrs.find(de.dwThreadId);
			if (it != m_mapThrs.end())
			{
				PDBG_THREAD pdt = it->second;
				m_mapThrs.erase(it);
				if (pdt == m_dp.MainThread)
					m_dp.MainThread = NULL;
				delete pdt;
			}
		}
		break;
		case EXIT_THREAD_DEBUG_EVENT:
		{
			PDBG_THREAD pdt = (PDBG_THREAD)lParam;
			if (pdt == m_dp.MainThread)
				m_dp.MainThread = NULL;
			delete pdt;
		}
		break;
		case UNLOAD_DLL_DEBUG_EVENT:
			delete (PDBG_MODULE)lParam;
		break;
		case OUTPUT_DEBUG_STRING_EVENT:
			delete[] (PBYTE)lParam; 
		break;
	}
	return bNoitExit;
}
