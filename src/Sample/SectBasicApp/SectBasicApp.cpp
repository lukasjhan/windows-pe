#include <windows.h>
#include <tchar.h>

#pragma section(".YHD1", read)	
__declspec(allocate(".YHD1")) TCHAR G_APP_CLS[16] = _T("YHD_HELLO_WND");

#pragma data_seg(".YHD2")
HINSTANCE g_hInstance = 0;
#pragma data_seg()

#pragma const_seg(".YHD3")
#if _WIN64
const TCHAR C_YHD_MSG[] = _T("YHD's WinApp : HINSTANCE=0x%016I64X");
#else
const TCHAR C_YHD_MSG[] = _T("YHD's WinApp : HINSTANCE=0x%08X");
#endif
#pragma const_seg()

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#pragma code_seg(".YHD4")
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst,
                     PTSTR pszCmdLine, int nCmdShow)
{
	g_hInstance = hInstance;

	////////////////////////////////////////////////////////////////////////////
	//윈도우즈 클래스 등록
	////////////////////////////////////////////////////////////////////////////
	WNDCLASSEX wc;
	wc.cbSize		 = sizeof(wc);
	wc.style		 = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.hIconSm		 = NULL;
	wc.lpszClassName = G_APP_CLS;
	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL, _T("RegisterClassEx FAILED!!!"),
			G_APP_CLS, MB_ICONERROR);
		return 0 ;
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
#pragma code_seg()

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static TCHAR s_szMsg[64];
	switch(uMsg)
	{
		case WM_CREATE:
			wsprintf(s_szMsg, C_YHD_MSG, g_hInstance);
		return TRUE;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			RECT        rc;
			HDC			hDC = BeginPaint(hWnd, &ps);

			GetClientRect(hWnd, &rc);
			int nStrLen = (int)_tcslen(s_szMsg);
			DrawText(hDC, s_szMsg, nStrLen, &rc, 
				DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);
			EndPaint(hWnd, &ps);
		}
		return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
