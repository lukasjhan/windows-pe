#include "stdafx.h"
#include <Windows.h>


PCTSTR G_PSZ_OPTS[] = { L"-cursor", L"-bitmap", L"-icon", L"-all" };
#define RT_ALL	(RT_MANIFEST + 1)	

#pragma pack(1)
typedef struct
{
	WORD	wReserved;		// Reserved
	WORD	wType;			// resource type (1 for icons)
	WORD	wCount;			// how many images?
} ICONDIR, *LPICONDIR;

typedef struct
{
	BYTE	 bWidth;			// Width of the image
	BYTE	 bHeight;			// Height of the image (times 2)
	BYTE	 bColorCount;		// Number of colors in image (0 if >=8bpp)
	BYTE	 bReserved;			// Reserved
	WORD	 wPlanes;			// Color Planes
	WORD	 wBitCount;			// Bits per pixel
	DWORD	 dwBytesInRes;		// how many bytes in this resource?
	DWORD	 dwImageOffset;		// where in the file is this image
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
	BYTE	bWidth;				// Width of the image
	BYTE	bHeight;			// Height of the image
	BYTE	bColorCount;		// Number of colors in image (0 if >=8bpp)
	BYTE	bReserved;			// Reserved
	WORD	wPlanes;			// Color Planes
	WORD	wBitCount;			// Bits per pixel
	DWORD	dwBytesInRes;		// how many bytes in this resource?
	WORD	wID;				// the ID
} RES_ICONDIRENTRY, *LPRES_ICONDIRENTRY;

typedef struct
{
	BYTE	 bWidth;			// Width of the image
	BYTE	 bHeight;			// Height of the image (times 2)
	BYTE	 bColorCount;		// Number of colors in image (0 if >=8bpp)
	BYTE	 bReserved;			// Reserved
	WORD	 wHotspotX;			// Hotspot X(Icon의 경우 Color Planes)
	WORD	 wHotspotY;			// Hotspot Y(Icon의 경우 Bits per pixel)
	DWORD	 dwBytesInRes;		// how many bytes in this resource?
	DWORD	 dwImageOffset;		// where in the file is this image
} CURSORDIRENTRY, *LPCURSORDIRENTRY;

typedef struct
{
	WORD	wWidth;			// Width of the image
	WORD	wHeight;		// Height of the image(times 2)
	WORD	wPlanes;		// Color Planes
	WORD	wBitCount;		// Bits per pixel
	DWORD	dwBytesInRes;	// how many bytes in this resource?
	WORD	wID;			// the ID
} RES_CURSORDIRENTRY, *LPRES_CURSORDIRENTRY;
#pragma pack()


void PrintUsage()
{
	printf("Image Extractor by YHD -> Usage:\n");
	printf("ImgExtract [-option] PEFilePath.\n");
	printf("  [-all]  : All image extracted.\n");
	printf("  -icon   : Cursor image extracted.\n");
	printf("  -cursor : Cursor image extracted.\n");
	printf("  -bitmap : Bitmap image extracted.\n");
}

PCTSTR ParseParams(int argc, _TCHAR* argv[])
{
	if (argc < 2)
		return NULL;
	if (argc == 2)
		return RT_ALL;

	int nOptCnt = sizeof(G_PSZ_OPTS) / sizeof(PCTSTR);
	for (int i = 0; i < nOptCnt; i++)
	{
		if (_tcsicmp(argv[1], G_PSZ_OPTS[i]) == 0)
		{
			if (i < nOptCnt - 1)
				return MAKEINTRESOURCE((WORD)(i + 1));
			return RT_ALL;
		}
	}
	return NULL;
}

