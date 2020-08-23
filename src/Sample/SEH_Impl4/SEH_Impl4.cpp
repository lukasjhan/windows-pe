#include "stdafx.h"
#include <windows.h>

DWORD g_dwValue;

EXCEPTION_DISPOSITION _cdecl YHD_ExceptHandler
(
	PEXCEPTION_RECORD pER, PVOID pFrame, PCONTEXT pCtx, PVOID pDC
)
{
	static PCSTR EH_FLAGS[] =
	{
		"NONCONTINUABLE", "UNWINDING", "EXIT_UNWIND", "STACK_INVALID",
		"NESTED_CALL", "TARGET_UNWIND", "COLLIDED_UNWIND"
	};

	printf("  YHD_ExceptHandler=> Exception Code: %08X, Exception Flags: %X",
		pER->ExceptionCode, pER->ExceptionFlags);
	for (int i = 0; i < 7;i++)
	{
		if (pER->ExceptionFlags & (1 << i))
			printf(" %s", EH_FLAGS[i]);
	}
	printf("\n");
	return ExceptionContinueSearch;
}

void FuncA()
{
	DWORD dwHandler = (DWORD)YHD_ExceptHandler;

	__asm
	{
		push	dwHandler
		push	fs : [0]
		mov		fs : [0], esp
	}

	printf("FuncA: Before Exception -> Value=%d\n", g_dwValue);
	__asm
	{
		mov		eax, 0
		mov		[eax], 38
	}
	printf("FuncA: After  Exception -> Value=%d\n", g_dwValue);

	__asm
	{
		mov		eax, [esp]
		mov		fs : [0], eax
		add		esp, 8
	}
}

void main()
{
	printf("main: Before calling FuncA -> Value=%d\n", g_dwValue);
	__try
	{
		FuncA();
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		printf("  __except => Caught the exception in main\n");
	}
	printf("main: After  calling FuncA -> Value=%d\n", g_dwValue);
}
