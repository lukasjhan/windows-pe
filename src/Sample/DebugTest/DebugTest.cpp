#include "stdafx.h"
#include <windows.h>


void PrintDebugEvent(DEBUG_EVENT& de)
{
	printf("  ==> ");
	switch (de.dwDebugEventCode)
	{
		case CREATE_PROCESS_DEBUG_EVENT:
			printf("CREATE_PROCESS_DEBUG_EVENT\n");
		break;
		case EXIT_PROCESS_DEBUG_EVENT:
			printf("EXIT_PROCESS_DEBUG_EVENT\n");
		break;
		case CREATE_THREAD_DEBUG_EVENT:
			printf("CREATE_THREAD_DEBUG_EVENT\n");
		break;
		case EXIT_THREAD_DEBUG_EVENT:
			printf("EXIT_THREAD_DEBUG_EVENT\n");
		break;
		case LOAD_DLL_DEBUG_EVENT:
			printf("LOAD_DLL_DEBUG_EVENT\n");
		break;
		case UNLOAD_DLL_DEBUG_EVENT:
			printf("UNLOAD_DLL_DEBUG_EVENT\n");
		break;
		case OUTPUT_DEBUG_STRING_EVENT:
			printf("OUTPUT_DEBUG_STRING_EVENT\n");
		break;
		case RIP_EVENT:
			printf("RIP_EVENT\n");
		break;
		case EXCEPTION_DEBUG_EVENT:
			printf("EXCEPTION_DEBUG_EVENT\n");
		break;
	}
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

		PrintDebugEvent(de);
		ContinueDebugEvent(de.dwProcessId, de.dwThreadId, DBG_CONTINUE);
	}

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}
