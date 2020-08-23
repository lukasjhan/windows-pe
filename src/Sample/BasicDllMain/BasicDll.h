#ifndef __BASICDLL_H__
#define __BASICDLL_H__

#ifndef BASICDLL_API
#	ifdef __cplusplus
#		define BASICDLL_API extern "C" __declspec(dllimport)
#	else
#		define BASICDLL_API __declspec(dllimport)
#	endif
#endif

BASICDLL_API TCHAR YHD4_MSG_BUFF[64];

BASICDLL_API void WINAPI YHD3_DrawText(HDC hDC, PCTSTR pszText, POINT pt);
BASICDLL_API SIZE WINAPI YHD6_CalcTextWidth(HDC hDC, PCTSTR pszText);
BASICDLL_API BOOL WINAPI YHD7_IsPointInRect(LPCRECT prc, POINT pt);

#endif	//__BASICDLL_H__