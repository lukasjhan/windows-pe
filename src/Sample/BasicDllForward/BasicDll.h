#ifndef __BASICDLL_H__
#define __BASICDLL_H__

#ifndef BASICDLL_API
#	define BASICDLL_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" 
{
#endif

BASICDLL_API TCHAR YHD4_MSG_BUFF[64];

void WINAPI YHD3_DrawText(HDC hDC, PCTSTR pszText, POINT pt);
SIZE WINAPI YHD6_CalcTextWidth(HDC hDC, PCTSTR pszText);
BOOL WINAPI YHD7_IsPointInRect(LPCRECT prc, POINT pt);
BOOL WINAPI YHD9_TextOut(HDC hdc, int x, int y, LPCSTR lpString, int c);

#ifdef __cplusplus
}
#endif

#endif	//__BASICDLL_H__