#include "stdafx.h"
#include "resource.h"
#include "Windows.h"

#define YHD_SECT_SIZE	(65536 << 1)

void _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		printf("The argument of Text file required...\n"
			"Usage : UsrSecText2Bin TextFile\n");
		return;
	}

	WCHAR  szTarget[MAX_PATH];
	HANDLE hSrcFile = INVALID_HANDLE_VALUE;
	HANDLE hDstFile = INVALID_HANDLE_VALUE;

	wsprintf(szTarget, L"%s.exe", argv[1]);
	try
	{
		hSrcFile = CreateFile(argv[1], GENERIC_READ,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (hSrcFile == INVALID_HANDLE_VALUE)
			throw GetLastError();
		hDstFile = CreateFile(szTarget, GENERIC_WRITE,
			0, NULL, OPEN_ALWAYS, 0, NULL);
		if (hDstFile == INVALID_HANDLE_VALUE)
			throw GetLastError();

		BYTE	arTxtBuff[YHD_SECT_SIZE];
		DWORD	dwReadBytes = 0;
		ReadFile(hSrcFile, arTxtBuff, sizeof(arTxtBuff), &dwReadBytes, NULL);

		UINT	uResId	= (sizeof(DWORD_PTR) == 8) ? IDR_BIN_VIEW64 : IDR_BIN_VIEW32;
		HMODULE	hMod	= GetModuleHandle(NULL);
		HRSRC	hBinRes = FindResource(hMod, MAKEINTRESOURCE(uResId), L"BINARY");
		DWORD	dwSize	= SizeofResource(hMod, hBinRes);
		HGLOBAL hBinExe = LoadResource(GetModuleHandle(NULL), hBinRes);
		PBYTE	pBinExe = (PBYTE)LockResource(hBinExe);

		PIMAGE_DOS_HEADER pdh = (PIMAGE_DOS_HEADER)pBinExe;
		PIMAGE_NT_HEADERS pnh = (PIMAGE_NT_HEADERS)(pBinExe + pdh->e_lfanew);
		PBYTE pIter = pBinExe + pdh->e_lfanew + sizeof(IMAGE_NT_HEADERS);

		PIMAGE_SECTION_HEADER psh = NULL;
		for (int i = 0; i < pnh->FileHeader.NumberOfSections; i++)
		{
			PIMAGE_SECTION_HEADER pshe = (PIMAGE_SECTION_HEADER)pIter;
			if (!memcmp(pshe->Name, ".YHD", 4))
			{
				psh = pshe;
				break;
			}
			pIter += sizeof(IMAGE_SECTION_HEADER);
		}
		if (psh != NULL)
		{
			DWORD dwWroteBytes = 0;
			WriteFile(hDstFile, pBinExe, dwSize, &dwWroteBytes, NULL);
			SetFilePointer(hDstFile, 0, NULL, FILE_BEGIN);
			SetFilePointer(hDstFile, psh->PointerToRawData, NULL, FILE_BEGIN);
			WriteFile(hDstFile, arTxtBuff, dwReadBytes, &dwWroteBytes, NULL);
		}
		UnlockResource(hBinRes);
	}
	catch (DWORD dwErrCode)
	{
		printf("Converting to EXE failed with code=0x%08X\n", dwErrCode);
	}
	if (hDstFile != INVALID_HANDLE_VALUE)
		CloseHandle(hDstFile);
	if (hSrcFile != INVALID_HANDLE_VALUE)
		CloseHandle(hSrcFile);
}
