#include <windows.h>
#include <tchar.h>
#include <stdio.h>


#if !defined _M_IX86
#error	"Not support on X64"
#endif


int BubbleSort(int nArrLen, PINT parBuff)
{
	int nSwapCnt = 0;
	for (int i = 0; i < nArrLen - 1; i++)
	{
		for (int j = 0; j < nArrLen - 1 -i; j++)
		{
			if (parBuff[j] > parBuff[j+1])
			{
				int nTemp = parBuff[j];
				parBuff[j] = parBuff[j+1];
				parBuff[j+1] = nTemp;
				nSwapCnt++;
			}
		}
	}
	return nSwapCnt;
}

int BubbleSort2(int nArrLen, PINT parBuff)
{
	int nSwapCnt = 0;
	__asm
	{
		mov esi, parBuff
		mov ecx, 0
		mov edx, nArrLen
		sub edx, 1
		jmp LabelUpper2

LabelUpper :
		inc ecx
LabelUpper2 :
		cmp ecx, edx
		je LabelEnd

		mov eax, 0
		mov ebx, edx
		sub	ebx, ecx
		jmp LabelLoop3

LabelLoop2 :
		inc eax
LabelLoop3 :
		cmp eax, ebx
		je LabelUpper

		mov edi, dword ptr[esi + eax * 4]
		cmp edi, dword ptr[esi + eax * 4 + 4]
		jle LabelLoop2

		push edi
		mov edi, dword ptr[esi + eax * 4 + 4]
		mov dword ptr[esi + eax * 4], edi
		pop edi
		mov dword ptr[esi + eax * 4 + 4], edi

		mov edi, nSwapCnt
		inc edi
		mov nSwapCnt, edi
		jmp LabelLoop2
	}

LabelEnd:
	return nSwapCnt;
}

void _tmain()
{
	int arBuff[5] = { 8, 5, 3, 1, -9 };
	int nSwapCnt = BubbleSort(5, arBuff);
	printf("Bubble Sort: ");
	for (int i = 0; i < 5; i++)
		printf("%d, ", arBuff[i]);
	printf("\nSwap Count : %d\n", nSwapCnt);
}
