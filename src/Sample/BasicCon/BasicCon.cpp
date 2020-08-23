#include "stdafx.h"
#include <stdio.h>

class YHD_CLS
{
	int _x, _y;

public:	
	YHD_CLS()
	{
		printf("YHD_CLS\n");
		_x = _y = 0;
	}
	YHD_CLS(int x, int y)
	{
		printf("YHD_CLS: x=%d, y=%d\n", x, y);
		_x = x, _y = y;
	}
	~YHD_CLS()
	{
		printf("~YHD_CLS(): (%d, %d)\n", _x, _y);
	}
};

YHD_CLS G_AAA;
YHD_CLS G_BBB(2, 4);
static YHD_CLS S_CCC(100, 200);

void main()
{
	static YHD_CLS S_DDD(-3, -1);

	printf("Hello YHD World!!!\n");
	getchar();
}