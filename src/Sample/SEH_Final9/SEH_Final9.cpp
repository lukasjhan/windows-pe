#include "stdafx.h"
#include "Windows.h"
#include "excpt.h"
#include "limits.h"


int YHD_Division(int dividend, int divider, int& remainder)
{
	int quotient = 0;
	__try	{
		__try	{
			__try { printf("Divided by Zero!!!\n"); }
			__except (EXCEPTION_CONTINUE_SEARCH) {}
		} __finally { printf("__FINAL1\n"); }

		__try {
			__try	{
				__try	{
					__try 	{
						__try 	{
							quotient = dividend / divider;
							remainder = dividend % divider;
						} __finally { printf("__FINAL2\n"); }
					}
					__except (EXCEPTION_CONTINUE_SEARCH) {}
				} __finally { printf("__FINAL4\n"); }

				__try	{
					__try	{ printf("Divided by Zero!!!\n"); }
					__except (EXCEPTION_CONTINUE_SEARCH) { }
				} __finally	{ printf("__FINAL6\n"); }
			} __finally { printf("__FINAL7\n"); }
		} __except (EXCEPTION_EXECUTE_HANDLER) { printf("__EXCEPT8\n"); }
	} __finally { printf("__FINAL9\n"); }
	return quotient;
}

void _tmain(int argc, TCHAR* argv[])
{
	int remainder = 0;
	int quotient = YHD_Division(7, 0, remainder);

	printf("7 / 0 = %d, remainder = %d\n", quotient, remainder);
}
