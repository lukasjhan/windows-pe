#include <windows.h>
#include <tchar.h>
#include <stdio.h>

#ifdef _WIN64
extern "C" PBYTE GetTibPtr();
#endif

#ifdef _WIN64
#define OFF_ExceptionList			0x000
#define OFF_StackBase				0x008
#define OFF_StackLimit				0x010
#define OFF_Self					0x030

#define OFF_ProcessEnvironmentBlock 0x0060
#define OFF_LastErrorValue			0x0068
#define OFF_CurrentLocale			0x0108
#define OFF_TlsSlots				0x1480 
#define OFF_TlsExpansionSlots		0x1780
#else
#define OFF_ExceptionList			0x000
#define OFF_StackBase				0x004
#define OFF_StackLimit				0x008
#define OFF_Self					0x018

#define OFF_ProcessEnvironmentBlock 0x0030
#define OFF_LastErrorValue			0x0034
#define OFF_CurrentLocale			0x00c4
#define OFF_TlsSlots				0x0e10
#define OFF_TlsExpansionSlots		0x0f6c
#endif

PCWSTR GSZ_FLDNAMES[] =
{
	L"ExceptionList     ",
	L"StackBase         ",
	L"StackLimit        ",
	L"Self              ",
	L"PEB               ",
	L"LastErrorValue    ",
	L"CurrentLocale     ",
	L"TlsSlots",
	L"TlsExpansionSlots "
};

INT GN_FLDOFFSETS[] =
{
	OFF_ExceptionList,
	OFF_StackBase,
	OFF_StackLimit,
	OFF_Self,
	OFF_ProcessEnvironmentBlock,
	OFF_LastErrorValue,
	OFF_CurrentLocale,
	OFF_TlsSlots,
	OFF_TlsExpansionSlots
};


void _tmain()
{
	SetLastError(ERROR_FILE_INVALID);

	int nTlsVal = 32;
	DWORD dwTlsIdx = TlsAlloc();
	TlsSetValue(dwTlsIdx, &nTlsVal);

	PBYTE pTib = NULL;
#ifdef _WIN64
	pTib = GetTibPtr();
#else
	__asm
	{
		mov eax, fs:[18h]
		mov pTib, eax
	}
#endif
	printf("TIB: 0x%p, ThreadID: %d ================\n", pTib, GetCurrentThreadId());

	int nFldCnt = sizeof(GSZ_FLDNAMES) / sizeof(PCWSTR);
	for (int i = 0; i < nFldCnt; i++)
	{
		if (i > 4 && i < 7)
			printf("\t%S: 0x%08X\n", GSZ_FLDNAMES[i], *((PDWORD)(pTib + GN_FLDOFFSETS[i])));
		else if (i == 7)
		{
			PDWORD_PTR pTlsSlots = (PDWORD_PTR)(pTib + GN_FLDOFFSETS[i]);
			printf("\t%S[%d]       : 0x%p, &nTlsVal=0x%p\n",
				GSZ_FLDNAMES[i], dwTlsIdx, pTlsSlots[dwTlsIdx], &nTlsVal);
		}
		else
			printf("\t%S: 0x%p\n", GSZ_FLDNAMES[i], *((PDWORD_PTR)(pTib + GN_FLDOFFSETS[i])));
	}
	TlsFree(dwTlsIdx);
}
