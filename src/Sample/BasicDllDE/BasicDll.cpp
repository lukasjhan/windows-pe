#include "Windows.h"
#include "tchar.h"

__declspec(dllexport) TCHAR YHD4_MSG_BUFF[64] = _T("YHD_BASIC_DLL_DE");

__declspec(dllexport) void WINAPI YHD3_DrawText(HDC hDC, PCTSTR pszText, POINT pt)
{
	YHD4_MSG_BUFF[0] = 0;
	TextOut(hDC, pt.x, pt.y, pszText, (int)_tcslen(pszText));
}

__declspec(dllexport) SIZE WINAPI YHD6_CalcTextWidth(HDC hDC, PCTSTR szText)
{
	SIZE si;
	if (!GetTextExtentPoint32(hDC, szText, (int)_tcslen(szText), &si))
		si.cx = si.cy = 0;
	return si;
}

__declspec(dllexport) BOOL WINAPI YHD7_IsPointInRect(LPCRECT prc, POINT pt)
{
	return PtInRect(prc, pt);
}
