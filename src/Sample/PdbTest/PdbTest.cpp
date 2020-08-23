#include "stdafx.h"

int G_VAL = 3;

enum YHD_ENUM_COLOR : short
{
	CLR_RED = 1,
	CLR_BLUE = 5,
	CLR_YELLO = 20
};

class MyClass
{
	const char* _pszClaName;

public:
	typedef int YHD_TYPE;
	MyClass()
	{

	}
	MyClass(const char* clsName)
	{
		_pszClaName = clsName;
		printf("MyClass: %s\n", _pszClaName);
	}
	~MyClass()
	{
		printf("~MyClass: %s\n", _pszClaName);
	}
};
MyClass G_AAA("AAA");

class MyDown : public MyClass
{
	int m_b;
	int m_bfTest1 : 3;
	int m_bfTest2 : 16;
	int m_bfTest3 : 13;

	struct Nested
	{
		int x, y;
	};
public:
	MyDown()
	{
		m_b = 0;
		printf("MyDown: %d\n", m_b);
	}
	~MyDown()
	{
		printf("~MyDown: %d\n", m_b);
	}
};
MyDown G_BBB;
static int SSSS = 0;

union TEST_UN
{
	int A;
	struct 
	{
		int x, y;
	} BBB;
};

typedef MyDown MY_DOWN;
MyClass::YHD_TYPE GM_TYPEDEF;

//typedef struct
//{
//	int _a, _b;
//} YHD_TYPEDEF;
//
//class YHD_ITER
//{
//	typedef YHD_ITER _MY_YHD_ITER;
//
//	int m_k, m_l;
//
//	_MY_YHD_ITER* Nezt;
//};
//
//YHD_ITER g_YHD_ITER;

struct IMyUnkown
{
	virtual int AddRef() = 0;
	virtual void Release() = 0;
};

class ImplMyUnk : public IMyUnkown
{
public:
	int AddRef()
	{
		return 1;
	}
	void Release()
	{
	}
};

// YHD_TestFunc
static __int64 YHD_TestFunc(int prm1, short prm2, __int64 prm3)
{
	int loc1 = prm1 + prm2;
	__int64 ret = prm1 - prm2;
	MY_DOWN AB;
	//YHD_TYPEDEF AAA;
	static int YHD_LOC = 0;
	YHD_ENUM_COLOR clr = CLR_RED;
	GM_TYPEDEF = 5;

	ImplMyUnk MYUNK;
	MYUNK.AddRef();

	// try~except
	try
	{
		if (loc1 == 0)
			throw (long)-1;

		clr = CLR_BLUE;
		SSSS = 7;
		YHD_LOC = 8;
		for (int i = 0; i < 12; i++)
		{
			ret++;
			if (ret == 10)
				goto LABEL_YHD_TEST;
			if (ret == 11)
				goto LABEL_YHD_block;
			G_VAL++;
		}
	}
	catch (long e)
	{
		printf("Error code = %d\n", e);
	}

	// test2
	{
		__int64 k = 0;
		k = prm1 + prm2 + prm3;
	LABEL_YHD_block:
		ret = k;
	}

LABEL_YHD_TEST:
	return ret;
}

void Power(int a, int b)
{
	printf("a b");
}

#include <Windows.h>

// Main Function
void _tmain()
{
	__int64 fret = YHD_TestFunc(3, -1, 12);
	printf("YHD_TestFunc -> %I64d\n", fret);
}
