#include <Windows.h>
#include <tchar.h>
#include <delayimp.h>

#ifndef BASICDLL_API
#	ifdef __cplusplus
#		define BASICDLL_API extern "C" __declspec(dllimport)
#	else
#		define BASICDLL_API __declspec(dllimport)
#	endif
#endif
BASICDLL_API void WINAPI YHD3_DrawText(HDC hDC, PCTSTR pszText, POINT pt);
BASICDLL_API SIZE WINAPI YHD6_CalcTextWidth(HDC hDC, PCTSTR pszText);
BASICDLL_API BOOL WINAPI YHD7_IsPointInRect(LPCRECT prc, POINT pt);

#ifdef _WIN64
#	ifdef _DEBUG 
#		pragma comment(lib, "../0.bin/x64/Debug/BasicDllDE3.lib")
#	else
#		pragma comment(lib, "../0.bin/x64/Release/BasicDllDE3.lib")
#	endif
#else
#	ifdef _DEBUG 
#		pragma comment(lib, "../0.bin/x86/Debug/BasicDllDE3.lib")
#	else
#		pragma comment(lib, "../0.bin/x86/Release/BasicDllDE3.lib")
#	endif
#endif
#pragma comment(lib, "DelayImp.lib")


TCHAR G_APP_CLS[16] = _T("YHD_HELLO_WND");
HINSTANCE g_hInstance;
#if _WIN64
PCTSTR C_YHD_MSG = _T("YHD's WinApp : HINSTANCE=0x%016I64X");
#else
PCTSTR C_YHD_MSG = _T("YHD's WinApp : HINSTANCE=0x%08X");
#endif

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LONG WINAPI DelayLoadExceptFileter(PEXCEPTION_POINTERS);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst,
	PTSTR pszCmdLine, int nCmdShow)
{
	g_hInstance = hInstance;

	////////////////////////////////////////////////////////////////////////////
	//윈도우즈 클래스 등록
	////////////////////////////////////////////////////////////////////////////
	WNDCLASSEX wc;
	wc.cbSize		 = sizeof(wc);
	wc.style		 = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInstance;
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName	 = NULL;
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
	static TCHAR s_szMag[128];
	switch (uMsg)
	{
	case WM_LBUTTONDOWN:
	{
		TCHAR szOutMsg[128];
		HDC hDC = GetDC(hWnd);
		POINT pt;

		pt.x = LOWORD(lParam), pt.y = HIWORD(lParam);
		wsprintf(szOutMsg, 
			_T("Mouse Position => X : %d, Y : %d"), pt.x, pt.y);
		__try
		{
			YHD3_DrawText(hDC, szOutMsg, pt);
		}
		__except (DelayLoadExceptFileter(GetExceptionInformation()))
		{
		}
		ReleaseDC(hWnd, hDC);
	}
	return 0;

	case WM_RBUTTONDOWN:
	{
		HDC hDC = GetDC(hWnd);
		SIZE si;
		__try
		{
			si = YHD6_CalcTextWidth(hDC, s_szMag);
		}
		__except (DelayLoadExceptFileter(GetExceptionInformation()))
		{
		}
		ReleaseDC(hWnd, hDC);

		TCHAR szMsg[128];
		wsprintf(szMsg, _T("Width : %d, Height : %d pixels"), si.cx, si.cy);
		MessageBox(hWnd, szMsg, _T("YHD Message"), MB_OK | MB_ICONINFORMATION);
	}
	return 0;

	case WM_LBUTTONDBLCLK:
	{
		__FUnloadDelayLoadedDLL2("BasicDllDE3.dll");
	}
	return 0;

	case WM_CREATE:
		wsprintf(s_szMag/*YHD4_MSG_BUFF*/, C_YHD_MSG, g_hInstance);
		return TRUE;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		RECT        rc;
		HDC hDC = BeginPaint(hWnd, &ps);
		GetClientRect(hWnd, &rc);
		DrawText(hDC, s_szMag, (int)_tcslen(s_szMag),
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


LONG WINAPI DelayLoadExceptFileter(PEXCEPTION_POINTERS pEP)
{
	LONG lFilter = EXCEPTION_EXECUTE_HANDLER;
	PDelayLoadInfo pDLI = (PDelayLoadInfo)
		pEP->ExceptionRecord->ExceptionInformation[0];
	TCHAR szOut[512];

	switch (pEP->ExceptionRecord->ExceptionCode)
	{
		case VcppException(ERROR_SEVERITY_ERROR, ERROR_MOD_NOT_FOUND):
			wsprintf(szOut, L"DLL %S을 찾을 수 없습니다.", pDLI->szDll);
		break;

		case VcppException(ERROR_SEVERITY_ERROR, ERROR_PROC_NOT_FOUND):
			if (pDLI->dlp.fImportByName)
				wsprintf(szOut, L"함수 %S를 DLL %S에서 찾을 수 없습니다.",
					pDLI->dlp.szProcName, pDLI->szDll);
			else
				wsprintf(szOut,
					L"함수의 서수 %d번을 DLL %S에서 찾을 수 없습니다.",
					pDLI->dlp.dwOrdinal, pDLI->szDll);
		break;

		default:
			lFilter = EXCEPTION_CONTINUE_SEARCH;
		break;
	}

	if (lFilter == EXCEPTION_EXECUTE_HANDLER)
		MessageBox(NULL, szOut, L"예외발생 테스트", MB_OK | MB_ICONERROR);

	return lFilter;
}

FARPROC WINAPI DelayLoadHook(unsigned dliNotify, PDelayLoadInfo pDLI)
{
	FARPROC fp = NULL;
	TCHAR szOut[512];
	lstrcpy(szOut, L"DelayLoad Hook 호출됨...(통지코드 : ");

	switch (dliNotify)
	{
	case dliStartProcessing:
		lstrcat(szOut, L"dliStartProcessing)");
		break;
	case dliNotePreLoadLibrary:
		fp = (FARPROC)(HMODULE)NULL;
		lstrcat(szOut, L"dliNotePreLoadLibrary)");
		break;
	case dliFailLoadLib:
		fp = (FARPROC)(HMODULE)NULL;
		lstrcat(szOut, L"dliFailLoadLib)");
		break;
	case dliNotePreGetProcAddress:
		fp = (FARPROC)NULL;
		lstrcat(szOut, L"dliNotePreGetProcAddress)");
		break;
	case dliFailGetProc:
		fp = (FARPROC)NULL;
		lstrcat(szOut, L"dliFailGetProc)");
		break;
	case dliNoteEndProcessing:
		lstrcat(szOut, L"dliNoteEndProcessing)");
		break;
	}
	MessageBox(NULL, szOut,
		L"DelayLoad 테스트", MB_OK | MB_ICONEXCLAMATION);
	return fp;
}

PfnDliHook __pfnDliFailureHook2 = DelayLoadHook;
PfnDliHook __pfnDliNotifyHook2 = DelayLoadHook;

