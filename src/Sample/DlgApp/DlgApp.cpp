#include <windows.h>
#include <tchar.h>
#include "resource.h"

#if _WIN64
PCTSTR C_YHD_MSG = _T("YHD's WinApp : HINSTANCE=0x%016I64X");
#else
PCTSTR C_YHD_MSG = _T("YHD's WinApp : HINSTANCE=0x%08X");
#endif

LRESULT CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInst,
                     PTSTR pszCmdLine, int nCmdShow)
{
	DialogBox
	(
		hInstance,
		MAKEINTRESOURCE(IDD_DLG_MAIN),
		NULL,
		(DLGPROC)DlgProc
	);

	return 0;
}

LRESULT CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			TCHAR szMsg[64];
			wsprintf(szMsg, C_YHD_MSG, GetModuleHandle(NULL));
			SetDlgItemText(hDlg, IDC_EDT_SAMPLE, szMsg);
		}
		return TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK)
			{
				EndDialog(hDlg, IDOK);
				return TRUE;
			}
		break;
	}

	return FALSE;
}
