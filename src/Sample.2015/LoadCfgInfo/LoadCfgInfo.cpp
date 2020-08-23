#include "stdafx.h"
#include <stdio.h>

void MyCallbackTest()
{
	printf("MyCallbackTest\n");
}

void main()
{
	void(*MyCallback)();
	MyCallback = MyCallbackTest;

	(*MyCallback)();
}
