#include "stdafx.h"
#include <windows.h>

void _tmain()
{
	int r = 0, a = 3, b = 0;
	try
	{
		if (b == 0)
			throw (int)5;

		r = a / b;
		if (r == 0)
			throw "b is greater than a...";

		printf("a / b = %d\n", r);
	}
	catch (int e)
	{
		printf("divided by zero: e = %d\n", e);
	}
	catch (PCSTR e)
	{
		printf(e);
	}
}
