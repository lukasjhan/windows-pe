#include "stdafx.h"
#include <windows.h>


int ExceptFilter(PEXCEPTION_POINTERS pep)
{
	printf("...FilterFunc -> Exception Flags: 0x%08X\n", pep->ExceptionRecord->ExceptionFlags);
	if (pep->ExceptionRecord->ExceptionFlags & EXCEPTION_NESTED_CALL)
		return EXCEPTION_EXECUTE_HANDLER;

	PINT pVal = 0;
	*pVal = 64;
	return EXCEPTION_CONTINUE_SEARCH;
}

void _tmain(int argc, TCHAR* argv[])
{
	PINT pVal = 0;
	__try
	{
		*pVal = 32;
	}
	__except (ExceptFilter(GetExceptionInformation()))
	{
		printf("HandlerFunc -> Nested exception occurred...\n");
	}
}
