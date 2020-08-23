#include "stdafx.h"
#include "Windows.h"

int ExceptFilter(PEXCEPTION_POINTERS pep, int nStep)
{
	for (int i = 0; i < nStep; i++)
		printf(" ");
	
	PEXCEPTION_RECORD per = pep->ExceptionRecord;
	printf("FILTER_#%d -> Code=0x%08X, Addr=0x%08X, Flags=0x%08X\n",
		nStep, per->ExceptionCode, per->ExceptionAddress, per->ExceptionFlags);
	return ((nStep == 0) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH);
}

void ExpFin2()
{
	__try
	{
		__try
		{
			printf("=> ExpFin2 will raise exception.\n\n");
			RaiseException(EXCEPTION_ILLEGAL_INSTRUCTION, 0, 0, NULL);
		}
		__finally
		{
			printf("  FINAL_#2 executed in ExpFin2...\n");
		}
	}
	__except (ExceptFilter(GetExceptionInformation(), 2))
	{
		printf("  CATCH_#2 executed in FinalTest2...\n");
	}
}

void ExpFin1()
{
	__try
	{
		__try
		{
			ExpFin2();
		}
		__except (ExceptFilter(GetExceptionInformation(), 1))
		{
			printf(" CATCH_#1 executed in ExpFin1...\n");
		}
	}
	__finally
	{
		printf(" FINAL_#1 executed in ExpFin1...\n");
	}
}


void _tmain(int argc, TCHAR* argv[])
{
	__try
	{
		__try
		{
			ExpFin1();
		}
		__except (ExceptFilter(GetExceptionInformation(), 0))
		{
			printf("CATCH_#0 executed in MAIN...\n");
			printf("===============================\n");
		}
	}
	__finally
	{
		printf("FINAL_#0 executed in MAIN...\n");
	}
}
