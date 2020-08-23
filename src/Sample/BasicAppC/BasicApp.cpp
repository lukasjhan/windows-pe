/*******************************************************************************
 * ��  �� : HelloWin.C
 * ��  �� : C ��ü
 * ��  �� : WinMain - Win32 Entry Point.
 *          WndProc - Windows Procedure.
 * ��  �� : ���� �������� �������� GUI ���ø����̼��� ����...
 * ----------------------------------------------------------------------------
 * ��  �� : Yi HoDong
 * ��  ¥ : 2001.11.06
 * ��  �� : 1.0
 * ȸ  �� : YHD Works Co. (http://www.yhdworks.com)
 ******************************************************************************/
#include <Windows.h>
#include <TChar.h>

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
TCHAR g_szAppID[] = _T("Basic App by YHD");

/*******************************************************************************
 * ��  �� : WIN32 ��Ʈ�� ����Ʈ �c��
 * ��  �� : ����
 * ��  �� : WinMain
 * ��  �� : HINSTANCE hInstance [IN] : �� ���μ����� �ν��Ͻ� �ڵ�(���۹���)
 *          HINSTANCE hPrevInst [IN] : ������ 0, ����(16��Ʈ���� ȣȯ�� ����)
 *          PSTR      szCmdLine [IN] : Ŀ�ǵ� ���� �з�����
 *          int       iCmdShow  [IN] : �����찡 �������� ����
 * ��  ȯ : int : 
 * ��  �� : WIN32 ��Ʈ�� ����Ʈ �c��
 *          �ܼ� ���α׷�(main)���� �ٸ��� GUI������ WinMain�� ��Ʈ�� ����Ʈ
 *          �Լ��� �ν��Ѵ�.
 * ----------------------------------------------------------------------------
 * ��  �� : Yi HoDong
 * ��  ¥ : 2001.08.06
 * ��  �� : 1.0
 * ȸ  �� : YHD Works Co. (http://www.yhdworks.com)
 ******************************************************************************/
int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                   PTSTR szCmdLine, int iCmdShow)
{
	////////////////////////////////////////////////////////////////////////////
	//�������� Ŭ���� ���
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
	//�������� ����
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
	//�޽��� ����
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
 * ��  �� : �������� ���ν��� �Լ�(�ݹ� �Լ�)
 * ��  �� : ����
 * ��  �� : WndProc
 * ��  �� : HWND   hWnd   [IN] : �޽����� ���� ������ �ڵ�
 *          UINT   uMsg   [IN] : �޽���
 *          WPARAM wParam [IN] : ����1
 *          LPARAM lParam [IN] : ����2
 * ��  ȯ : LRESULT : 
 * ��  �� : �������� ���ν���
 *          WINDOWCLASS�� lpfnWndProc �ʵ忡 �� �Լ��� ������ ����ؾ� �Ѵ�.
 *          �������� ������ �� �Լ� ������ ��Ʈ���Ѵ�.
 * ----------------------------------------------------------------------------
 * ��  �� : Yi HoDong
 * ��  ¥ : 2001.08.06
 * ��  �� : 1.0
 * ȸ  �� : YHD Works Co. (http://www.yhdworks.com)
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