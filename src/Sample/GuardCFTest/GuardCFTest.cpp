#include "stdafx.h"
#include <stdio.h>

void myfunc1()
{
	printf("myfunc1\n");
}

void main()
{
	void(*myfunc)();
	myfunc = myfunc1;
	(*myfunc)();
}
