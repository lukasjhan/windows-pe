#include "stdio.h"
#pragma warning(disable : 4075)

class MyClass
{
	const char* _pszClaName;

public:
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


typedef void(__cdecl *_PF)(void);

#pragma section(".YHD$A", read)
__declspec(allocate(".YHD$A")) const _PF InitSegBegin = (_PF)1;
#pragma section(".YHD$Z",read)
__declspec(allocate(".YHD$Z")) const _PF InitSegEnd	  = (_PF)1;

#pragma init_seg(".YHD$M")
MyClass G_BBB("BBB");
MyClass G_CCC("CCC");


void InitializeObjects()
{
	const _PF* pfn = &InitSegBegin;
	for (++pfn; pfn < &InitSegEnd; ++pfn)
	{
		if (*pfn != NULL)
			(*pfn)();
	}
}

void main()
{
	printf("=====> enter main!!!\n");
	InitializeObjects();
	printf("<===== leave main!!!\n");
}
