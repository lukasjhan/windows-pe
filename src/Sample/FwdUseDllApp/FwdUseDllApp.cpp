#include <Windows.h>
#include <tchar.h>

//#define USE_MODULE_DEF

#ifdef USE_MODULE_DEF
#	include "../BasicDllMD2/BasicDll.h" 
#	ifdef _WIN64
#		ifdef _DEBUG 
#			pragma comment(lib, "../0.bin/x64/Debug/BasicDllMD2.lib")
#		else
#			pragma comment(lib, "../0.bin/x64/Release/BasicDllMD2.lib")
#		endif
#	else
#		ifdef _DEBUG 
#			pragma comment(lib, "../0.bin/x86/Debug/BasicDllMD2.lib")
#		else
#			pragma comment(lib, "../0.bin/x86/Release/BasicDllMD2.lib")
#		endif
#	endif
#else
#	include "../BasicDllDE2/BasicDll.h" 
#	ifdef _WIN64
#		ifdef _DEBUG 
#			pragma comment(lib, "../0.bin/x64/Debug/BasicDllDE2.lib")
#		else
#			pragma comment(lib, "../0.bin/x64/Release/BasicDllDE2.lib")
#		endif
#	else
#		ifdef _DEBUG 
#			pragma comment(lib, "../0.bin/x86/Debug/BasicDllDE2.lib")
#		else
#			pragma comment(lib, "../0.bin/x86/Release/BasicDllDE2.lib")
#		endif
#	endif
#endif

TCHAR G_APP_CLS[] = _T("YHD_HELLO_WND");
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                   PTSTR szCmdLine, int iCmdShow)
{
	////////////////////////////////////////////////////////////////////////////
	//윈도우즈 클래스 등록
	WNDCLASSEX wc;
	wc.cbSize		 = sizeof(wc);
	wc.style		 = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInst;
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.hIconSm		 = NULL;
	wc.lpszClassName = G_APP_CLS;
	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, _T("RegisterClassEx API call failed!"),
			G_APP_CLS, MB_ICONERROR);
		return 0;
	}

	////////////////////////////////////////////////////////////////////////////
	//윈도우즈 생성
	HWND hWnd = CreateWindowEx
	(
		0,
		G_APP_CLS,					// window class name
		_T("YHD Hello Program"),	// window caption
		WS_OVERLAPPEDWINDOW,		// window style
		CW_USEDEFAULT,              // initial x position
		CW_USEDEFAULT,              // initial y position
		400,						// initial x size
		150,						// initial y size
		NULL,                       // parent window handle
		NULL,                       // window menu handle
		hInst,						// program instance handle
		NULL						// creation parameters
	);
	if (hWnd == NULL)
	{
		MessageBox(NULL, _T("CreateWindowEx API call failed!"),
			G_APP_CLS, MB_ICONERROR);
		return 0;
	}
	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	////////////////////////////////////////////////////////////////////////////
	//메시지 루프
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
 * 종  류 : 윈도우즈 프로시저 함수(콜백 함수)
 * 소  속 : 없음
 * 이  름 : WndProc
 * 인  수 : HWND   hWnd   [IN] : 메시지를 받을 윈도우 핸들
 *          UINT   uMsg   [IN] : 메시지
 *          WPARAM wParam [IN] : 인자1
 *          LPARAM lParam [IN] : 인자2
 * 반  환 : LRESULT : 
 * 내  용 : 윈도우즈 프로시저
 *          WINDOWCLASS의 lpfnWndProc 필드에 이 함수의 번지를 등록해야 한다.
 *          윈도우의 동작을 이 함수 내에서 컨트롤한다.
 * ----------------------------------------------------------------------------
 * 저  자 : Yi HoDong
 * 날  짜 : 2001.08.06
 * 버  젼 : 1.0
 * 회  사 : YHD Works Co. (http://www.yhdworks.com)
 ******************************************************************************/
LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static TCHAR szMsg[] = _T("The Most Simple Windows GUI Program by YHD.");
	switch (uMsg)
	{
		case WM_LBUTTONDOWN:
		{
			TCHAR szOutMsg[128];
			HDC hDC = GetDC(hWnd);
			POINT pt;

			pt.x = LOWORD(lParam), pt.y = HIWORD(lParam);
			wsprintf(szOutMsg, _T("Mouse Position => X : %d, Y : %d"),
				pt.x, pt.y);
			YHD3_DrawText(hDC, szOutMsg, pt);
			ReleaseDC(hWnd, hDC);
		}
		return 0;

		case WM_RBUTTONDOWN:
		{
			HDC hDC = GetDC(hWnd);
			SIZE si = YHD6_CalcTextWidth(hDC, szMsg);
			ReleaseDC(hWnd, hDC);

			TCHAR szMsg[128];
			wsprintf(szMsg, _T("Width : %d, Height : %d pixels"), si.cx, si.cy);
			MessageBox(hWnd, szMsg, _T("Test"), MB_OK | MB_ICONINFORMATION);
		}
		return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			RECT        rc;
			HDC hDC = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rc);
			DrawText(hDC, szMsg, (int)_tcslen(szMsg),
				&rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			EndPaint(hWnd, &ps);
		}
		return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
////////////////////////////////////////////////////////////////////////////////