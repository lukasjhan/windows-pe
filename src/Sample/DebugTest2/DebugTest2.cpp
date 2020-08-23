#include "stdafx.h"
#include <windows.h>


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
#include <map>
typedef std::map<DWORD, LPCREATE_PROCESS_DEBUG_INFO> PROCESS_MAP;
typedef std::map<DWORD, LPCREATE_THREAD_DEBUG_INFO>	 THREAD_MAP;
typedef std::map<PBYTE, LPLOAD_DLL_DEBUG_INFO>		 DLL_MAP;

PROCESS_MAP G_PMAP;
THREAD_MAP	G_TMAP;
DLL_MAP		G_DMAP;

bool PrintDebugEvent(DEBUG_EVENT& de)
{
	bool bExit = false;
	printf("==> %S\n", G_DBG_INFOS[de.dwDebugEventCode - 1]);
	switch (de.dwDebugEventCode)
	{
		case CREATE_PROCESS_DEBUG_EVENT:
		{
			CREATE_PROCESS_DEBUG_INFO& di = de.u.CreateProcessInfo;
			printf("  File                = %d\n", (UINT)di.hFile);
			printf("  Process             = %d\n", (UINT)di.hProcess);
			printf("  Thread              = %d\n", (UINT)di.hThread);
			printf("  BaseOfImage         = 0x%p\n", di.lpBaseOfImage);
			printf("  DebugInfoFileOffset = %d\n", di.dwDebugInfoFileOffset);
			printf("  DebugInfoSize       = %d\n", di.nDebugInfoSize);
			printf("  ThreadLocalBase     = 0x%p\n", di.lpThreadLocalBase);
			printf("  StartAddress        = 0x%p\n", di.lpStartAddress);
			printf("  ImageName           = 0x%p\n", di.lpImageName);
			printf("  Unicode             = %s\n", (di.fUnicode != 0) ? "true" : "false");

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
				delete it->second;
				G_PMAP.erase(it);
				bExit = true;
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
			printf("  File                = %d\n", (UINT)di.hFile);
			printf("  BaseOfImage         = 0x%p\n", di.lpBaseOfDll);
			printf("  DebugInfoFileOffset = %d\n", di.dwDebugInfoFileOffset);
			printf("  DebugInfoSize       = %d\n", di.nDebugInfoSize);
			printf("  ImageName           = 0x%p\n", di.lpImageName);
			printf("  Unicode             = %s\n", (di.fUnicode != 0) ? "true" : "false");

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
				printf("  StartAddress    = 0x%p\n", it->second->lpBaseOfDll);
				delete it->second;
				G_DMAP.erase(it);
			}
		}
		break;

		case OUTPUT_DEBUG_STRING_EVENT:
		{
			OUTPUT_DEBUG_STRING_INFO& di = de.u.DebugString;
			printf("  DebugStringData   = 0x%p\n", di.lpDebugStringData);
			printf("  Unicode           = %s\n", (di.fUnicode != 0) ? "true" : "false");
			printf("  DebugStringLength = %d\n", di.nDebugStringLength);
		}
		break;

		case RIP_EVENT:
		{
			RIP_INFO& di = de.u.RipInfo;
			printf("Error = %d\n", di.dwError);
			printf("Type  = %d\n", di.dwType);
		}
		break;

		case EXCEPTION_DEBUG_EVENT:
		{
			EXCEPTION_DEBUG_INFO& di = de.u.Exception;
			printf("  FirstChance      = %s\n", (di.dwFirstChance != 0) ? "true" : "false");
			printf("  ExceptionCode    = 0x%08X\n", di.ExceptionRecord.ExceptionCode);
			printf("  ExceptionFlags   = 0x%08X\n", di.ExceptionRecord.ExceptionFlags);
			printf("  ExceptionAddress = 0x%p\n", di.ExceptionRecord.ExceptionAddress);
		}
		break;
	}
	printf("\n");
	return bExit;
}

void _tmain(int argc, TCHAR* argv[])
{
	if (argc < 2)
	{
		printf("DebugTest requires a argument of debugee....\n");
		return;
	}

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	WCHAR szCmdLine[MAX_PATH << 1];
	int nLen = 0;
	for (int i = 1; i < argc; i++)
		nLen += wsprintf(szCmdLine + nLen, L"%s ", argv[i]);
	szCmdLine[nLen - 1] = 0;

	if (!CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE,
		DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi))
	{
		printf("CreateProcess error, code=%d\n", GetLastError());
		return;
	}

	DEBUG_EVENT de = { 0 };
	while (true)
	{
		if (!WaitForDebugEvent(&de, INFINITE))
			break;

		if (PrintDebugEvent(de))
			break;
		ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}
