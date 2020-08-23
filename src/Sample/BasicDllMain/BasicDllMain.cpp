#include "Windows.h"
#include "tchar.h"

#ifdef __cplusplus
#	define BASICDLL_API extern "C" __declspec(dllexport)
#else
#	define BASICDLL_API __declspec(dllexport)
#endif
#include "BasicDll.h"


TCHAR YHD4_MSG_BUFF[64] = _T("YHD_BASIC_DLL_MAIN");

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

BOOL WINAPI DllMain(HANDLE  hDllHandle, DWORD dwReason, PVOID preserved)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_PROCESS_DETACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		break;
	}
	return TRUE;
}
