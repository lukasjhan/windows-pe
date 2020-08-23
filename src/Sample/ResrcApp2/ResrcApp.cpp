#include <windows.h>
#include <tchar.h>
#include "Resource.h"

#define		YHD_WND_TITLE	_T("YHD Hello Program")
TCHAR		G_APP_CLS[16] = _T("YHD_HELLO_WND");
HINSTANCE	g_hInstance;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                     PTSTR szCmdLine, int iCmdShow)
{
	g_hInstance = hInst;

	WNDCLASSEX wc;
	memset(&wc, 0, sizeof(wc));
	wc.cbSize		 = sizeof(wc);
	wc.style		 = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = WndProc;
	wc.hInstance	 = hInst;
	wc.hIcon		 = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDR_YHD_ICON));
	wc.hCursor       = LoadCursor(g_hInstance, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName	 = MAKEINTRESOURCE(IDR_YHD_MENU);
	wc.lpszClassName = G_APP_CLS;
	if(!RegisterClassEx(&wc))
	{
		TCHAR szMsg[64];
		LoadString(hInst, IDS_FAIL_REGISTER_CLASS, szMsg, sizeof(szMsg));
		MessageBox(NULL, szMsg, G_APP_CLS, MB_ICONERROR);
		return 0 ;
	}

	HWND hWnd = CreateWindowEx
	(
		0, G_APP_CLS, YHD_WND_TITLE,
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		400, 150, NULL, NULL, hInst, NULL
	);
	if (hWnd == NULL)
	{
		TCHAR szMsg[64];
		LoadString(hInst, IDS_FAIL_CREATE_WINDOW, szMsg, sizeof(szMsg));
		MessageBox(NULL, szMsg, G_APP_CLS, MB_ICONERROR);
		return 0;
	}
	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);

	HACCEL hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_YHD_ACCEL));
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(hWnd, hAccel, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static TCHAR	s_szMsg[64];
	static HBITMAP	s_hbmBkgnd;

	switch (uMsg)
	{
		case WM_CREATE:
		{
			TCHAR szMsg[64];
#if _WIN64
			LoadString(g_hInstance, IDS_TEXT_MSG64, szMsg, sizeof(szMsg));
#else
			LoadString(g_hInstance, IDS_TEXT_MSG32, szMsg, sizeof(szMsg));
#endif
			wsprintf(s_szMsg, szMsg, g_hInstance);
		}
		return TRUE;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			RECT        rc;
			HDC			hDC = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rc);

			HBRUSH hbr = NULL, hOldBr = NULL;
			INT nOldBkMode = 0;
			if (s_hbmBkgnd != NULL)
			{
				hbr = CreatePatternBrush(s_hbmBkgnd);
				hOldBr = (HBRUSH)SelectObject(hDC, hbr);
				PatBlt(hDC, 0, 0, rc.right, rc.bottom, PATCOPY);
				nOldBkMode = SetBkMode(hDC, TRANSPARENT);
			}
			else
			{
				hbr = CreateSolidBrush(RGB(255, 255, 255));
				FillRect(hDC, &rc, hbr);
			}

			int nStrLen = (int)_tcslen(s_szMsg);
			DrawText(hDC, s_szMsg, nStrLen, &rc, 
				DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS);
			if (s_hbmBkgnd != NULL)
			{
				SetBkMode(hDC, nOldBkMode);
				SelectObject(hDC, hOldBr);
				DeleteObject(hbr);
			}
			EndPaint(hWnd, &ps);
		}
		return 0;

		case WM_DESTROY:
			if (s_hbmBkgnd != NULL)
			{
				DeleteObject(s_hbmBkgnd);
				s_hbmBkgnd = NULL;
			}
			PostQuitMessage(0);
		return 0;


		case WM_COMMAND:
		{
			WORD wID = LOWORD(wParam);
			switch (wID)
			{
				case ID_MENU_CHANGECURSOR:
				case ID_ACCEL_CHANGECURSOR:
				{
					HMENU hSubMenu = GetSubMenu(GetMenu(hWnd), 0);
					UINT uFlag = GetMenuState(hSubMenu, 0, MF_BYPOSITION);
					LONG_PTR uIcon = 0;
					if (uFlag & MF_CHECKED)
					{
						uFlag = MF_BYPOSITION | MF_UNCHECKED;
						uIcon = (LONG_PTR)LoadCursor(NULL, IDC_ARROW);
					}
					else
					{
						uFlag = MF_BYPOSITION | MF_CHECKED;
						uIcon = (LONG_PTR)LoadCursor(g_hInstance, MAKEINTRESOURCE(IDR_YHD_CURSOR));
					}
					SetClassLongPtr(hWnd, GCLP_HCURSOR, uIcon);
					CheckMenuItem(hSubMenu, 0, uFlag);
				}
				break;

				case ID_MENU_SHOWBITMAP:
				case ID_ACCEL_SHOWBITMAP:
				{
					if (s_hbmBkgnd == NULL)
					{
						HMENU hmnu = GetSubMenu(GetSubMenu(GetMenu(hWnd), 0), 2);
						EnableMenuItem(hmnu, 0, MF_BYPOSITION | MF_GRAYED );
						EnableMenuItem(hmnu, 1, MF_BYPOSITION | MF_ENABLED);
						s_hbmBkgnd = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDR_YHD_BITMAP));
						InvalidateRect(hWnd, NULL, TRUE);
					}
				}
				break;

				case ID_MENU_CLEARBITMAP:
				case ID_ACCEL_CLEARBITMAP:
				{
					if (s_hbmBkgnd != NULL)
					{
						HMENU hmnu = GetSubMenu(GetSubMenu(GetMenu(hWnd), 0), 2);
						EnableMenuItem(hmnu, 0, MF_BYPOSITION | MF_ENABLED);
						EnableMenuItem(hmnu, 1, MF_BYPOSITION | MF_GRAYED);
						DeleteObject(s_hbmBkgnd);
						s_hbmBkgnd = NULL;
						InvalidateRect(hWnd, NULL, TRUE);
					}
				}
				break;

				case ID_MENU_EXIT:
				case ID_ACCEL_EXIT:
				{
					TCHAR szMsg[64];
					LoadString(g_hInstance, IDS_QUESTION, szMsg, sizeof(szMsg));
					if (MessageBox(hWnd, szMsg, YHD_WND_TITLE,
						MB_YESNO | MB_ICONQUESTION) == IDYES)
						DestroyWindow(hWnd);
				}
				break;
			}
		}
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
