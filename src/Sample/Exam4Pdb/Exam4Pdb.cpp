#include <windows.h>
#include <stdio.h>

int G_VAL = 3;
const char* G_MSG = "HAHAHAHAHAHA";

enum YHD_COLOR
{
	YHD_RED = 1, YHD_GREEN = 5, YHD_BLUE = 11
};

union YHD_Union
{
	short	_nWord;
	int		_nDWord;
	__int64	_nQward;
};

typedef int YHD_INT;
YHD_INT	G_YHD_INT;

struct YHD_Struct
{
	short	_nWord;
	int		_nDWord;
	__int64	_nQward;

	int		_bfVal1 :  3;
	int		_bfVal2 : 16;
	int		_bfVal3 : 13;

};

class YHD_Base
{
private:
	int m_Base;

public:
	void Func1()
	{
		printf("Func1.....\n");
	}
};
YHD_Base G_AAA;

class YHD_Test : public YHD_Base
{ 
	friend class CImpYHD;
	static int TEST_STATIC;

private:
	int m_Test;

public:
	void Func2()
	{
		printf("Func2 %s\n", G_MSG);
	}
};
int YHD_Test::TEST_STATIC;
YHD_Test G_BBB;

struct SVertBase
{
	int m_ref;
	virtual int FuncPure(int x) = 0;
	virtual void FuncVert() {}
};

class CImpVert : public SVertBase
{
public:
	int FuncPure(int x)
	{
		return x;
	}
	void FuncVert()
	{
		printf("FuncPure.....\n");
	}
};

//typedef struct
//{
//	int _a, _b;
//} YHD_TYPEDEF;
//
class YHD_ITER
{
	typedef YHD_ITER _MY_YHD_ITER;

	int m_k, m_l;

	_MY_YHD_ITER* Next;
};
YHD_ITER g_YHD_ITER;

// YHD_TestFunc
static __int64 YHD_TestFunc(int prm1, short prm2, __int64 prm3)
{
	int loc1 = prm1 + prm2;
	__int64 ret = prm1 - prm2;

	for (int i = 0; i < 12; i++)
	{
		__int64 blkVal = ret++;
		if (blkVal == 10)
			goto LABEL_YHD_TEST;
		G_VAL++;
	}

LABEL_YHD_TEST:
	return ret;
}

void Power(int a, int b)
{
	printf("a b");
}

struct B
{
	__int64	m_b;
	int 	m_d;
};

struct C
{
	int	m_c;
};

class A : public B, public C
{
	int val;

public:
	class S
	{
		int m_s;
	};

	typedef int NESTED_INT;
	void Func() {}
};


// Main Function
void wmain()
{
	static YHD_Struct G_SSS;
	memset(&G_SSS, 0, sizeof(YHD_Struct));

	YHD_Union yhdUn;
	yhdUn._nQward = 0;

	YHD_COLOR color = YHD_BLUE;

	PTHREAD_START_ROUTINE PFN = NULL;


	G_AAA.Func1();
	G_BBB.Func2();

	CImpVert impl;
	impl.FuncVert();

	A a;
	a.Func();

	__int64 fret = YHD_TestFunc(3, -1, 12);
	printf("YHD_TestFunc -> %I64d\n", fret);
}
