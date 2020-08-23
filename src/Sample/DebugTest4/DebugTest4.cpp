#include "stdafx.h"
#include <windows.h>

#include "..\..\Solution\PEDAsm\PEDAsm.h"
#ifdef _DEBUG 
#	pragma comment(lib, "../../3.lib/x64/Debug/PEDAsm.lib")
#else
#	pragma comment(lib, "../../3.lib/x64/Release/PEDAsm.lib")
#endif


PCWSTR G_DBG_INFOS[] =
{
	L"EXCEPTION",
	L"CREATE_THREAD",
	L"CREATE_PROCESS",
	L"EXIT_THREAD",
	L"EXIT_PROCESS",
	L"LOAD_DLL",
	L"UNLOAD_DLL",
	L"OUTPUT_DBG_STR",
	L"RIP_INFO",
};

#define EVTIDX_EXIT		0
#define EVTIDX_SUSPEND	1
#define EVTIDX_RESUME	2
#define EVTIDX_READMEM	3
#define EVTIDX_CONTEXT	4
#define EVTIDX_BREAK	5
#define EVTIDX_NEXT		6
#define EVTIDX_RUN		7
#define EVTIDX_DEBUG	8
#define EVTIDX_COUNT	(EVTIDX_DEBUG + 1)

#define TF_BIT		0x100
#define OP_INT3		0xCC
#define DGB_STEP	1
#define DGB_RUN		2

struct INIT_INFO
{
	PCWSTR	ExePath;
	PWSTR	Args;
	HANDLE	DbgEvts[EVTIDX_COUNT];
	HANDLE	WaitSig;
	BOOL	BPOnceHit;
	BOOL	BreakEP;
	BOOL	DbgStop;
	BYTE	OpOld;
};
typedef INIT_INFO* PINIT_INFO;

#include <map>
typedef std::map<DWORD, LPCREATE_PROCESS_DEBUG_INFO> PROCESS_MAP;
typedef std::map<DWORD, LPCREATE_THREAD_DEBUG_INFO>	 THREAD_MAP;
typedef std::map<PBYTE, LPLOAD_DLL_DEBUG_INFO>		 DLL_MAP;

PROCESS_MAP G_PMAP;
THREAD_MAP	G_TMAP;
DLL_MAP		G_DMAP;


void PrintImageName(PVOID pImgName, HANDLE hProc, HANDLE hFile, BOOL bUniCode)
{
	WCHAR szDllName[MAX_PATH] = { 0, };
	if (pImgName != NULL)
	{
		PBYTE pAddr = NULL;
		ReadProcessMemory(hProc, pImgName, &pAddr, sizeof(PBYTE), NULL);
		if (pAddr != NULL)
		{
			int nSize = MAX_PATH * sizeof(CHAR);
			ReadProcessMemory(hProc, pAddr, szDllName, nSize, NULL);
			if (bUniCode)
				printf("  ImageName =>> %S\n", szDllName);
			else
				printf("  ImageName =>> %s\n", (PCWSTR)szDllName);
		}
	}
	if (szDllName[0] == 0)
	{
		DWORD dwLen = GetFinalPathNameByHandle(hFile, szDllName, MAX_PATH, VOLUME_NAME_DOS);
		if (dwLen > 0)
		{
			PWSTR pPos = wcschr(szDllName, L'?');
			if (pPos != NULL)
			{
				if (*(pPos + 1) == L'\\')
					pPos += 2;
				printf("  ImageName =>> %S\n", pPos);
			}
		}
	}
}

