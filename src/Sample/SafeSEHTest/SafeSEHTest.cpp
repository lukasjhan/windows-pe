#include "stdafx.h"
#include "Windows.h"
#include "excpt.h"
#include "limits.h"


class Complex
{
public:
	int m_r, m_i;

	Complex()
	{
		m_r = m_i = 0;
	}
	Complex(int r, int i)
	{
		m_r = r, m_i = i;
	}
	~Complex()
	{
		m_r = m_i = 0;
	}
};

int ExceptFilter(/*unsigned int code, */struct _EXCEPTION_POINTERS *ep)
{
	if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_INT_DIVIDE_BY_ZERO)
	{
		printf(" -> Devided by Zero.\n");
		return EXCEPTION_CONTINUE_SEARCH;
	}
	else
	{
		printf(" -> Uncaught exception. continue search...\n");
		return EXCEPTION_CONTINUE_SEARCH;
	};
}

int YHD_FuncExcept(int val, int div, int& rem)
{
	int quot = 0;
	__try
	{
		quot = val / div;
		rem = val % div;
		return quot;
	}
	//__except (ExceptFilter(/*GetExceptionCode(), */GetExceptionInformation()))
	__except (EXCEPTION_CONTINUE_SEARCH)
	{
		printf("YHD_FuncExcept: Exception occurred...\n");
		return INT_MAX;
	}
}

void YHD_FuncTerm(int* parVals, int nCnt)
{
	int* pRet = new int[nCnt];
	__try
	{
		for (int i = 0; i < nCnt; i++)
			pRet[i] = parVals[nCnt - 1 - i];
		memcpy(parVals, pRet, nCnt*sizeof(int));
	}
	__finally
	{
		delete[] pRet;
	}
}

int YHD_FuncTermAndExcept()
{
	int a = 0, r = -1;
	__try
	{
		__try
		{
			r = 32 / a;
			__try
			{
				r = 32 / a;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				r = 1;
			}
		}
		__except (EXCEPTION_CONTINUE_SEARCH)
		{
		}
	}
	__finally
	{
		a = 8;
	}

	__try
	{
		__try
		{
			r = 16 / a;
		}
		__finally
		{
			r = 1;
		}
	}
	__except (EXCEPTION_CONTINUE_SEARCH)
	{
		a = 8;
	}
	return r;
}

int YHD_Func2()
{
	return YHD_FuncTermAndExcept();
}

int YHD_Func1()
{
	return YHD_Func2();
}


int YHD_FuncTest()
{
	int r = -1;
	__try
	{
		r = YHD_Func1();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		r = INT_MAX;
	}
	return r;
}

void _tmain()
{
	int arVals[5] = { 1, 2, 3, 4, 5 };
	YHD_FuncTerm(arVals, 5);

	int rem = 0;
	int quot = YHD_FuncExcept(129, 0, rem);

	int c = YHD_FuncTest();
	printf("3 + 5 = %d\n", c);
}
