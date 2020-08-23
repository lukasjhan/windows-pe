#include "stdafx.h"

int add(int a, int b)
{
	return (a + b);
}

void _tmain()
{
	int x = 3, y = 5;
	int r = add(x, y);

	printf("%d + %d = %d\n", x, y, r);
}
