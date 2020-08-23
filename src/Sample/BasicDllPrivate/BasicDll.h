#ifndef __BASICDLL_H__
#define __BASICDLL_H__

#ifdef __cplusplus
extern "C"
{
#endif
	void WINAPI YHD3_DrawText(HDC hDC, PCTSTR pszText, POINT pt);
	SIZE WINAPI YHD6_CalcTextWidth(HDC hDC, PCTSTR pszText);
	BOOL WINAPI YHD7_IsPointInRect(LPCRECT prc, POINT pt);
#ifdef __cplusplus
}
#endif

#endif	//__BASICDLL_H__