bool PrintDebugEvent(DEBUG_EVENT& de, PPROCESS_INFORMATION ppi)
{
	printf("\n==> %S [ProcessId = %d, ThreadId = %d]\n", 
		G_DBG_INFOS[de.dwDebugEventCode - 1], de.dwProcessId, de.dwThreadId);

	bool bTerminated = false;
	switch (de.dwDebugEventCode)
	{
		case CREATE_PROCESS_DEBUG_EVENT:
		{
			CREATE_PROCESS_DEBUG_INFO& di = de.u.CreateProcessInfo;
			printf("  File            = %d\n", (UINT)di.hFile);
			printf("  Process         = %d\n", (UINT)di.hProcess);
			printf("  Thread          = %d\n", (UINT)di.hThread);
			printf("  BaseOfImage     = 0x%p\n", di.lpBaseOfImage);
			printf("  ThreadLocalBase = 0x%p\n", di.lpThreadLocalBase);
			printf("  StartAddress    = 0x%p\n", di.lpStartAddress);
			PrintImageName(di.lpImageName, di.hProcess, di.hFile, di.fUnicode);

			LPCREATE_PROCESS_DEBUG_INFO pdi = new CREATE_PROCESS_DEBUG_INFO();
			*pdi = di;
			G_PMAP.insert(std::make_pair(de.dwProcessId, pdi));
		}
		break;

		case EXIT_PROCESS_DEBUG_EVENT:
		{
			EXIT_PROCESS_DEBUG_INFO& di = de.u.ExitProcess;
			printf("  ExitCode = %d\n", di.dwExitCode);

			PROCESS_MAP::iterator it = G_PMAP.find(de.dwProcessId);
			if (it != G_PMAP.end())
			{
				printf("  BaseOfImage    = 0x%p\n", it->second->lpBaseOfImage);
				if (de.dwProcessId == ppi->dwProcessId)
					bTerminated = true;
				delete it->second;
				G_PMAP.erase(it);
			}
		}
		break;

		case CREATE_THREAD_DEBUG_EVENT:
		{
			CREATE_THREAD_DEBUG_INFO& di = de.u.CreateThread;
			printf("  Thread          = %d\n", (UINT)di.hThread);
			printf("  ThreadLocalBase = 0x%p\n", di.lpThreadLocalBase);
			printf("  StartAddress    = 0x%p\n", di.lpStartAddress);

			LPCREATE_THREAD_DEBUG_INFO pdi = new CREATE_THREAD_DEBUG_INFO();
			*pdi = di;
			G_TMAP.insert(std::make_pair(de.dwThreadId, pdi));
		}
		break;

		case EXIT_THREAD_DEBUG_EVENT:
		{
			EXIT_THREAD_DEBUG_INFO& di = de.u.ExitThread;
			printf("  ExitCode = %d\n", di.dwExitCode);

			THREAD_MAP::iterator it = G_TMAP.find(de.dwThreadId);
			if (it != G_TMAP.end())
			{
				printf("  StartAddress    = 0x%p\n", it->second->lpStartAddress);
				delete it->second;
				G_TMAP.erase(it);
			}
		}
		break;

		case LOAD_DLL_DEBUG_EVENT:
		{
			LOAD_DLL_DEBUG_INFO& di = de.u.LoadDll;
			printf("  File        = %d\n", (UINT)di.hFile);
			printf("  BaseOfImage = 0x%p\n", di.lpBaseOfDll);
			printf("  LoadType    = %s\n", (di.lpImageName == NULL) ? "Dynamic" : "Static");

			PROCESS_MAP::iterator it = G_PMAP.find(de.dwProcessId);
			if (it != G_PMAP.end())
				PrintImageName(di.lpImageName, it->second->hProcess, di.hFile, di.fUnicode);

			LPLOAD_DLL_DEBUG_INFO pdi = new LOAD_DLL_DEBUG_INFO();
			*pdi = di;
			G_DMAP.insert(std::make_pair((PBYTE)di.lpBaseOfDll, pdi));
		}
		break;

		case UNLOAD_DLL_DEBUG_EVENT:
		{
			UNLOAD_DLL_DEBUG_INFO& di = de.u.UnloadDll;
			printf("  BaseOfDll = 0x%p\n", di.lpBaseOfDll);

			DLL_MAP::iterator it = G_DMAP.find((PBYTE)di.lpBaseOfDll);
			if (it != G_DMAP.end())
			{
				delete it->second;
				G_DMAP.erase(it);
			}
		}
		break;

		case OUTPUT_DEBUG_STRING_EVENT:
		{
			OUTPUT_DEBUG_STRING_INFO& di = de.u.DebugString;
			int nSize = di.nDebugStringLength;
			if (di.fUnicode)
				nSize *= sizeof(WCHAR);
			PBYTE pszMsg = new BYTE[nSize + 2];
			ReadProcessMemory(ppi->hProcess, di.lpDebugStringData, pszMsg, nSize, NULL);
			pszMsg[nSize] = 0, pszMsg[nSize + 1] = 0;
			if (di.fUnicode)
				printf("  Message = %S\n", (PCWSTR)pszMsg);
			else
				printf("  Message = %s\n", (PCSTR)pszMsg);
		}
		break;

		case RIP_EVENT:
		{
			RIP_INFO& di = de.u.RipInfo;
			printf("  Error = %d\n", di.dwError);
			printf("  Type  = %d\n", di.dwType);
		}
		break;

		case EXCEPTION_DEBUG_EVENT:
		{
			EXCEPTION_DEBUG_INFO& di = de.u.Exception;
			printf("  FirstChance   = %d\n", di.dwFirstChance);
			printf("  ExceptionCode = 0x%08X, Address = 0x%p\n",
				di.ExceptionRecord.ExceptionCode, di.ExceptionRecord.ExceptionAddress);
		}
		break;
	}
	return bTerminated;
}

