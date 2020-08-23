#include "stdafx.h"
#include <windows.h>
#include <excpt.h>

void f1()
{
	int r = 0, a = 3, b = 0;
	__try
	{
		if (b == 0)
			throw (int)-1;

		r = a / b;
		if (r == 0)
			throw "Result is zero.\n";
	}
	__finally
	{
		printf("unknown\n");
	}
}

void _tmain()
{
	int r = 0, a = 3, b = 0;
	try
	{
		try
		{
			try
			{
				f1();
			}
			catch (int e)
			{
				r = e;
			}
			catch (PCSTR e)
			{
				printf(e);
			}
		}
		catch (int e)
		{
			r = e;
		}
	}
	catch (int e)
	{
		r = e;
	}
	catch (PCSTR e)
	{
		printf(e);
	}
	catch (...)
	{
		printf("unknown\n");
	}

	try
	{
		if (b == 0)
			throw 5;

		r = a / b;
		if (r == 0)
			throw "Result is zero.\n";
	}
	catch (int e)
	{
		r = e;
	}
	catch (PCSTR e)
	{
		printf(e);
	}
	printf("3 + 5 = %d\n", r);
}
