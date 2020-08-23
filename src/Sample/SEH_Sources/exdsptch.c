#include "stdafx.h"
#include "windows.h"
#include "ntrtlp.h"


#define EXCEPTION_CHAIN_END	0xFFFFFFFF

//
// Dispatcher context structure definition.
typedef struct _DISPATCHER_CONTEXT 
{
	PEXCEPTION_REGISTRATION_RECORD RegistrationFrame;
} DISPATCHER_CONTEXT;

//
// Execute handler for exception function prototype.
EXCEPTION_DISPOSITION RtlpExecuteHandlerForException
(
	IN PEXCEPTION_RECORD ExceptionRecord,
	IN PVOID EstablisherFrame,
	IN OUT PCONTEXT ContextRecord,
	IN OUT PVOID dc,
	IN PEXCEPTION_ROUTINE ExceptionRoutine
);

//
// Execute handler for unwind function prototype.
EXCEPTION_DISPOSITION RtlpExecuteHandlerForUnwind
(
	IN PEXCEPTION_RECORD ExceptionRecord,
	IN PVOID EstablisherFrame,
	IN OUT PCONTEXT ContextRecord,
	IN OUT PVOID dc,
	IN PEXCEPTION_ROUTINE ExceptionRoutine
);



BOOLEAN RtlDispatchException(PEXCEPTION_RECORD ExceptionRecord, PCONTEXT ContextRecord)
{
	DISPATCHER_CONTEXT dc;
	EXCEPTION_DISPOSITION Disposition;
	PEXCEPTION_REGISTRATION_RECORD pCurFrame, pNestedFrame = NULL;
	ULONG HighAddress, HighLimit, LowLimit;
	EXCEPTION_RECORD er;


	RtlpGetStackLimits(&LowLimit, &HighLimit);

	pCurFrame = RtlpGetRegistrationHead();

	while (pCurFrame != EXCEPTION_CHAIN_END)
	{
		HighAddress = (ULONG)pCurFrame + sizeof(EXCEPTION_REGISTRATION_RECORD);
		if (((ULONG)pCurFrame < LowLimit || HighAddress > HighLimit) ||
			((ULONG)pCurFrame & 0x3) != 0)
		{
			ExceptionRecord->ExceptionFlags |= EXCEPTION_STACK_INVALID;
			return FALSE;
		}

		Disposition = RtlpExecuteHandlerForException
		(
			ExceptionRecord, pCurFrame, ContextRecord, &dc,
			(PEXCEPTION_ROUTINE)pCurFrame->Handler
		);

		// If the current scan is within a nested context and the frame
		// just examined is the end of the context region, then clear
		// the nested context frame and the nested exception in the
		// exception flags.
		if (pNestedFrame == pCurFrame)
		{
			ExceptionRecord->ExceptionFlags &= (~EXCEPTION_NESTED_CALL);
			pNestedFrame = NULL;
		}

		// Case on the handler disposition.
		switch (Disposition)
		{
			case ExceptionContinueExecution:
			if ((ExceptionRecord->ExceptionFlags & EXCEPTION_NONCONTINUABLE) != 0) 
			{
				er.ExceptionCode	= STATUS_NONCONTINUABLE_EXCEPTION;
				er.ExceptionFlags   = EXCEPTION_NONCONTINUABLE;
				er.ExceptionRecord  = ExceptionRecord;
				er.NumberParameters = 0;
				RtlRaiseException(&er);
			}
			else
			{
				return TRUE;
			}

			case ExceptionContinueSearch:
			break;

			case ExceptionNestedException:
			ExceptionRecord->ExceptionFlags |= EXCEPTION_NESTED_CALL;
			if (dc.RegistrationFrame > pNestedFrame)
			{
				pNestedFrame = dc.RegistrationFrame;
			}
			break;

			default:
			er.ExceptionCode	= STATUS_INVALID_DISPOSITION;
			er.ExceptionFlags	= EXCEPTION_NONCONTINUABLE;
			er.ExceptionRecord	= ExceptionRecord;
			er.NumberParameters	= 0;
			RtlRaiseException(&er);
			break;
		}

		pCurFrame = pCurFrame->Next;
	}

	return FALSE;
}