bool CreateDbgEvents(PHANDLE pDbgEvts)
{
	int i = 0;
	for (; i < EVTIDX_COUNT; i++)
	{
		pDbgEvts[i] = CreateEvent
		(
			NULL, (i == EVTIDX_EXIT || i == EVTIDX_DEBUG) ? TRUE : FALSE, 
			(i == EVTIDX_DEBUG) ? TRUE : FALSE, NULL
		);
		if (pDbgEvts[i] == NULL)
		{
			printf("CreateEvent error, code=%d\n", GetLastError());
			break;
		}
	}
	if (i < EVTIDX_COUNT)
	{
		for (int j = 0; j < i; j++)
		{
			CloseHandle(pDbgEvts[j]);
			pDbgEvts[j] = NULL;
		}
		return false;
	}
	return true;
}

BYTE SetBreakPoint(HANDLE hProcess, PBYTE pBrkPtAddr)
{
	BYTE opBrk = OP_INT3, opOld = 0;
	ReadProcessMemory(hProcess, pBrkPtAddr, &opOld, sizeof(BYTE), NULL);
	WriteProcessMemory(hProcess, pBrkPtAddr, &opBrk, sizeof(BYTE), NULL);
	FlushInstructionCache(hProcess, pBrkPtAddr, sizeof(BYTE));
	return opOld;
}

void RestoreBreakPoint(HANDLE hProcess, PBYTE pBrkPtAddr, BYTE opOld)
{
	WriteProcessMemory(hProcess, pBrkPtAddr, &opOld, sizeof(BYTE), NULL);
	FlushInstructionCache(hProcess, pBrkPtAddr, sizeof(BYTE));
}

void PrintAsmCode(HANDLE hProc, PBYTE pCodeAddr, bool bIs32Bit)
{
	BYTE opCode[15];
	ReadProcessMemory(hProc, pCodeAddr, opCode, sizeof(opCode), NULL);

	DECODED_INS dis;
	int ndiCnt = 1;
	int nReads = PEDisAsm::DisAssemble(&dis, ndiCnt, opCode, 0, 15, bIs32Bit);
	DECODED dec;
	PEDisAsm::DecodedFormat(&dis, &dec, bIs32Bit);

	char szCode[64];
	int nLen = 0;
	for (int i = 0; i < dec.Count; i++)
		nLen += sprintf_s(szCode + nLen, 64 - nLen, "%02X ", opCode[i]);

	printf("      %p %s  %s %s\n", pCodeAddr, szCode, dec.Mnemonic,
		(dec.Operands != NULL) ? dec.Operands : "");
}

