#include "windows.h"

#define EXCEPTION_VCPP_RAISE	0xE06D7363
void (__cdecl * const _pDestructExceptionObject)(PEXCEPTION_RECORD, BOOLEAN);

typedef PSCOPE_TABLE_AMD64 PC_SCOPE_TABLE;


EXCEPTION_DISPOSITION __C_specific_handler2
(
	_In_	 PEXCEPTION_RECORD		ExceptionRecord,
	_In_	 PVOID					EstablisherFrame,
	_Inout_	 PCONTEXT				ContextRecord,
	_Inout_	 PDISPATCHER_CONTEXT	DispatcherContext
)
{
	DWORD64 ImageBase = DispatcherContext->ImageBase;
	DWORD64 ControlPc = DispatcherContext->ControlPc - ImageBase;
	PC_SCOPE_TABLE ScopeTable = (PC_SCOPE_TABLE)DispatcherContext->HandlerData;

	if((ExceptionRecord->ExceptionFlags & EXCEPTION_UNWIND) == 0)
	{
		EXCEPTION_POINTERS ExceptionPointers;
		ExceptionPointers.ExceptionRecord = ExceptionRecord;
		ExceptionPointers.ContextRecord   = ContextRecord;
		
		DWORD Index = DispatcherContext->ScopeIndex;
		while (Index < ScopeTable->Count)
		{
			if (ControlPc <  ScopeTable->ScopeRecord[Index].BeginAddress ||
				ControlPc >= ScopeTable->ScopeRecord[Index].EndAddress ||
				ScopeTable->ScopeRecord[Index].JumpTarget == 0)
			{
				Index++;
				continue;
			}

			LONG Value = 1;
			if (ScopeTable->ScopeRecord[Index].HandlerAddress != 1)
			{
				PEXCEPTION_FILTER ExceptionFilter = (PEXCEPTION_FILTER)
					(ImageBase + ScopeTable->ScopeRecord[Index].HandlerAddress);
				Value = ExceptionFilter(EstablisherFrame, &ExceptionPointers);
			}

			if(Value <= 0)
			{
				if (Value < 0)
					return ExceptionContinueExecution;

				Index++;
				continue;
			}

			if (ExceptionRecord->ExceptionCode == EXCEPTION_VCPP_RAISE) 
			{
				if(_pDestructExceptionObject != NULL)
				{
					if(!_IsNonwritableInCurrentImage(&_pDestructExceptionObject))
					{
						_pDestructExceptionObject(ExceptionRecord, TRUE);
					}
				}
			}

			
			DWORD64 Handler = ScopeTable->ScopeRecord[Index].JumpTarget + ImageBase;
			_NLG_Notify(Handler, EstablisherFrame, 1);

			RtlUnwindEx
			(
				EstablisherFrame,
				(PVOID)(ImageBase + ScopeTable->ScopeRecord[Index].JumpTarget),
				ExceptionRecord,
				(PVOID)ExceptionRecord->ExceptionCode,	// ReturnValue,
				DispatcherContext->ContextRecord,
				DispatcherContext->HistoryTable
			);
			__NLG_Return2();
			//Index++;
		}
	}
	else
	{
		DWORD64 TargetPc = DispatcherContext->TargetIp - ImageBase;
		DWORD Index = DispatcherContext->ScopeIndex;

		while (Index < ScopeTable->Count)
		{
			if (ControlPc <  ScopeTable->ScopeRecord[Index].BeginAddress ||
				ControlPc >= ScopeTable->ScopeRecord[Index].EndAddress)
			{
				Index++;
				continue;
			}

			if((ExceptionRecord->ExceptionFlags & EXCEPTION_TARGET_UNWIND) != 0)
			{
				DWORD TargetIndex = 0;
				while (TargetIndex < ScopeTable->Count)
				{
					if ((ControlPc >= ScopeTable->ScopeRecord[TargetIndex].BeginAddress &&
						 ControlPc <  ScopeTable->ScopeRecord[TargetIndex].EndAddress) &&
						(ScopeTable->ScopeRecord[Index].JumpTarget     == ScopeTable->ScopeRecord[TargetIndex].JumpTarget &&
						 ScopeTable->ScopeRecord[Index].HandlerAddress == ScopeTable->ScopeRecord[TargetIndex].HandlerAddress))
						break;
					TargetIndex++;
				}
						
				if(TargetIndex != ScopeTable->Count)
					return ExceptionContinueSearch;
			}

			if (ScopeTable->ScopeRecord[Index].JumpTarget != 0)
			{
				if (TargetPc != ScopeTable->ScopeRecord[Index].JumpTarget)
					continue;
				else
					return ExceptionContinueSearch;
			}
			
			PTERMINATION_HANDLER TerminationHandler = (PTERMINATION_HANDLER)
				(ScopeTable->ScopeRecord[Index].HandlerAddress + ImageBase);
			TerminationHandler(TRUE, EstablisherFrame);

			Index++;
		}
	}
	
	return ExceptionContinueSearch;
}
