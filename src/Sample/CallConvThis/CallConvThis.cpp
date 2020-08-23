#include <windows.h>
#include <tchar.h>
#include <stdio.h>

class CallConv
{
	int m_opRet;

public:
	int __thiscall Test_ThisCall(BOOL bAdd, __int64 a, int b, short c)
	{
		int nRet = 0;
		if (bAdd)
			nRet = ((int)a + b + c);
		else
			nRet = ((int)a - b - c);
		m_opRet = nRet;
		return nRet;
	}

	int __cdecl Test_CDecl(BOOL bAdd, __int64 a, int b, short c)
	{
		int nRet = 0;
		if (bAdd)
			nRet = ((int)a + b + c);
		else
			nRet = ((int)a - b - c);
		m_opRet = nRet;
		return nRet;
	}

	int __stdcall Test_StdCall(BOOL bAdd, __int64 a, int b, short c)
	{
		int nRet = 0;
		if (bAdd)
			nRet = ((int)a + b + c);
		else
			nRet = ((int)a - b - c);
		m_opRet = nRet;
		return nRet;
	}

	int __fastcall Test_FastCall(BOOL bAdd, __int64 a, short b, short c)
	{
		int nRet = 0;
		if (bAdd)
			nRet = ((int)a + b + c);
		else
			nRet = ((int)a - b - c);
		m_opRet = nRet;
		return nRet;
	}
};


void _tmain()
{
	CallConv cc;

	int t = cc.Test_ThisCall(FALSE, 5, 6, 7);

	int c = cc.Test_CDecl(TRUE, 5, 6, 7);

	int s = cc.Test_StdCall(FALSE, 5, 6, 7);

	int f = cc.Test_FastCall(TRUE, 5, 6, 7);

	printf("this: %d, cdecl: %d, std: %d, fast: %d\n", t, c, s, f);
}
