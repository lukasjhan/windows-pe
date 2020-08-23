#include "stdafx.h"
#include <windows.h>


struct YHD_EXFRAME : EXCEPTION_REGISTRATION_RECORD
{
	DWORD GotoFunc;
	DWORD SavedEBP;
};
typedef YHD_EXFRAME* PYHD_EXFRAME;

EXCEPTION_DISPOSITION __cdecl YHD_ExceptHandler
(
	PEXCEPTION_RECORD pER, PVOID pFrame, PCONTEXT pCtx, PVOID pDC
)
{
	PYHD_EXFRAME pEF = (PYHD_EXFRAME)pFrame;
	printf("==> YHD_ExceptHandler: Exception caught!!!\n");
	printf("    Frame=0x%08X, Prev=0x%08X, Handler=0x%08X\n", pEF, pEF->Next, pEF->Handler);
	printf("    Exception=0x%08X, Address=0x%08X\n", pER->ExceptionCode, pER->ExceptionAddress);

	__asm
	{
		mov		edx, pEF
		mov		ebp, dword ptr[edx + 12];
		mov		eax, dword ptr[edx + 8];
		call	eax;
	}
	return ExceptionContinueSearch;
}



int YHD_Moduler(int dividend, int mod)
{
	return dividend % mod;
}

void main()
{
	DWORD dwHandler = (DWORD)YHD_ExceptHandler;
	DWORD dwSavedESP = 0;
	int   nRemainder = 0;

	__asm
	{
		push	ebp
		mov		eax, $GOTO_HANDLER
		push	eax
		push	dwHandler
		push	fs:[0]
		mov		fs:[0], esp
		mov		dwSavedESP, esp
	}

	printf("Before calling YHD_Moduler -> Remainder=%d\n", nRemainder);
	nRemainder = YHD_Moduler(23, 0);
	goto $RETURN;

$GOTO_HANDLER:
	__asm	mov esp, dwSavedESP
	nRemainder = -1;

$RETURN:
	printf("After  calling YHD_Moduler -> Remainder=%d\n", nRemainder);
	__asm
	{
		mov		eax, [esp]
		mov		fs:[0], eax
		add		esp, 10h
	}
}
