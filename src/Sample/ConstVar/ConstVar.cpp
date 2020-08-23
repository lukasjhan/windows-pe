#include "stdafx.h"

const int	C_INL = 2;
const int	C_INT = 3;
int			G_INT = 5;
const int*	P_CINT = &C_INT;
int* const	CP_INT = &G_INT;

const int		ARR_CINT[3] = { 2, 12, 27 };
const wchar_t	ARR_CSTR[16] = L"STRING TEST.";

const wchar_t*	P_CSTR = L"PTR for CONST STR.";
wchar_t* const	CP_STR = L"CONST PTR for STR.";

const wchar_t*	PARR_CSTR[3] = { L"111", L"222", L"333" };
wchar_t* const	CPARR_STR[3] = { L"333", L"222", L"111" };

void main()
{
	printf("C_INL=%d, C_INT=%d, G_INT=%d, P_CINT=%d, CP_INT=%d\n",
		C_INL, C_INT, G_INT, *P_CINT, *CP_INT);

	for (int i = 0; i < 3; i++)
		printf("ARR_CINT[%d]=%d, ", i, ARR_CINT[i]);
	printf("\n");
	printf("ARR_CSTR=%S\n", ARR_CSTR);

	printf("P_CSTR=%S\n", P_CSTR);
	printf("CP_STR=%S\n", CP_STR);

	for (int i = 0; i < 3; i++)
	{
		printf("PARR_CSTR[%d]=%S, CPARR_STR[%d]=%S\n", 
			i, PARR_CSTR[i], i, CPARR_STR[i]);
	}
}
