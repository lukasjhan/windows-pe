#include <windows.h>
#include <tchar.h>
#include <stdio.h>


#if !defined _M_IX86
#error	"Not support on X64"
#endif


void asm_memcpy(PVOID pDst, PVOID pSrc, int nLen)
{
	__asm
	{
		mov esi, pSrc
		mov edi, pDst
		mov ecx, nLen
		rep movs byte ptr [edi], byte ptr [esi]
	}
}

void asm_memset(PVOID pDst, int nVal, int nLen)
{
	__asm
	{
		mov eax, nVal
		mov edi, pDst
		mov ecx, nLen
		rep stosb
	}
}

int asm_memcmp(PVOID pDst, PVOID pSrc, int nLen)
{
	int nRet;

	__asm
	{
		mov esi, pSrc
		mov edi, pDst
		mov ecx, nLen
		rep cmpsb

		jg	RLT_GREATER
		jl	RLT_LESS
		xor	eax, eax
		jmp	RLT_SET

	RLT_GREATER:
		mov eax, 1
		jmp	RLT_SET

	RLT_LESS:
		mov eax, 0FFFFFFFFh

	RLT_SET:
		mov nRet, eax
	}

	return nRet;
}

void _tmain()
{
	int arBuff1[5] = { 8, 5, 3, 1, -9 };
	int arBuff2[5];

	printf("memset => ");
	asm_memset(arBuff2, 0, 5 * sizeof(int));
	for (int i = 0; i < 5; i++)
		printf("%d, ", arBuff2[i]);
	printf("\n");

	printf("memcpy => ");
	asm_memcpy(arBuff2, arBuff1, 5 * sizeof(int));
	for (int i = 0; i < 5; i++)
		printf("%d, ", arBuff2[i]);
	printf("\n");

	printf("memcmp => ");
	int nCmp = asm_memcmp(arBuff1, arBuff2, 5 * sizeof(int));
	printf("Result %d, %sequal\n", nCmp, (nCmp != 0) ? "not " : "");
}
