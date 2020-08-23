#include "Windows.h"
#include "tchar.h"

#define BASICDLL_API	extern
#include "BasicDll.h"


TCHAR YHD4_MSG_BUFF[64] = _T("YHD_BASIC_DLL_MDS");

void YHD3_DrawText(HDC hDC, PCTSTR pszText, POINT pt)
{
	TextOut(hDC, pt.x, pt.y, pszText, (int)_tcslen(pszText));
}

SIZE YHD6_CalcTextWidth(HDC hDC, PCTSTR szText)
{
	SIZE si;
	if (!GetTextExtentPoint32(hDC, szText, (int)_tcslen(szText), &si))
		si.cx = si.cy = 0;
	return si;
}

BOOL YHD7_IsPointInRect(LPCRECT prc, POINT pt)
{
	return PtInRect(prc, pt);
}
