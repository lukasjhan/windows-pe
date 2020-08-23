#include "stdafx.h"
#include <windows.h>
#include <Imagehlp.h>

#pragma comment(lib, "Imagehlp.lib")


int _tmain(int argc, TCHAR *argv[])
{
	if (argc != 2)
	{
		printf("Usage: CheckSumTest <filename>\n");
		return 0;
	}

	bool	bIsNew	 = false;
	HANDLE	hImgFile = INVALID_HANDLE_VALUE;
	HANDLE	hImgMap	= NULL;
	PBYTE	pImgView = NULL;

	try
	{
		hImgFile = CreateFile(argv[1], GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
		if (hImgFile == INVALID_HANDLE_VALUE)
			throw GetLastError();

		hImgMap = CreateFileMapping(hImgFile, NULL,
			PAGE_READWRITE, 0, 0, NULL);
		if (hImgMap == NULL)
			throw GetLastError();

		pImgView = (PBYTE)MapViewOfFile(hImgMap, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		if (pImgView == NULL)
			throw GetLastError();

		DWORD dwSize = GetFileSize(hImgFile, NULL);
		DWORD dwOldChecksum = 0, dwNewChecksum = 0;
		PIMAGE_NT_HEADERS pnh = CheckSumMappedFile(pImgView, dwSize, &dwOldChecksum, &dwNewChecksum);
		if (pnh == NULL || dwOldChecksum == 0)
			throw GetLastError();

		if (pnh->OptionalHeader.CheckSum == 0)
		{
			printf("Calculated Checksum = 0x08X\n", dwNewChecksum);
			bIsNew = true;
		}
		else
		{
			printf("CheckSum : File = 0x%08X, Calculated = 0x%08X\n",
				dwOldChecksum, dwNewChecksum);
			if (dwOldChecksum != dwNewChecksum)
			{
				printf("  CheckSum is invalid, updates new Checksum...\n");
				bIsNew = true;
			}
			else
				printf("  CheckSum is valid.\n");
		}
	}
	catch (DWORD hr)
	{
		printf("Error occurred with code=0x%08X", hr);
	}

	if (pImgView != NULL) UnmapViewOfFile(pImgView);
	if (hImgMap != NULL) CloseHandle(hImgMap);
	if (hImgFile != INVALID_HANDLE_VALUE) CloseHandle(hImgFile);
	return 0;
}
