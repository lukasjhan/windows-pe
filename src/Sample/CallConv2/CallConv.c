#include <windows.h>
#include <tchar.h>
#include <stdio.h>

int __cdecl Test_CDecl(BOOL bAdd, int a, int b, int c, int d)
{
	if (bAdd)
		return (a + b + c + d);
	else
		return (a - b - c - d);
}

int __stdcall Test_StdCall(BOOL bAdd, int a, int b, int c, int d)
{
	if (bAdd)
		return (a + b + c + d);
	else
		return (a - b - c - d);
}

int __fastcall Test_FastCall(BOOL bAdd, int a, int b, int c, int d)
{
	if (bAdd)
		return (a + b + c + d);
	else
		return (a - b - c - d);
}

void __declspec(naked) Test_Naked(BOOL bAdd, int a, int b, int c, int d)
{
	__asm
	{
		cmp	dword ptr[esp + 4], 0;
		je	LABLE_MINUS;
		mov	eax, dword ptr[esp + 08h];
		add	eax, dword ptr[esp + 0Ch];
		add	eax, dword ptr[esp + 10h];
		add	eax, dword ptr[esp + 14h];
		jmp LABLE_RETURN;
	LABLE_MINUS :
		mov	eax, dword ptr[esp + 08h];
		sub	eax, dword ptr[esp + 0Ch];
		sub	eax, dword ptr[esp + 10h];
		sub	eax, dword ptr[esp + 14h];
	LABLE_RETURN :
		ret;
	}
}


//
//class CallConv
//{
//public:
//	int  Test_cdecl(int a, int b, int c, int d, int e)
//	{
//		return (a + b);
//	}
//	int _stdcall Test_stdc(int a, int b, int c, int d, int e)
//	{
//		return (a + b);
//	}
//
//};

int _tmain()
{
	int c = Test_CDecl(TRUE, 5, 6, 7, 8);
	int s = Test_StdCall(FALSE, 5, 6, 7, 8);
	int f = Test_FastCall(TRUE, 5, 6, 7, 8);
	Test_Naked(FALSE, 5, 6, 7, 8);
	int n = 0;
	__asm mov [n], eax;

	//CallConv cc;
	//int i = cc.Test_cdecl(7, 8, 6, 7, 8);
	//int j = cc.Test_stdc(7, 8, 6, 7, 8);

	printf("cdecl: %d, std: %d, fast: %d, naked: %d\n", c, s, f, n);
}
