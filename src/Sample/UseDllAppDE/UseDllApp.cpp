#include <Windows.h>
#include <tchar.h>

#include "../BasicDllDE/BasicDll.h" 
#ifdef _WIN64
#	ifdef _DEBUG 
#		pragma comment(lib, "../0.bin/x64/Debug/BasicDllDE.lib")
#	else
#		pragma comment(lib, "../0.bin/x64/Release/BasicDllDE.lib")
#	endif
#else
#	ifdef _DEBUG 
#		pragma comment(lib, "../0.bin/x86/Debug/BasicDllDE.lib")
#	else
#		pragma comment(lib, "../0.bin/x86/Release/BasicDllDE.lib")
#	endif
#endif


TCHAR G_APP_CLS[16] = _T("YHD_HELLO_WND");
HINSTANCE g_hInstance;
#if _WIN64
PCTSTR C_YHD_MSG = _T("YHD's WinApp : HINSTANCE=0x%016I64X");
#else
PCTSTR C_YHD_MSG = _T("YHD's WinApp : HINSTANCE=0x%08X");
#endif

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst,
	PTSTR pszCmdLine, int nCmdShow)
{
	g_hInstance = hInstance;

	////////////////////////////////////////////////////////////////////////////
	//윈도우즈 클래스 등록
	////////////////////////////////////////////////////////////////////////////
	WNDCLASSEX wc;
	wc.cbSize = sizeof(wc);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.hIconSm = NULL;
	wc.lpszClassName = G_APP_CLS;
	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, _T("RegisterClassEx FAILED!!!"),
			G_APP_CLS, MB_ICONERROR);
		return 0;
	}
	////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////
	//윈도우즈 생성
	////////////////////////////////////////////////////////////////////////////
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
		hInstance,					// program instance handle
		NULL						// creation parameters
		);
	if (hWnd == NULL)
	{
		MessageBox(NULL, _T("CreateWindowEx FAILED!!!"),
			G_APP_CLS, MB_ICONERROR);
		return 0;
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////
	//메시지 루프
	////////////////////////////////////////////////////////////////////////////
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	////////////////////////////////////////////////////////////////////////////

	return (int)msg.wParam;
}
////////////////////////////////////////////////////////////////////////////////


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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
		SIZE si = YHD6_CalcTextWidth(hDC, YHD4_MSG_BUFF);
		ReleaseDC(hWnd, hDC);

		TCHAR szMsg[128];
		wsprintf(szMsg, _T("Width : %d, Height : %d pixels"), si.cx, si.cy);
		MessageBox(hWnd, szMsg, _T("YHD Message"), MB_OK | MB_ICONINFORMATION);
	}
	return 0;

	case WM_CREATE:
		wsprintf(YHD4_MSG_BUFF, C_YHD_MSG, g_hInstance);
		return TRUE;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		RECT        rc;
		HDC hDC = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rc);
		DrawText(hDC, YHD4_MSG_BUFF, (int)_tcslen(YHD4_MSG_BUFF),
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