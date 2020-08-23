#include <Windows.h>
#include <tchar.h>


FARPROC MyGetProcAddr(HINSTANCE hMod, PCSTR pszSymbol)
{
	PBYTE pImgBase = (PBYTE)hMod;
	if (!pImgBase)
		return NULL;

	PIMAGE_DOS_HEADER pdh = (PIMAGE_DOS_HEADER)pImgBase;
	if (pdh->e_magic != IMAGE_DOS_SIGNATURE)
		return NULL;
	PIMAGE_NT_HEADERS pnh = (PIMAGE_NT_HEADERS)(pImgBase + pdh->e_lfanew);
	if (pnh->Signature != IMAGE_NT_SIGNATURE)
		return NULL;

	PIMAGE_DATA_DIRECTORY pdd = &pnh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	PIMAGE_EXPORT_DIRECTORY ped = (PIMAGE_EXPORT_DIRECTORY)(pImgBase + pdd->VirtualAddress);

	DWORD	dwFuncIdx = 0;
	PDWORD	pFuncTbl = (PDWORD)(pImgBase + ped->AddressOfFunctions);
	PWORD	pOrdnTbl = (PWORD)(pImgBase + ped->AddressOfNameOrdinals);
	if ((DWORD_PTR)pszSymbol <= 0xFFFF)
	{
		WORD wOrdinal = (WORD)IMAGE_ORDINAL((DWORD_PTR)pszSymbol);
		wOrdinal -= (WORD)ped->Base;
		if (wOrdinal < ped->NumberOfFunctions)
			return (FARPROC)(pImgBase + pFuncTbl[wOrdinal]);
	}
	else
	{
		PDWORD pFuncNameTbl = (PDWORD)(pImgBase + ped->AddressOfNames);
		for (; dwFuncIdx < ped->NumberOfNames; dwFuncIdx++)
		{
			PCSTR pFuncName = (PCSTR)(pImgBase + pFuncNameTbl[dwFuncIdx]);
			if (strcmp(pszSymbol, pFuncName) == 0)
			{
				WORD wOrdinal = pOrdnTbl[dwFuncIdx];
				return (FARPROC)(pImgBase + pFuncTbl[wOrdinal]);
			}
		}
	}
	return NULL;
}

typedef void (WINAPI *PYHD3_DRAWTEXT)(HDC hDC, PCTSTR pszText, POINT pt);
typedef SIZE (WINAPI *PYHD6_CALCTEXT)(HDC hDC, PCTSTR pszText);


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
	static HMODULE			s_hDll = NULL;
	static PTSTR			s_Yhd4MsgBuff = NULL;
	static PYHD3_DRAWTEXT	s_Yhd3DrawText = NULL;
	static PYHD6_CALCTEXT	s_Yhd6CalcText = NULL;

	switch (uMsg)
	{
		case WM_CREATE:
		{
			//s_hDll = LoadLibrary(L"BasicDllDE3.dll");
			s_hDll = LoadLibrary(L"BasicDllMD3.dll");
			if (s_hDll == NULL)
			{
				MessageBox(NULL, 
					L"BasicDllDE3.dll 로딩에 실패했습니다.", L"에러", MB_OK);
				return 0;
			}

			//s_Yhd4MsgBuff = (PTSTR)MyGetProcAddr(s_hDll, "YHD4_MSG_BUFF");
			s_Yhd4MsgBuff = (PTSTR)MyGetProcAddr(s_hDll, MAKEINTRESOURCEA(4));
			if (s_Yhd4MsgBuff == NULL)
			{
				MessageBox(NULL,
					L"변수 YHD4_MSG_BUFF를 획득할 수 없습니다.", L"에러", MB_OK);
				return 0;
			}
			//s_Yhd3DrawText = (PYHD3_DRAWTEXT)MyGetProcAddr(s_hDll, "YHD3_DrawText");
			s_Yhd3DrawText = (PYHD3_DRAWTEXT)MyGetProcAddr(s_hDll, MAKEINTRESOURCEA(3));
			if (s_Yhd3DrawText == NULL)
			{
				MessageBox(NULL,
					L"함수 YHD3_DrawText를 획득할 수 없습니다.", L"에러", MB_OK);
				return 0;
			}
			//s_Yhd6CalcText = (PYHD6_CALCTEXT)MyGetProcAddr(s_hDll, "YHD6_CalcTextWidth");
			s_Yhd6CalcText = (PYHD6_CALCTEXT)MyGetProcAddr(s_hDll, MAKEINTRESOURCEA(6));
			if (s_Yhd6CalcText == NULL)
			{
				MessageBox(NULL,
					L"함수 YHD6_CalcTextWidth를 획득할 수 없습니다.", L"에러", MB_OK);
				return 0;
			}
			wsprintf(s_Yhd4MsgBuff, C_YHD_MSG, g_hInstance);
		}
		return TRUE;

		case WM_LBUTTONDOWN:
		{
			if (!s_Yhd3DrawText)
				return 0;

			TCHAR szOutMsg[128];
			HDC hDC = GetDC(hWnd);
			POINT pt;

			pt.x = LOWORD(lParam), pt.y = HIWORD(lParam);
			wsprintf(szOutMsg, _T("Mouse Position => X : %d, Y : %d"),
				pt.x, pt.y);
			s_Yhd3DrawText(hDC, szOutMsg, pt);
			ReleaseDC(hWnd, hDC);
		}
		return 0;

		case WM_RBUTTONDOWN:
		{
			if (!s_Yhd6CalcText)
				return 0;

			HDC hDC = GetDC(hWnd);
			SIZE si = s_Yhd6CalcText(hDC, s_Yhd4MsgBuff);
			ReleaseDC(hWnd, hDC);

			TCHAR szMsg[128];
			wsprintf(szMsg, _T("Width : %d, Height : %d pixels"), si.cx, si.cy);
			MessageBox(hWnd, szMsg, _T("YHD Message"), MB_OK | MB_ICONINFORMATION);
		}
		return 0;

		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			RECT        rc;
			HDC hDC = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rc);
			DrawText(hDC, s_Yhd4MsgBuff, (int)_tcslen(s_Yhd4MsgBuff),
					&rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
			EndPaint(hWnd, &ps);
		}
		return 0;

		case WM_DESTROY:
			if (s_hDll != NULL)
				FreeLibrary(s_hDll);
			PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
////////////////////////////////////////////////////////////////////////////////