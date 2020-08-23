#include "stdafx.h"
#include "Windows.h"


LONG GetFilteredValue(PEXCEPTION_RECORD pER)
{
	printf("ExceptionCode = 0x%08X, Address = 0x%08X\n", 
		pER->ExceptionCode, pER->ExceptionAddress);
	__try
	{
		int a = 0;
		int b = a / 0;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf(".... Exception 2 handler executed!!!\n");
	}

	if (pER->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
	{
		printf("\tAttempt failed to \"%s\", Address=0x%08X\n",
			(pER->ExceptionInformation[0] == 0)? "Read" : "Write",
			pER->ExceptionInformation[1]);
	}

	return EXCEPTION_EXECUTE_HANDLER;
}

void _tmain(int argc, TCHAR* argv[])
{
	PSTR pszBuffer = (PSTR)0x1234;
	EXCEPTION_DISPOSITION;
	PEXCEPTION_ROUTINE;
	PEXCEPTION_REGISTRATION_RECORD;
	__try
	{
		strcpy(pszBuffer, "Test for exception...");
	}
	__except (GetFilteredValue(((PEXCEPTION_POINTERS)
		GetExceptionInformation())->ExceptionRecord))
	{
		__try
		{
			int a = 0;
			int b = a / 0;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			printf(".... Exception 2 handler executed!!!\n");
		}
		printf(".... Exception handler executed!!!\n");
	}
}

//void _tmain(int argc, TCHAR* argv[])
//{
//	PEXCEPTION_POINTERS pEP = NULL;
//	PSTR pszBuffer = (PSTR)0x1234;
//
//	__try
//	{
//		strcpy(pszBuffer, "Test for exception...");
//	}
//	__except (pEP = GetExceptionInformation(), EXCEPTION_EXECUTE_HANDLER)
//	{
//		int a = 0;
//		int b = a / 0;
//		PEXCEPTION_RECORD pER = pEP->ExceptionRecord;
//		if (pER->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
//		{
//			printf("\"0x%08X\"에 있는 명령이 \"0x%08X\"의 메모리를 "
//				"참조했습니다. 메모리는 \"%s\"될 수 없었습니다.\n",
//				pER->ExceptionAddress, pER->ExceptionInformation[1],
//				(pER->ExceptionInformation[0] == 0) ? "Read" : "Written");
//		}
//		else
//			printf("ExceptionCode = 0x%08X, Address = 0x%08X\n", 
//				pER->ExceptionCode, pER->ExceptionAddress);
//	}
//}
