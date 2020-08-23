#include "stdafx.h"
#include <Windows.h>
#include "..\..\Solution\PEPlus\PEPlus.h"


#ifdef _WIN64
#	ifdef _DEBUG 
#		pragma comment(lib, "../../3.lib/x64/Debug/PEPlus.lib")
#	else
#		pragma comment(lib, "../../3.lib/x64/Release/PEPlus.lib")
#	endif
#else
#	ifdef _DEBUG 
#		pragma comment(lib, "../../3.lib/x86/Debug/PEPlus.lib")
#	else
#		pragma comment(lib, "../../3.lib/x86/Release/PEPlus.lib")
#	endif
#endif

void _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 3)
	{
		printf("The argument of PE file required...\n"
			"Usage : RebaseTest address pefile\n");
		return;
	}

	if (_wcsnicmp(argv[1], L"0x", 2) != 0)
	{
		printf("Address format requires 0x...\n");
		return;
	}
	
	PWSTR pEnd = NULL;
	DWORD64 llNewBase = (DWORD64)wcstoll(argv[1], &pEnd, 16);
	if (llNewBase == 0)
	{
		printf("Address format is invalid.\n");
		return;
	}

	HANDLE	hImgFile = INVALID_HANDLE_VALUE;
	HANDLE	hImgMap  = NULL;
	PBYTE	pImgView = NULL;
	try
	{
		hImgFile = CreateFile(argv[2], GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hImgFile == INVALID_HANDLE_VALUE)
			throw HRESULT_FROM_WIN32(GetLastError());

		hImgMap = CreateFileMapping(hImgFile, NULL,
			PAGE_READWRITE, 0, 0, NULL);
		if (hImgMap == NULL)
			throw HRESULT_FROM_WIN32(GetLastError());

		pImgView = (PBYTE)MapViewOfFile(hImgMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		if (pImgView == NULL)
			throw HRESULT_FROM_WIN32(GetLastError());

		int nRelocCnt = PEPlus::BaseRelocation(pImgView, llNewBase);
		printf("%d relocations applied...\n", nRelocCnt);
	}
	catch (HRESULT hr)
	{
		CString sz = PEPlus::GetErrMsg(hr);
		printf("Error: %S\n", sz);
	}

	if (pImgView != NULL)
		UnmapViewOfFile(pImgView);
	if (hImgMap != NULL)
		CloseHandle(hImgMap);
	if (hImgFile != INVALID_HANDLE_VALUE)
		CloseHandle(hImgFile);
}