DWORD WINAPI DebugThreadProc(PVOID pParam)
{
	PINIT_INFO pii = (PINIT_INFO)pParam;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	WCHAR szCmdLine[MAX_PATH];
	if (pii->Args != NULL)
		wsprintf(szCmdLine, L"%s %s", pii->ExePath, pii->Args);
	else
		wsprintf(szCmdLine, L"%s", pii->ExePath);
	if (!CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE,
		DEBUG_PROCESS, NULL, NULL, &si, &pi))
	{
		printf("CreateProcess error, code=%d\n", GetLastError());
		return 0;
	}

	bool bIsOK = CreateDbgEvents(pii->DbgEvts);
	SetEvent(pii->WaitSig);
	if (!bIsOK)
	{
		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);
		return 0;
	}

	DEBUG_EVENT de = { 0 };
	bool bRunDbg = true;
	while (bRunDbg)
	{
		DWORD dwIdx = WaitForMultipleObjects
		(
			EVTIDX_COUNT, pii->DbgEvts, FALSE, INFINITE
		);
		switch (dwIdx)
		{
			case EVTIDX_EXIT:
				bRunDbg = false;
			break;

			case EVTIDX_SUSPEND:
				for (THREAD_MAP::iterator it = G_TMAP.begin(); it != G_TMAP.end(); it++)
				{
					SuspendThread(it->second->hThread);
				}
				printf(" -> All threads are suspended........\n");
				ResetEvent(pii->DbgEvts[EVTIDX_DEBUG]);
			break;
			
			case EVTIDX_RESUME:
				for (THREAD_MAP::iterator it = G_TMAP.begin(); it != G_TMAP.end(); it++)
				{
					ResumeThread(it->second->hThread);
				}
				printf(" -> All threads are resumed..........\n");
				SetEvent(pii->DbgEvts[EVTIDX_DEBUG]);
			break;

			case EVTIDX_READMEM:
			{
				PROCESS_MAP::iterator it = G_PMAP.find(pi.dwProcessId);
				if (it == G_PMAP.end())
					break;

				BYTE btDump[64];
				ReadProcessMemory(it->second->hProcess, it->second->lpBaseOfImage, btDump, 64, NULL);

				char szMsg[128];
				PBYTE pAddr = (PBYTE)it->second->lpBaseOfImage;
				PBYTE pIter = btDump;
				for (int i = 0; i < 4; i++)
				{
					int nLen = sprintf_s(szMsg, 128, "%p  ", pAddr);
					for (int j = 0; j < 16; j++, pIter++)
					{
						nLen += sprintf_s(szMsg + nLen, 128 - nLen, "%02X ", *pIter);
						if (j == 7)
							szMsg[nLen++] = ' ';
					}
					printf("%s\n", szMsg);
					pAddr += 16;
				}
			}
			break;

			case EVTIDX_CONTEXT:
			{
				PCSTR C_REGS[] = 
				{
					"RAX", "RCX", "RDX", "RBX", "RSP", "RBP", "RSI", "RDI",
					"R8 ", "R9 ",  "R10", "R11", "R12", "R13", "R14", "R15"
				};

				CONTEXT ctx;
				ctx.ContextFlags = CONTEXT_FULL;
				GetThreadContext(pi.hThread, &ctx);
				PDWORD64 pRegs = &ctx.Rax;
				for (int i = 0; i < 8; i++)
				{
					printf("%s=0x%p, %s=0x%p\n", C_REGS[i * 2], *pRegs, C_REGS[i * 2 + 1], *(pRegs + 1));
					pRegs += 2;
				}
				printf("%RIP=0x%p, EFLAGS=0x%08X\n", ctx.Rip, ctx.EFlags);
			}
			break;

			case EVTIDX_BREAK:
			{
				if (!pii->DbgStop)
					pii->DbgStop = true;

				SuspendThread(pi.hThread);

				CONTEXT ctx;
				ctx.ContextFlags = CONTEXT_CONTROL;
				GetThreadContext(pi.hThread, &ctx);

				pii->OpOld = SetBreakPoint(pi.hProcess, (PBYTE)ctx.Rip);
				printf("  ....User BreakPoint is set at address 0x%p\n", (PBYTE)ctx.Rip);

				ResumeThread(pi.hThread);
			}
			break;
			
			case EVTIDX_NEXT:
			case EVTIDX_RUN:
			{
				if (!pii->DbgStop)
					break;

				if (dwIdx == EVTIDX_NEXT)
				{
					HANDLE hThread = NULL;
					if (de.dwThreadId == pi.dwThreadId)
						hThread = pi.hThread;
					else
					{
						THREAD_MAP::iterator ih = G_TMAP.find(de.dwThreadId);
						if (ih != G_TMAP.end())
							hThread = ih->second->hThread;
					}

					if (hThread != NULL)
					{
						CONTEXT ctx;
						ctx.ContextFlags = CONTEXT_CONTROL;
						GetThreadContext(hThread, &ctx);
						ctx.EFlags |= TF_BIT;
						SetThreadContext(hThread, &ctx);
					}
				}
				else
					pii->DbgStop = FALSE;
				ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
			}
			break;

			case EVTIDX_DEBUG:
			{
				if (!WaitForDebugEvent(&de, 100))
					continue;

				if(PrintDebugEvent(de, &pi))
				{
					bRunDbg = false;
					break;
				}
				if (de.dwDebugEventCode != EXCEPTION_DEBUG_EVENT)
				{
					ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
					break;
				}

				EXCEPTION_DEBUG_INFO& ei = de.u.Exception;
				DWORD dwExpCode = ei.ExceptionRecord.ExceptionCode;
				if (dwExpCode != EXCEPTION_BREAKPOINT && dwExpCode != EXCEPTION_SINGLE_STEP)
				{
					ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_EXCEPTION_NOT_HANDLED);
					break;
				}

				PROCESS_MAP::iterator it = G_PMAP.find(de.dwProcessId);
				if (it == G_PMAP.end())
				{
					ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
					break;
				}
				LPCREATE_PROCESS_DEBUG_INFO ppi = it->second;

				if (dwExpCode == EXCEPTION_BREAKPOINT)
				{
					if (!pii->BPOnceHit)
					{
						BYTE dpData[65536];
						ReadProcessMemory(ppi->hProcess, ppi->lpBaseOfImage, dpData, sizeof(dpData), NULL);

						PIMAGE_NT_HEADERS64 pnh = (PIMAGE_NT_HEADERS64)(dpData + PIMAGE_DOS_HEADER(dpData)->e_lfanew);
						PBYTE pEPAddr = (PBYTE)ppi->lpBaseOfImage + pnh->OptionalHeader.AddressOfEntryPoint;

						pii->OpOld = SetBreakPoint(ppi->hProcess, pEPAddr);
						printf("  ....BreakPoint is set at address 0x%p\n", pEPAddr);
						pii->BPOnceHit = TRUE;

						ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
					}
					else
					{
						HANDLE hThread = NULL;
						if (de.dwThreadId == pi.dwThreadId)
							hThread = pi.hThread;
						else
						{
							THREAD_MAP::iterator ih = G_TMAP.find(de.dwThreadId);
							if (ih == G_TMAP.end())
							{
								ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
								break;
							}
							hThread = ih->second->hThread;
						}

						CONTEXT ctx;
						ctx.ContextFlags = CONTEXT_CONTROL;
						GetThreadContext(hThread, &ctx);
						ctx.Rip--;
						ctx.EFlags |= TF_BIT;
						SetThreadContext(hThread, &ctx);

						RestoreBreakPoint(ppi->hProcess, (PBYTE)ei.ExceptionRecord.ExceptionAddress, pii->OpOld);

						printf("  ....Excecution stopped at code address 0x%p\n", ei.ExceptionRecord.ExceptionAddress);
						PrintAsmCode(ppi->hProcess, (PBYTE)ei.ExceptionRecord.ExceptionAddress, false);
					}
				}
				else	// EXCEPTION_SINGLE_STEP
				{
					printf("  ....Single line excecution completed, "
						"netx code address is 0x%p\n", ei.ExceptionRecord.ExceptionAddress);
					PrintAsmCode(ppi->hProcess, (PBYTE)ei.ExceptionRecord.ExceptionAddress, false);

					if (!pii->DbgStop)
						ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
				}
			}
			break;
		}
	}

	for (DLL_MAP::iterator it = G_DMAP.begin(); it != G_DMAP.end(); it++)
	{
		CloseHandle(it->second->hFile);
		delete it->second;
	}
	for (THREAD_MAP::iterator it = G_TMAP.begin(); it != G_TMAP.end(); it++)
	{
		delete it->second;
	}
	for (PROCESS_MAP::iterator it = G_PMAP.begin(); it != G_PMAP.end(); it++)
	{
		CloseHandle(it->second->hFile);
		TerminateProcess(it->second->hProcess, 0);
		delete it->second;
	}

	for (int i = 0; i < EVTIDX_COUNT; i++)
	{
		CloseHandle(pii->DbgEvts[i]);
		pii->DbgEvts[i] = NULL;
	}
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return 0;
}

