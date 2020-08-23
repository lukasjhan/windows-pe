#include "Windows.h"
#include "tchar.h"

#define CLASSDLL_API __declspec(dllexport)
#include "ClassDll.h"



YhdDC::YhdDC()
{
	m_hDC = NULL;
}

YhdDC::YhdDC(HWND hWnd)
{
	m_hWnd = hWnd;
	m_hDC = GetDC(hWnd);
}

YhdDC::~YhdDC()
{
	if (m_hDC != NULL)
		ReleaseDC(m_hWnd, m_hDC);
}


BOOL WINAPI YhdDC::YHD7_IsPointInRect(LPCRECT prc, POINT pt)
{
	return PtInRect(prc, pt);
}


void YhdDC::YHD3_DrawText(PCTSTR pszText, POINT pt)
{
	TextOut(m_hDC, pt.x, pt.y, pszText, (int)_tcslen(pszText));
}

SIZE YhdDC::YHD6_CalcTextWidth(PCTSTR szText)
{
	SIZE si;
	if (!GetTextExtentPoint32(m_hDC, szText, (int)_tcslen(szText), &si))
		si.cx = si.cy = 0;
	return si;
}
