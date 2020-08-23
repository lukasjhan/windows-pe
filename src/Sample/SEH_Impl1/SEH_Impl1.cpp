#include "stdafx.h"
#include <windows.h>

DWORD g_dwValue;

EXCEPTION_DISPOSITION __cdecl YHD_ExceptHandler
(
	PEXCEPTION_RECORD pER, PVOID pFrame, PCONTEXT pCtx, PVOID pDC
)
{
	PEXCEPTION_REGISTRATION_RECORD pEF = PEXCEPTION_REGISTRATION_RECORD(pFrame);

	printf("==> YHD_ExceptHandler: Exception caught!!!\n");
	printf("    Frame=0x%08X, Prev=0x%08X, Handler=0x%08X\n", pEF, pEF->Next, pEF->Handler);
	printf("    Exception=0x%08X, Address=0x%08X\n", pER->ExceptionCode, pER->ExceptionAddress);

	pCtx->Eax = (DWORD)&g_dwValue;

	return ExceptionContinueExecution;
}

void main()
{
	DWORD dwHandler = (DWORD)YHD_ExceptHandler;

	__asm
	{
		push	dwHandler
		push	fs:[0]
		mov		fs:[0], esp
	}

	printf("Before Exception -> Value=%d\n", g_dwValue);
	__asm
	{
		mov		eax, 0
		mov		[eax], 36
	}
	printf("After  Exception -> Value=%d\n", g_dwValue);

	__asm
	{
		mov		eax, [esp]
		mov		fs:[0], eax
		add		esp, 8
	}
}
