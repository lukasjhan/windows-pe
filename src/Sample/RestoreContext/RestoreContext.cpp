#include "stdafx.h"
#include <windows.h>

#if !defined _M_X64
#error	"Not support on x86-32"
#endif


void WalkStackFrames()
{
	CONTEXT ctx;
	RtlCaptureContext(&ctx);
	DWORD64 ulCtrlPc = ctx.Rip;
	DWORD64 ulStcPtr = ctx.Rsp;

	for (int nIndex = 0; ulCtrlPc != 0; nIndex++)
	{
		DWORD64 ulImgBase = 0;
		PRUNTIME_FUNCTION prf = RtlLookupFunctionEntry(ulCtrlPc, &ulImgBase, NULL);
		if (prf == NULL)
		{
			ulCtrlPc = ctx.Rip = *((PDWORD64)ulStcPtr);
			ctx.Rsp += sizeof(DWORD64);
			ulStcPtr = ctx.Rsp;
			continue;
		}

		CONTEXT ctx2 = ctx;
		DWORD64 ulHandlerdata = 0, ulEstablisherFrame = 0;
		PEXCEPTION_ROUTINE pfnHdlr = RtlVirtualUnwind
		(
			UNW_FLAG_NHANDLER, ulImgBase, ulCtrlPc, prf, &ctx,
			(PVOID*)&ulHandlerdata, &ulEstablisherFrame, NULL
		);

		if (nIndex == 2)
			RtlRestoreContext(&ctx, NULL);

		ulStcPtr = ctx.Rsp;
		ulCtrlPc = ctx.Rip;
	}
}

void F3()
{
	printf("      F3   : before calling WalkStackFrames\n");
	WalkStackFrames();
	printf("      F3   : after  calling WalkStackFrames\n");
}

void F2()
{
	printf("    F2     : before calling F3\n");
	F3();
	printf("    F2     : after  calling F3\n");
}

void F1()
{
	printf("  F1       : before calling F2\n");
	F2();
	printf("  F1       : after  calling F2\n");
}

void _tmain()
{
	printf("Main       : before calling F1\n");
	F1();
	printf("Main       : after  calling F1\n");
}
