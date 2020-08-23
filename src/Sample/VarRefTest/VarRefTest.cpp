#include "stdafx.h"
#include <windows.h>
#include <excpt.h>


struct MyStruct
{
	char	_chVal;
	short	_shVal;
	int		_nVal;
};

int g_nTeatVal;
MyStruct g_ms;


//void SetMyStruct(MyStruct* pms)
//{
//	pms->_shVal = 4;
//	pms->_nVal = -25;
//	pms->_chVal = 'A';
//}

void SetMyStruct(int nPrm1, char chPrm2, short shPrm3)
{
	int		nLoc1 = 0;
	char	chLoc2 = 0;
	short	shLoc3 = 0;

	nLoc1  = nPrm1;
	chLoc2 = chPrm2;
	shLoc3 = shPrm3;

	g_nTeatVal = nLoc1 + chLoc2 + shLoc3;
}

void _tmain()
{
	g_nTeatVal = 16;

	SetMyStruct(32, 'A', -5);
	//g_ms._shVal = -2;
	//g_ms._nVal = 16;
	//memset(g_ms._szBuff, 0, sizeof(g_ms._szBuff));
}
