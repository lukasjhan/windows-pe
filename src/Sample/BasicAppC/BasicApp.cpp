/*******************************************************************************
 * 파  일 : HelloWin.C
 * 종  류 : C 본체
 * 정  의 : WinMain - Win32 Entry Point.
 *          WndProc - Windows Procedure.
 * 내  용 : 가장 원초적인 윈도우즈 GUI 어플리케이션의 정의...
 * ----------------------------------------------------------------------------
 * 저  자 : Yi HoDong
 * 날  짜 : 2001.11.06
 * 버  젼 : 1.0
 * 회  사 : YHD Works Co. (http://www.yhdworks.com)
 ******************************************************************************/
#include <Windows.h>
#include <TChar.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
TCHAR g_szAppID[] = _T("Basic App by YHD");

/*******************************************************************************
 * 종  류 : WIN32 엔트리 포인트 홤수
 * 소  속 : 없음
 * 이  름 : WinMain
 * 인  수 : HINSTANCE hInstance [IN] : 본 프로세스의 인스턴스 핸들(시작번지)
 *          HINSTANCE hPrevInst [IN] : 언제나 0, 무시(16비트와의 호환을 위해)
 *          PSTR      szCmdLine [IN] : 커맨드 라인 패러미터
 *          int       iCmdShow  [IN] : 윈도우가 보여지는 형태
 * 반  환 : int : 
 * 내  용 : WIN32 엔트리 포인트 홤수
 *          콘솔 프로그램(main)과는 다르게 GUI에서는 WinMain을 엔트리 포인트
 *          함수로 인식한다.
 * ----------------------------------------------------------------------------
 * 저  자 : Yi HoDong
 * 날  짜 : 2001.08.06
 * 버  젼 : 1.0
 * 회  사 : YHD Works Co. (http://www.yhdworks.com)
 ******************************************************************************/
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                   PTSTR szCmdLine, int iCmdShow)
{
	////////////////////////////////////////////////////////////////////////////
	//윈도우즈 클래스 등록
	////////////////////////////////////////////////////////////////////////////
	WNDCLASSEX wc;
	wc.cbSize		 = sizeof(wc);
	wc.style         = CS_HREDRAW|CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance	 = hInst;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = g_szAppID;
	wc.hIconSm		 = NULL;
	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL, _T("Calling RegisterClassEx failed!"),
			g_szAppID, MB_ICONERROR);
		return 0 ;
	}
	////////////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////////////
	//윈도우즈 생성
	////////////////////////////////////////////////////////////////////////////
	HWND hWnd = CreateWindowEx
	(
		0,						// Extended Windows Styles
		g_szAppID,				// window class name
		g_szAppID,				// window caption
		WS_OVERLAPPEDWINDOW,	// window style
		CW_USEDEFAULT,			// initial x position
		CW_USEDEFAULT,			// initial y position
		400,					// initial x size
		150,					// initial y size
		NULL,					// parent window handle
		NULL,					// window menu handle
		hInst,					// program instance handle
		NULL					// creation parameters
	);
	if (hWnd == NULL)
	{
		MessageBox(NULL, _T("Calling CreateWindowEx failed!"),
			g_szAppID, MB_ICONERROR);
		return 0;
	}
	ShowWindow(hWnd, iCmdShow);
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
	switch(uMsg)
	{
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