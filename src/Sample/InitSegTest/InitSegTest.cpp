#include "stdio.h"

class MyClass
{
public:
	MyClass()
	{
		printf("MyClass()\n");
	}
	~MyClass()
	{
		printf("~MyClass()\n");
	}
};
MyClass G_AAA;

void main()
{
}
