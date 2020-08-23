#include <Windows.h>
#include <tchar.h>
#include "../ClassDll/ClassDll.h" 

#ifdef _WIN64
#	ifdef _DEBUG 
#		pragma comment(lib, "../0.bin/x64/Debug/ClassDll.lib")
#	else
#		pragma comment(lib, "../0.bin/x64/Release/ClassDll.lib")
#	endif
#else
#	ifdef _DEBUG 
#		pragma comment(lib, "../0.bin/x86/Debug/ClassDll.lib")
#	else
#		pragma comment(lib, "../0.bin/x86/Release/ClassDll.lib")
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


LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static TCHAR szMsg[] = _T("The Most Simple Windows GUI Program by YHD.");
	switch (uMsg)
	{
		case WM_LBUTTONDOWN:
		{
			POINT pt;
			pt.x = LOWORD(lParam), pt.y = HIWORD(lParam);
			TCHAR szOutMsg[128];
			wsprintf(szOutMsg, _T("Mouse Position => X : %d, Y : %d"),
				pt.x, pt.y);

			YhdDC dc(hWnd);
			dc.YHD3_DrawText(szOutMsg, pt);
		}
		return 0;

		case WM_RBUTTONDOWN:
		{
			YhdDC dc(hWnd);
			SIZE si = dc.YHD6_CalcTextWidth(szMsg);

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