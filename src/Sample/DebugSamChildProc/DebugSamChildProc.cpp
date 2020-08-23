#include "stdafx.h"
#include <windows.h>


void _tmain()
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	getchar();
	if (!CreateProcess(L"C:\\YHD\\ref\\02.srcs\\Sample\\0.bin\\x64\\Debug\\BasicApp.exe", 
		 NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
	{
		printf("CreateProcess error, code=%d\n", GetLastError());
		return;
	}
	printf("........................Process Created!!!\n");

	WaitForSingleObject(pi.hProcess, INFINITE);
	printf("........................Process Terminated!!!\n");

	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
}