void SaveBitmapFile(HANDLE hFile, PBYTE pData, DWORD dwSize)
{
	BITMAPFILEHEADER bfh;
	memset(&bfh, 0, sizeof(bfh));
	bfh.bfType = 0x4D42;
	bfh.bfSize = dwSize;
	bfh.bfOffBits = sizeof(bfh) + sizeof(BITMAPINFOHEADER);

	PBITMAPINFOHEADER pbih = (PBITMAPINFOHEADER)pData;
	if (pbih->biBitCount == 8)
		bfh.bfOffBits += 256 * sizeof(COLORREF);
	else if (pbih->biBitCount == 4)
		bfh.bfOffBits += 16 * sizeof(COLORREF);
	else if (pbih->biBitCount == 1)
		bfh.bfOffBits += 2 * sizeof(COLORREF);

	DWORD dwWrote = 0;
	if (!WriteFile(hFile, &bfh, sizeof(bfh), &dwWrote, NULL))
		throw HRESULT_FROM_WIN32(GetLastError());
	if (!WriteFile(hFile, pData, dwSize, &dwWrote, NULL))
		throw HRESULT_FROM_WIN32(GetLastError());
}

void SaveCurOrIcoFile(HANDLE hFile, PBYTE pData, DWORD dwSize, PBYTE pImgBase)
{
	PBYTE pIter = pData;
	LPICONDIR pDir = (LPICONDIR)pIter;
	pIter += sizeof(ICONDIR);

	DWORD dwImgOffset = sizeof(ICONDIR) + pDir->wCount * sizeof(ICONDIRENTRY);
	DWORD dwWrote = 0;
	if (!WriteFile(hFile, pDir, sizeof(ICONDIR), &dwWrote, NULL))
		throw HRESULT_FROM_WIN32(GetLastError());

	DWORD dwCurPos = sizeof(ICONDIR);
	DWORD dwPreIcoSize = 0;
	for (WORD i = 0; i < pDir->wCount; i++)
	{
		ICONDIRENTRY ide;
		WORD wId = 0;
		if (pDir->wType == 1)
		{
			LPRES_ICONDIRENTRY pie = (LPRES_ICONDIRENTRY)pIter;
			pIter += sizeof(RES_ICONDIRENTRY);
			wId = pie->wID;
			memcpy(&ide, pie, sizeof(RES_ICONDIRENTRY));
		}
		else
		{
			LPRES_CURSORDIRENTRY pie = (LPRES_CURSORDIRENTRY)pIter;
			pIter += sizeof(RES_CURSORDIRENTRY);
			wId = pie->wID;
			memset(&ide, 0, sizeof(ICONDIRENTRY));
			ide.bWidth  = (BYTE)pie->wWidth;
			ide.bHeight = (BYTE)(pie->wHeight >> 1);
			ide.dwBytesInRes = pie->dwBytesInRes - sizeof(DWORD);
		}
		ide.dwImageOffset = dwImgOffset + dwPreIcoSize;

		DWORD dwIcoSize = 0;
		PBYTE pImg = PEPlus::FindPEResource(dwIcoSize, pImgBase, 
			(PCWSTR)wId, (pDir->wType == 1) ? RT_ICON : RT_CURSOR);
		if (pImg == NULL)
			throw E_INVALIDARG;

		if (pDir->wType != 1)
		{
			ide.wPlanes   = *((PWORD)pImg); pImg += sizeof(WORD);
			ide.wBitCount = *((PWORD)pImg); pImg += sizeof(WORD);
			dwIcoSize -= sizeof(DWORD);
		}
		if (!WriteFile(hFile, &ide, sizeof(ICONDIRENTRY), &dwWrote, NULL))
			throw HRESULT_FROM_WIN32(GetLastError());
		dwCurPos += sizeof(ICONDIRENTRY);

		SetFilePointer(hFile, ide.dwImageOffset, NULL, FILE_BEGIN);
		if (!WriteFile(hFile, pImg, dwIcoSize, &dwWrote, NULL))
			throw HRESULT_FROM_WIN32(GetLastError());
		SetFilePointer(hFile, dwCurPos, NULL, FILE_BEGIN);
		dwPreIcoSize += dwIcoSize;
	}
}