VOID RtlUnwind(PVOID TargetFrame, PVOID TargetIp, PEXCEPTION_RECORD ExceptionRecord, PVOID ReturnValue)
{
	PCONTEXT pCtxRec;
	CONTEXT cr;
	DISPATCHER_CONTEXT dc;
	EXCEPTION_DISPOSITION Disposition;
	PEXCEPTION_REGISTRATION_RECORD pCurFrame, pRemFrame;
	ULONG HighAddress, HighLimit, LowLimit;
	EXCEPTION_RECORD er, er2;

	RtlpGetStackLimits(&LowLimit, &HighLimit);

	if (ExceptionRecord == NULL)
	{
		ExceptionRecord		= &er;
		er.ExceptionCode	= STATUS_UNWIND;
		er.ExceptionFlags	= 0;
		er.ExceptionRecord	= NULL;
		er.ExceptionAddress = RtlpGetReturnAddress();
		er.NumberParameters = 0;
	}

	if (TargetFrame != NULL)
	{
		ExceptionRecord->ExceptionFlags |= EXCEPTION_UNWINDING;
	}
	else
	{
		ExceptionRecord->ExceptionFlags |= (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND);
	}

	pCtxRec = &cr;
	cr.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL | CONTEXT_SEGMENTS;
	RtlpCaptureContext(pCtxRec);
	pCtxRec->Esp += (sizeof(TargetFrame) + sizeof(TargetIp) + sizeof(ExceptionRecord) + sizeof(ReturnValue));
	pCtxRec->Eax = (ULONG)ReturnValue;

	pCurFrame = RtlpGetRegistrationHead();
	while (pCurFrame != EXCEPTION_CHAIN_END)
	{
		if ((ULONG)pCurFrame == (ULONG)TargetFrame) 
		{
			ZwContinue(pCtxRec, FALSE);
		}
		else if (TargetFrame != NULL && (ULONG)TargetFrame < (ULONG)pCurFrame)
		{
			er2.ExceptionCode	 = STATUS_INVALID_UNWIND_TARGET;
			er2.ExceptionFlags	 = EXCEPTION_NONCONTINUABLE;
			er2.ExceptionRecord	 = ExceptionRecord;
			er2.NumberParameters = 0;
			RtlRaiseException(&er2);
		}

		HighAddress = (ULONG)pCurFrame + sizeof(EXCEPTION_REGISTRATION_RECORD);
		if (((ULONG)pCurFrame < LowLimit || HighAddress > HighLimit) ||
			((ULONG)pCurFrame & 0x3) != 0)
		{
			er2.ExceptionCode	 = STATUS_BAD_STACK;
			er2.ExceptionFlags	 = EXCEPTION_NONCONTINUABLE;
			er2.ExceptionRecord	 = ExceptionRecord;
			er2.NumberParameters = 0;
			RtlRaiseException(&er2);
		}
		else
		{
			Disposition = RtlpExecuteHandlerForUnwind
			(
				ExceptionRecord, pCurFrame, pCtxRec, &dc,
				pCurFrame->Handler
			);

			switch (Disposition) 
			{
				case ExceptionContinueSearch:
				break;

				case ExceptionCollidedUnwind:
				pCurFrame = dc.RegistrationFrame;
				break;

				default:
				er2.ExceptionCode	 = STATUS_INVALID_DISPOSITION;
				er2.ExceptionFlags	 = EXCEPTION_NONCONTINUABLE;
				er2.ExceptionRecord	 = ExceptionRecord;
				er2.NumberParameters = 0;
				RtlRaiseException(&er2);
				break;
			}

			pRemFrame = pCurFrame;
			pCurFrame = pCurFrame->Next;
			RtlpUnlinkHandler(pRemFrame);
		}
	}

	if (TargetFrame == EXCEPTION_CHAIN_END) 
	{
		ZwContinue(pCtxRec, FALSE);
	}
	else
	{
		ZwRaiseException(ExceptionRecord, pCtxRec, FALSE);
	}
}