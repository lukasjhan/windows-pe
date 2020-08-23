#include "stdafx.h"
#include "Windows.h"
#include "excpt.h"
#include "limits.h"


int YHD_Division(int dividend, int divider, int& remainder)
{
	__try
	{
		int quotient = dividend / divider;
		remainder = dividend % divider;
		return quotient;
	}
	__except ((GetExceptionCode() == EXCEPTION_INT_DIVIDE_BY_ZERO) ?
				EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		printf("Divided by Zero!!!\n");
		return INT_MAX;
	}
}

void _tmain(int argc, TCHAR* argv[])
{
	if (argc < 3)
	{
		printf("SEH_None.exe int1, int2\n");
		return;
	}

	int dividend = _ttoi(argv[1]);
	int divider = _ttoi(argv[2]);

	int remainder = 0;
	int quotient = YHD_Division(dividend, divider, remainder);

	printf("%d / %d = %d, remainder = %d\n",
		dividend, divider, quotient, remainder);
}
