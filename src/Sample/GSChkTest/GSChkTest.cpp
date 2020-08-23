#include "stdafx.h"
#include <windows.h>
#include <excpt.h>


int ExceptFilter(unsigned int code, struct _EXCEPTION_POINTERS *ep)
{
	if (code == EXCEPTION_INT_DIVIDE_BY_ZERO)
	{
		printf(" -> Devided by Zero.\n");
		return EXCEPTION_EXECUTE_HANDLER;
	}
	else
	{
		printf(" -> Uncaught exception. continue search...\n");
		return EXCEPTION_EXECUTE_HANDLER;
	};
}

void YHD_GSChkWithSEH()
{
	wchar_t buff[16];
	int k = 2342342342;
	wsprintf(buff, L"%d: YHD_GSChkWithSEH", k);

	//__try
	//{
	//	INT A = 5 + 2;
	//}
	//__except (EXCEPTION_EXECUTE_HANDLER)
	//{
	//	printf("YHD_FuncExcept: Exception occurred...\n");
	//}
}

void YHD_GSChkWithCPPEH()
{
	wchar_t buff[16];
	int k = 2342342342;
	try
	{
		wsprintf(buff, L"%d: YHD_GSChkWithCPPEH", k);
	}
	catch(...)
	{
		printf("YHD_FuncExcept: Exception occurred...\n");
	}
}

void _tmain()
{
	__try
	{
		YHD_GSChkWithSEH();
	}
	__except (ExceptFilter(GetExceptionCode(), GetExceptionInformation()))
	{
		printf("YHD_FuncExcept: Exception occurred...\n");
	}
	//YHD_GSChkWithCPPEH();
}
