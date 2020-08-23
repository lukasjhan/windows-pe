#include "stdafx.h"
#include "Windows.h"

#if defined _M_IX86
#error	"Not support on X86"
#endif

#define UNWIND_HISTORY_TABLE_NONE	 0
#define UNWIND_HISTORY_TABLE_GLOBAL	 1
#define UNWIND_HISTORY_TABLE_LOCAL	 2


int YHD_Division(int dividend, int divider, int& remainder)
{
	__try
	{
		DWORD64 ulImageBase;
		PVOID Handlerdata;
		DWORD64 EstablisherFrame;

		CONTEXT ctx;
		RtlCaptureContext(&ctx);

		UNWIND_HISTORY_TABLE uht;
		uht.Count = 0;
		uht.Search = UNWIND_HISTORY_TABLE_NONE;
		uht.LowAddress = -1;
		uht.HighAddress = 0;

		PRUNTIME_FUNCTION pFunc = RtlLookupFunctionEntry(ctx.Rip, &ulImageBase, &uht);

		PEXCEPTION_ROUTINE pfnHdlr = RtlVirtualUnwind
		(
			UNW_FLAG_EHANDLER|UNW_FLAG_UHANDLER,
			ulImageBase,
			ctx.Rip,
			pFunc,
			&ctx,
			&Handlerdata,
			&EstablisherFrame,
			NULL
		);

		int quotient = dividend / divider;
		remainder = dividend % divider;
		return quotient;
	}
	__finally
	//__except ((GetExceptionCode() == EXCEPTION_INT_DIVIDE_BY_ZERO) ?
	//			EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
	{
		printf("Divided by Zero!!!\n");
	}
		return INT_MAX;
}

void YHD_Test()
{
	__try	// TryLevel = 0, 1195
	{
		__try	// TryLevel = 1, 1195
		{
			__try	// TryLevel = 2, 1195
			{
				__try	// TryLevel = 3, 1195
				{
					__try	// TryLevel = 4, 1195
					{
						int nRemainder = 0;
						int nRet = YHD_Division(32, 0, nRemainder);
					}
					__finally
					{
						printf("........\n");
					}	//11B2
				}
				__except (EXCEPTION_CONTINUE_SEARCH)
				{
				}	//11C1
			}
			__finally
			{
				printf("........\n");
			}	//11C1

			__try	// TryLevel = 5, 11CF
			{
				int nRemainder = 0;
				int nRet = YHD_Division(32, 4, nRemainder);
			}
			__finally
			{
				printf("........\n");
			}	//11EF
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}	//11FE

		__try	// TryLevel = 6
		{
		}
		__finally
		{
			printf("........\n");
		}
	}
	__except (EXCEPTION_CONTINUE_SEARCH)
	{
		printf("........EXCEPTION_EXECUTE_HANDLER\n");
	}	//120D

}

void _tmain(int argc, TCHAR* argv[])
{
	YHD_Test();
}
