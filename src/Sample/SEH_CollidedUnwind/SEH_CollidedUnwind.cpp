#include "stdafx.h"
#include <windows.h>

void FuncA()
{
	PINT pVal = 0;
	__try
	{
		*pVal = 12;
	}
	__finally
	{
		*pVal = 64;
		printf("...FuncA::FinalFunc called!!!\n");
	}
}

int ExceptFilter(PEXCEPTION_POINTERS pep)
{
	PEXCEPTION_RECORD per = pep->ExceptionRecord;
	PCONTEXT pctx = pep->ContextRecord;
	printf("...FilterFunc -> Code=0x%08X, Addr=0x%08X, Flags=0x%08X\n", 
		per->ExceptionCode, per->ExceptionAddress, per->ExceptionFlags);
	return EXCEPTION_EXECUTE_HANDLER;
}

void _tmain(int argc, TCHAR* argv[])
{
	__try
	{
		__try
		{
			FuncA();
			printf("...main!!!\n");
		}
		__finally
		{
			printf("...main::FinalFunc called!!!\n");
		}
	}
	__except (ExceptFilter(GetExceptionInformation()))
	{
		printf("HandlerFunc -> Collied unwinding occurred...\n");
	}
}
