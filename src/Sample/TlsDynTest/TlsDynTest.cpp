#include "stdafx.h"
#include "Windows.h"

DWORD g_dwTlsIncIdx;
DWORD g_dwTlsFacIdx;

void Increase(int val)
{
	PINT pnInc = (PINT)TlsGetValue(g_dwTlsIncIdx);
	*pnInc += val;
	TlsSetValue(g_dwTlsIncIdx, pnInc);
}

void Factorial(int val)
{
	PUINT64 pnMul = (PUINT64)TlsGetValue(g_dwTlsFacIdx);
	for (int i = 2; i <= val; i++)
		*pnMul *= i;
	TlsSetValue(g_dwTlsFacIdx, pnMul);
}

DWORD WINAPI ThreadProc(PVOID lParam)
{
	int nVal = (int)lParam;
	PINT pnInc = new int(500);
	TlsSetValue(g_dwTlsIncIdx, pnInc);
	PUINT64 pnFac = new UINT64(200);
	TlsSetValue(g_dwTlsFacIdx, pnFac);

	for (int i = 0; i < 100; i++)
		Increase(nVal);
	Factorial(nVal);

	pnInc = (PINT)TlsGetValue(g_dwTlsIncIdx);
	pnFac = (PUINT64)TlsGetValue(g_dwTlsFacIdx);
	printf("Child Thread #%d\t: Increase=%d, Factorial=%I64u\n",
		GetCurrentThreadId(), *pnInc, *pnFac);
	delete pnFac;
	delete pnInc;
	return 0;
}

void main()
{
	g_dwTlsIncIdx = TlsAlloc();
	g_dwTlsFacIdx = TlsAlloc();

	HANDLE arhWorkers[5];
	DWORD  dwThrId = 0;
	for (int i = 0; i < 5; i++)
		arhWorkers[i] = CreateThread(NULL, 0, ThreadProc, (PVOID)(i + 1), 0, &dwThrId);
	WaitForMultipleObjects(5, arhWorkers, TRUE, INFINITE);
	for (int i = 0; i < 5; i++)
		CloseHandle(arhWorkers[i]);

	TlsFree(g_dwTlsFacIdx);
	TlsFree(g_dwTlsIncIdx);
}

#if 0
int		g_dwInc = 500;
UINT64	g_ulFac = 200;

void Increase(int val)
{
	g_dwInc += val;
}

void Factorial(int val)
{
	for (int i = 2; i <= val; i++)
		g_ulFac *= i;
}

void main()
{
	for (int i = 0; i<100; i++)
		Increase(3);
	Factorial(3);

	printf("Child Thread #%d\t: Increase=%d, Factorial=%I64u\n",
		GetCurrentThreadId(), g_dwInc, g_ulFac);
}
#endif