int SaveSubResoures(PBYTE pImgBase, PIMAGE_SECTION_HEADER psh, PBYTE pResSect, 
	PIMAGE_RESOURCE_DIRECTORY_ENTRY prde, PCWSTR pszName, PCWSTR pszType, bool bScanSub)
{
	PBYTE pIter = pResSect + prde->OffsetToDirectory;
	PIMAGE_RESOURCE_DIRECTORY prd = (PIMAGE_RESOURCE_DIRECTORY)pIter;
	pIter += sizeof(IMAGE_RESOURCE_DIRECTORY);

	int nSavedCnt = 0;
	WORD wEntCnt = prd->NumberOfIdEntries + prd->NumberOfNamedEntries;
	for (WORD i = 0; i < wEntCnt; i++)
	{
		PIMAGE_RESOURCE_DIRECTORY_ENTRY prdet = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)pIter;
		pIter += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);

		CString szName = pszName;
		if (prdet->NameIsString)
		{
			PIMAGE_RESOURCE_DIR_STRING_U prds = 
				(PIMAGE_RESOURCE_DIR_STRING_U)(pResSect + prdet->NameOffset);
			CString sz;
			PWSTR pv = sz.GetBufferSetLength(prds->Length);
			memcpy(pv, prds->NameString, prds->Length * sizeof(WCHAR));
			sz.ReleaseBuffer();
			szName.AppendFormat(L".%s", sz);
		}
		else
		{
			szName.AppendFormat(L".%04X", prdet->Id);
		}

		if (bScanSub)
			nSavedCnt += SaveSubResoures(pImgBase, psh, pResSect, prdet, szName, pszType, false);
		else
		{
			static PCWSTR arszExts[] = { L".cur", L".bmp", L".ico" };

			WORD wId = (WORD)pszType;
			if (wId != (WORD)RT_BITMAP)
				wId -= DIFFERENCE;
			szName.Append(arszExts[wId - 1]);

			CString szInfo = szName;
			int nPos = szName.ReverseFind(L'\\');
			if (nPos >= 0)
				szInfo = szName.Mid(nPos + 1);

			PIMAGE_RESOURCE_DATA_ENTRY pdata = 
				PIMAGE_RESOURCE_DATA_ENTRY(pResSect + prdet->OffsetToDirectory);
			DWORD dwSize = pdata->Size;
			PBYTE pData = (pImgBase + RVA_TO_OFFSET(psh, pdata->OffsetToData));

			HANDLE hImgFile = INVALID_HANDLE_VALUE;
			try
			{
				hImgFile = CreateFile(szName, GENERIC_ALL,
					FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, 0, NULL);
				if (hImgFile == INVALID_HANDLE_VALUE)
					throw MAKE_HRESULT(1, FACILITY_WIN32, GetLastError());

				if (pszType == RT_BITMAP)
					SaveBitmapFile(hImgFile, pData, dwSize);
				else
					SaveCurOrIcoFile(hImgFile, pData, dwSize, pImgBase);
				nSavedCnt++;
				printf(" => %S extraction SUCCEEDED!!!\n", szInfo);
			}
			catch (HRESULT hr)
			{
				printf(" => %S extraction FAILED, Code=0x%08X\n", szInfo, hr);
			}
			if (hImgFile != INVALID_HANDLE_VALUE)
				CloseHandle(hImgFile);
		}
	}
	return nSavedCnt;
}

