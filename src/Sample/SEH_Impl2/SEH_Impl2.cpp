#include "stdafx.h"
#include <windows.h>

DWORD g_dwValue;

EXCEPTION_DISPOSITION __cdecl YHD_ExcpHdlr1
(
	PEXCEPTION_RECORD pER, PVOID pFrame, PCONTEXT pCtx, PVOID pDC
)
{
	PEXCEPTION_REGISTRATION_RECORD pEF = PEXCEPTION_REGISTRATION_RECORD(pFrame);
	printf("==> YHD_ExcpHdlr1: Exception caught!!!\n");
	printf("    Frame=0x%08X, Prev=0x%08X, Handler=0x%08X\n", pEF, pEF->Next, pEF->Handler);
	printf("    Exception=0x%08X, Address=0x%08X\n", pER->ExceptionCode, pER->ExceptionAddress);

	return ExceptionContinueSearch;
}


struct YHD_EXFRAME : EXCEPTION_REGISTRATION_RECORD
{
	DWORD RefAddr;
	PCSTR MoreInfo;
};
typedef YHD_EXFRAME* PYHD_EXFRAME;

EXCEPTION_DISPOSITION __cdecl YHD_ExcpHdlr2
(
	PEXCEPTION_RECORD pER, PVOID pFrame, PCONTEXT pCtx, PVOID pDC
)
{
	PYHD_EXFRAME pEF = PYHD_EXFRAME(pFrame);
	printf("==> YHD_ExcpHdlr2: Exception caught!!!\n");
	printf("    Frame=0x%08X, Prev=0x%08X, Handler=0x%08X\n", pEF, pEF->Next, pEF->Handler);
	printf("    Exception=0x%08X, Address=0x%08X\n", pER->ExceptionCode, pER->ExceptionAddress);

	printf("    RefAddr=0x%08X, MoreInfo=%s\n", pEF->RefAddr, pEF->MoreInfo);
	pCtx->Eax = pEF->RefAddr;

	return ExceptionContinueExecution;
}


void FuncB()
{
	DWORD dwHandler = (DWORD)YHD_ExcpHdlr1;

	__asm
	{
		push	dwHandler
		push	fs:[0]
		mov		fs:[0], esp
	}

	printf("FuncB: Before Exception -> Value=%d\n", g_dwValue);
	__asm
	{
		mov		eax, 0
		mov		[eax], 38
	}
	printf("FuncB: After  Exception -> Value=%d\n", g_dwValue);

	__asm
	{
		mov		eax, [esp]
		mov		fs:[0], eax
		add		esp, 8
	}
}

void FuncA()
{
	printf("FuncA: Before calling FuncB -> Value=%d\n", g_dwValue);
	FuncB();
	printf("FuncA: After  calling FuncB -> Value=%d\n", g_dwValue);
}


void main()
{
	DWORD dwHandler = (DWORD)YHD_ExcpHdlr2;
	DWORD dwRefAddr = (DWORD)&g_dwValue;
	PCSTR pszStr = "SEH Frame's additive info in main";

	__asm
	{
		push	pszStr
		push	dwRefAddr
		push	dwHandler
		push	fs:[0]
		mov		fs:[0], esp
	}

	printf("main: Before calling FuncA -> Value=%d\n", g_dwValue);
	FuncA();
	printf("main: After  calling FuncA -> Value=%d\n", g_dwValue);

	__asm
	{
		mov		eax, [esp]
		mov		fs:[0], eax
		add		esp, 10h
	}
}
