#include "stdafx.h"
#include <windows.h>


int ExceptFilter(PEXCEPTION_POINTERS pep, LONG lFltVal)
{
	printf("...FilterFunc -> Exception Flags: 0x%08X\n", lFltVal);
	return lFltVal;
}


void FuncC()
{
	PINT pVal = 0;
	__try
	{
		*pVal = 64;
	}
	__finally
	{
		printf("...FinalFunc called!!!\n");
	}
}


void FuncB()
{
	__try
	{
		FuncC();
	}
	__except (ExceptFilter(GetExceptionInformation(), EXCEPTION_CONTINUE_SEARCH))
	{
	}
}

void FuncA()
{
	PINT pVal = 0;
	__try
	{
		*pVal = 64;
	}
	__finally
	{
		printf("...FuncA::FinalFunc called!!!\n");
		goto $Label_Collided;
	}
$Label_Collided:
	printf("...FuncA::$Label_Collided!!!\n");
}

void _tmain(int argc, TCHAR* argv[])
{
	PINT pVal = 0;
	__try
	{
		FuncA();
		printf("..._tmain!!!\n");
	}
	__except (ExceptFilter(GetExceptionInformation(), EXCEPTION_EXECUTE_HANDLER))
	{
		printf("HandlerFunc -> Collied unwinding occurred...\n");
	}
}
