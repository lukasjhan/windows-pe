#ifndef __BASICDLL_H__
#define __BASICDLL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef BASICDLL_API
	__declspec(dllimport) TCHAR YHD4_MSG_BUFF[64];
#else
	__declspec(dllexport) TCHAR YHD4_MSG_BUFF[64] = _T("YHD_BASIC_DLL_DE2");
#endif

	__declspec(dllexport) void YHD3_DrawText(HDC hDC, PCTSTR pszText, POINT pt);
	__declspec(dllexport) SIZE YHD6_CalcTextWidth(HDC hDC, PCTSTR pszText);
	__declspec(dllexport) BOOL YHD7_IsPointInRect(LPCRECT prc, POINT pt);

#ifdef __cplusplus
}
#endif

#endif	//__BASICDLL_H__