#include "Windows.h"
#include "tchar.h"

#if _WIN64
TCHAR YHD4_MSG_BUFF[64] = _T("YHD's WinApp : HINSTANCE=0x%016I64X");
#else
TCHAR YHD4_MSG_BUFF[64] = _T("YHD's WinApp : HINSTANCE=0x%08X");
#endif

void WINAPI YHD3_DrawText(HDC hDC, PCTSTR pszText, POINT pt)
{
	TextOut(hDC, pt.x, pt.y, pszText, (int)_tcslen(pszText));
}

SIZE WINAPI YHD6_CalcTextWidth(HDC hDC, PCTSTR szText)
{
	SIZE si;
	if (!GetTextExtentPoint32(hDC, szText, (int)_tcslen(szText), &si))
		si.cx = si.cy = 0;
	return si;
}

BOOL WINAPI YHD7_IsPointInRect(LPCRECT prc, POINT pt)
{
	return PtInRect(prc, pt);
}
