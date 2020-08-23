#include "stdafx.h"
#include "Windows.h"
#include "excpt.h"
#include "limits.h"


void _tmain(int argc, TCHAR* argv[])
{
	char szBuff[8];
	strcpy(szBuff, "This is");

	printf("%s\n", szBuff);
}
