#include <windows.h>
#include <tchar.h>
#include <stdio.h>

int Add2(int a, int b)
{
	return (a + b);
}


__int64 Add6(char bPrm1, short wPrm2, int dPrm3, __int64 qPrm4, int nPrm5, int nPrm6)
{
	char	bLoc1 = bPrm1;
	short	wLoc2 = wPrm2;
	int		dLoc3 = dPrm3;
	__int64 qLoc4 = qPrm4;

	__int64 iiRet = bLoc1 + wLoc2 + dLoc3 + qLoc4;
	iiRet += Add2(nPrm5, nPrm6);

	return iiRet;
}


void _tmain()
{
	__int64 iiVal = Add6(4, 5, 6, 7, 8, 9);

	printf("Sum: %I64d\n", iiVal);
}
