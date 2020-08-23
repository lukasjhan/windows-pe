#include "stdafx.h"
#include "Windows.h"

__declspec(thread) DWORD64 gt_dwTlsVal = 2000;

void NTAPI Tls_Callback(PVOID DllHandle, DWORD Reason, PVOID Reserved);

#pragma section(".CRT$XLY", long, read)
extern "C" __declspec(allocate(".CRT$XLY"))
PIMAGE_TLS_CALLBACK _xl_y = Tls_Callback;

void main()
{
	printf("TlsChkIsDbg main function started...\n");
	gt_dwTlsVal = 300;
	printf("TlsChkIsDbg main function terminates...\n");
}

void NTAPI Tls_Callback(PVOID DllHandle, DWORD Reason, PVOID Reserved)
{
	if (IsDebuggerPresent() && Reason == DLL_PROCESS_ATTACH)
	{
		MessageBoxA(NULL, "Not allowed Debuging!!!", "TlsChkIsDbg", MB_OK);
		ExitProcess(1);
	}
}
