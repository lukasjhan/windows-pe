#include "stdafx.h"

int YHD_Division(int dividend, int divider, int remainder)
{
	int quotient = dividend / divider;
	remainder = dividend % divider;

	return quotient;
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
