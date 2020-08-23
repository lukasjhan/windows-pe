#ifndef __CLASSDLL_H__
#define __CLASSDLL_H__

#ifndef CLASSDLL_API
#	define CLASSDLL_API __declspec(dllimport)
#endif

class CLASSDLL_API YhdDC
{
	HWND m_hWnd;
	HDC	 m_hDC;

public:
	YhdDC();
	YhdDC(HWND hWnd);
	~YhdDC();

public:
	static BOOL WINAPI YHD7_IsPointInRect(LPCRECT prc, POINT pt);

	void YHD3_DrawText(PCTSTR pszText, POINT pt);
	SIZE YHD6_CalcTextWidth(PCTSTR szText);

};

#endif	//__CLASSDLL_H__