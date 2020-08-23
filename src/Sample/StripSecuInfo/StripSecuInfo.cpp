#include "stdafx.h"
#include <windows.h>

int _tmain(int argc, TCHAR *argv[])
{
	if (argc != 2)
	{
		printf("Usage: CheckSumTest <filename>\n");
		return 0;
	}

	HANDLE	hImgFile = INVALID_HANDLE_VALUE;
	HANDLE	hImgMap = NULL;
	PBYTE	pImgView = NULL;
	DWORD	dwSecuOff = 0;

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

		DWORD dwOffset = 0;
		PIMAGE_DOS_HEADER pdh = PIMAGE_DOS_HEADER(pImgView);
		if (pdh->e_magic != IMAGE_DOS_SIGNATURE)
			throw ERROR_INVALID_PARAMETER;
		dwOffset = pdh->e_lfanew;

		PBYTE pIter = pImgView + dwOffset;
		DWORD dwSig = *PDWORD(pIter); pIter += sizeof(DWORD);
		if (dwSig != IMAGE_NT_SIGNATURE)
			throw ERROR_INVALID_PARAMETER;

		PIMAGE_FILE_HEADER pnh = (PIMAGE_FILE_HEADER)pIter;
		pIter += (sizeof(IMAGE_FILE_HEADER) + pnh->SizeOfOptionalHeader);
		pIter -= sizeof(IMAGE_DATA_DIRECTORY) * IMAGE_NUMBEROF_DIRECTORY_ENTRIES;

		PIMAGE_DATA_DIRECTORY pdd = (PIMAGE_DATA_DIRECTORY)pIter + IMAGE_DIRECTORY_ENTRY_SECURITY;
		if (pdd->VirtualAddress > 0)
		{
			dwSecuOff = pdd->VirtualAddress;
			pdd->VirtualAddress = pdd->Size = 0;

			PIMAGE_OPTIONAL_HEADER poh = (PIMAGE_OPTIONAL_HEADER)((PBYTE)pnh + sizeof(IMAGE_FILE_HEADER));
			poh->DllCharacteristics &= ~IMAGE_DLLCHARACTERISTICS_FORCE_INTEGRITY;
			poh->CheckSum = 0;
		}
	}
	catch (DWORD hr)
	{
		printf("Error occurred with code=0x%08X", hr);
	}

	if (pImgView != NULL) UnmapViewOfFile(pImgView);
	if (hImgMap != NULL) CloseHandle(hImgMap);

	if (hImgFile != INVALID_HANDLE_VALUE)
	{
		if (dwSecuOff > 0)
		{
			SetFilePointer(hImgFile, dwSecuOff, NULL, FILE_BEGIN);
			SetEndOfFile(hImgFile);
		}
		CloseHandle(hImgFile);
	}
	return 0;
}