void _tmain(int argc, TCHAR* argv[])
{
	if (argc < 2)
	{
		printf("DebugTest requires a argument of debugee....\n");
		return;
	}

	INIT_INFO ii;
	memset(&ii, 0, sizeof(ii));
	ii.BreakEP = 1;
	ii.DbgStop = 1;
	ii.ExePath = argv[1];
	if (argc > 2)
	{
		int nLen = 0;
		for (int i = 2; i < argc; i++)
			nLen += ((int)wcslen(argv[i]) + 1);

		ii.Args = new WCHAR[nLen + 1];
		nLen = 0;
		for (int i = 2; i < argc; i++)
			nLen += wsprintf(ii.Args + nLen, L"%s ", argv[i]);
		ii.Args[nLen - 1] = 0;
	}
	ii.WaitSig = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (ii.WaitSig == NULL)
	{
		printf("CreateEvent error, code=%d\n", GetLastError());
		return;
	}

	printf("======================================================\n");
	printf("*** Debuger started. input command....\n\n");

	HANDLE hDbgThr = NULL;
	char szIn[256];
	while (true)
	{
		gets_s(szIn);
		if (_stricmp(szIn, "start") == 0)
		{
			if (hDbgThr != NULL)
			{
				if (WaitForSingleObject(hDbgThr, 0) != WAIT_OBJECT_0)
					continue;
				CloseHandle(hDbgThr);
			}
			DWORD dwThreadId = 0;
			hDbgThr = CreateThread(NULL, 0, DebugThreadProc, &ii, 0, &dwThreadId);
			if (hDbgThr == NULL)
			{
				printf("CreateThread error, code=%d\n", GetLastError());
				break;
			}
			WaitForSingleObject(ii.WaitSig, INFINITE);
			if (ii.DbgEvts[0] == NULL)
				break;
		}
		else if (_stricmp(szIn, "stop") == 0)
		{
			if (hDbgThr == NULL)
				continue;
			ResetEvent(ii.DbgEvts[EVTIDX_DEBUG]);
			SetEvent(ii.DbgEvts[EVTIDX_EXIT]);
			WaitForSingleObject(hDbgThr, INFINITE);
			CloseHandle(hDbgThr); hDbgThr = NULL;
		}
		else if (_stricmp(szIn, "suspend") == 0)
		{
			SetEvent(ii.DbgEvts[EVTIDX_SUSPEND]);
		}
		else if (_stricmp(szIn, "resume") == 0)
		{
			SetEvent(ii.DbgEvts[EVTIDX_RESUME]);
		}
		else if (_stricmp(szIn, "read") == 0)
		{
			SetEvent(ii.DbgEvts[EVTIDX_READMEM]);
		}
		else if (_stricmp(szIn, "context") == 0)
		{
			SetEvent(ii.DbgEvts[EVTIDX_CONTEXT]);
		}
		else if (_stricmp(szIn, "break") == 0)
		{
			SetEvent(ii.DbgEvts[EVTIDX_BREAK]);
		}
		else if (_stricmp(szIn, "next") == 0)
		{
			SetEvent(ii.DbgEvts[EVTIDX_NEXT]);
		}
		else if (_stricmp(szIn, "run") == 0)
		{
			SetEvent(ii.DbgEvts[EVTIDX_RUN]);
		}
		else if (_stricmp(szIn, "quit") == 0)
		{
			if (hDbgThr != NULL)
			{
				ResetEvent(ii.DbgEvts[EVTIDX_DEBUG]);
				SetEvent(ii.DbgEvts[EVTIDX_EXIT]);
				WaitForSingleObject(hDbgThr, INFINITE);
			}
			break;
		}
	}
	if (ii.Args != NULL)
		delete[] ii.Args;
	if (ii.DbgEvts[EVTIDX_EXIT] != NULL)
		CloseHandle(ii.DbgEvts[EVTIDX_EXIT]);
	if (hDbgThr != NULL)
		CloseHandle(hDbgThr);
}
