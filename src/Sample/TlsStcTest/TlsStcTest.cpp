#include "stdafx.h"
#include "Windows.h"

__declspec(thread) int		gt_dwInc = 500;
__declspec(thread) UINT64	gt_dwFac = 200;

void Increase(int val)
{
	gt_dwInc += val;
}

void Factorial(int val)
{
	for (int i = 2; i <= val; i++)
		gt_dwFac *= i;
}

DWORD WINAPI ThreadProc(PVOID lParam)
{
	int nVal = (int)lParam;
	for (int i = 0; i<100; i++)
		Increase(nVal);
	Factorial(nVal);

	printf("Child Thread #%d\t: Increase=%d, Factorial=%I64u\n",
		GetCurrentThreadId(), gt_dwInc, gt_dwFac);
	return 0;
}

void main()
{
	HANDLE arhWorkers[5];
	DWORD  dwThrId = 0;
	for (int i = 0; i < 5; i++)
		arhWorkers[i] = CreateThread(NULL, 0, ThreadProc, (PVOID)(i + 1), 0, &dwThrId);
	WaitForMultipleObjects(5, arhWorkers, TRUE, INFINITE);
	for (int i = 0; i < 5; i++)
		CloseHandle(arhWorkers[i]);
}