int ScanImgResources(PBYTE pImgBase, PCWSTR pszType, PCWSTR pszPath)
{
	if (pszType == RT_ICON || pszType == RT_CURSOR)
		pszType = (PCWSTR)((ULONG_PTR)pszType + DIFFERENCE);

	PIMAGE_DATA_DIRECTORY pdd = PEPlus::GetDataDir(pImgBase, IMAGE_DIRECTORY_ENTRY_RESOURCE);
	if (pdd->VirtualAddress == 0)
		return 0;
	PIMAGE_SECTION_HEADER psh = PEPlus::FindSectHdr(pImgBase, pdd->VirtualAddress);
	if (psh == NULL)
		return 0;

	PBYTE pResSect = pImgBase + RVA_TO_OFFSET(psh, pdd->VirtualAddress);
	PBYTE pIter = pResSect;
	PIMAGE_RESOURCE_DIRECTORY prd = (PIMAGE_RESOURCE_DIRECTORY)pIter;
	pIter += sizeof(IMAGE_RESOURCE_DIRECTORY);

	WORD wEntCnt = prd->NumberOfIdEntries;
	pIter += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY) * prd->NumberOfNamedEntries;

	int nSavedCnt = 0;
	for (WORD i = 0; i < wEntCnt; i++)
	{
		PIMAGE_RESOURCE_DIRECTORY_ENTRY prde = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)pIter;
		pIter += sizeof(IMAGE_RESOURCE_DIRECTORY_ENTRY);

		if (pszType != RT_ALL)
		{
			if ((WORD)pszType == prde->Id)
			{
				nSavedCnt += SaveSubResoures(pImgBase, psh, pResSect, prde, pszPath, pszType, true);
				break;
			}
		}
		else
		{
			if (prde->Id == (WORD)RT_BITMAP ||
				prde->Id == (WORD)RT_GROUP_CURSOR || prde->Id == (WORD)RT_GROUP_ICON)
				nSavedCnt += SaveSubResoures(pImgBase, psh, pResSect, prde, pszPath, (PCWSTR)prde->Id, true);
		}
	}
	return nSavedCnt;
}

void _tmain(int argc, _TCHAR* argv[])
{
	PCTSTR pszType = ParseParams(argc, argv);
	if (pszType == NULL)
	{
		PrintUsage();
		return;
	}

	HANDLE hImgFile = INVALID_HANDLE_VALUE;
	HANDLE hImgMap  = NULL;
	PBYTE pImgView  = NULL;
	try
	{
		PBYTE pCode = NULL;
		DWORD dwOffset = 0;
		int nBuffSize = 0;

		hImgFile = CreateFile
		(
			argv[argc - 1], GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL
		);
		if (hImgFile == INVALID_HANDLE_VALUE)
			throw MAKE_HRESULT(1, FACILITY_WIN32, GetLastError());

		hImgMap = CreateFileMapping
		(
			hImgFile, NULL, PAGE_READONLY, 0, 0, NULL
		);
		if (hImgMap == NULL)
			throw MAKE_HRESULT(1, FACILITY_WIN32, GetLastError());

		pImgView = (PBYTE)MapViewOfFile(hImgMap, FILE_MAP_READ, 0, 0, 0);
		if (pImgView == NULL)
			throw MAKE_HRESULT(1, FACILITY_WIN32, GetLastError());

		PIMAGE_DOS_HEADER pdh = PIMAGE_DOS_HEADER(pImgView);
		if (pdh->e_magic != IMAGE_DOS_SIGNATURE)
			throw E_INVALIDARG;

		dwOffset = pdh->e_lfanew;
		PIMAGE_NT_HEADERS pnh = PIMAGE_NT_HEADERS(pImgView + dwOffset);
		if (pnh->Signature != IMAGE_NT_SIGNATURE)
			throw E_INVALIDARG;

		CString szPath = argv[argc - 1];
		int nPos = szPath.ReverseFind(L'.');
		if (nPos >= 0)
			szPath = szPath.Left(nPos);
		int nSavedCnt = ScanImgResources(pImgView, pszType, szPath);
		printf("%d files are saved.\n", nSavedCnt);
	}
	catch (HRESULT e)
	{
		if (e == E_INVALIDARG)
			printf("Invalid PE File Format.\n", e);
		else
			printf("Fail to load PE File, Code = 0x%08X\n", e);
	}

	if (pImgView != NULL)
		UnmapViewOfFile(pImgView);
	if (hImgMap != NULL)
		CloseHandle(hImgMap);
	if (hImgFile != INVALID_HANDLE_VALUE)
		CloseHandle(hImgFile);
}
