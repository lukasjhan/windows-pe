#include "stdio.h"

class MyLib
{
public:
	MyLib()
	{
		printf("MyLib()\n");
	}
	~MyLib()
	{
		printf("~MyLib()\n");
	}
};
#pragma init_seg(lib)
MyLib G_LIB;

