#include "stdafx.h"
#include <Windows.h>
#include <stdlib.h>

#pragma comment(lib, "version.lib")

CString GetInfoFromVersion(PBYTE pVerData, DWORD dwLangCode, LPCTSTR pszWant)
{
	CString	szQuery;
	szQuery.Format(_T("\\StringFileInfo\\%08lx\\%s"), dwLangCode, pszWant);

	LPVOID	pVerStr = NULL;
	UINT	uInfoLen = 0;
	if (!VerQueryValue(pVerData, (LPTSTR)(LPCTSTR)szQuery, &pVerStr, &uInfoLen))
		return L"";
	if (uInfoLen == 0)
		return L"";

	TCHAR szDetails[1024];
	memcpy(szDetails, pVerStr, uInfoLen * sizeof(TCHAR));
	szDetails[uInfoLen] = 0;
	return (PCWSTR)szDetails;
}

void _tmain(int argc, TCHAR* argv[])
{
	if (argc < 2)
	{
		printf(	"The argument of PE file required...\n"
				"Usage : GetVerInfo PE_File\n");
		return;
	}

	PBYTE pVerData = NULL;
	try
	{
		DWORD dwVerSize = GetFileVersionInfoSize(argv[1], NULL);
		if (dwVerSize == 0)
			throw GetLastError();

		pVerData = new BYTE[dwVerSize + 1];
		if (!GetFileVersionInfo(argv[1], 0, dwVerSize, pVerData))
			throw GetLastError();

		UINT uInfoLen = 0;
		VS_FIXEDFILEINFO* pFvi = NULL;
		if (!VerQueryValue(pVerData, _T("\\"), (PVOID*)&pFvi, &uInfoLen))
			throw GetLastError();

		PVOID pInfo = NULL;
		if (!VerQueryValue(pVerData, _T("\\VarFileInfo\\Translation"),
			&pInfo, &uInfoLen) || uInfoLen == 0)
			throw GetLastError();

		WORD	wDefLangID = GetUserDefaultLangID();
		UINT	uLangCodeCnt = (uInfoLen >> 2);
		DWORD	dwLangCode = *((LPDWORD)pInfo);
		dwLangCode = MAKELONG(HIWORD(dwLangCode), LOWORD(dwLangCode));

		DWORD	dwFoundCode = 0xFFFFFFFF;
		for (UINT i = 0; i<uLangCodeCnt; i++)
		{
			DWORD dwTempCode = *(((LPDWORD)pInfo + i));
			if (LOWORD(dwTempCode) == wDefLangID)
			{
				dwFoundCode = MAKELONG(HIWORD(dwTempCode), LOWORD(dwTempCode));
				break;
			}
		}
		if (dwFoundCode == 0xFFFFFFFF)
			dwFoundCode = dwLangCode;

		PCWSTR pszFlds[] = 
		{
			L"FileDescription", L"FileVersion"     , L"InternalName", L"CompanyName", 
			L"LegalCopyright" , L"OriginalFilename", L"ProductName" , L"ProductVersion"
		};
		for (int i = 0; i < sizeof(pszFlds) / sizeof(PCWSTR); i++)
		{
			CString szResInfo = GetInfoFromVersion(pVerData, dwFoundCode, pszFlds[i]);
			if (dwFoundCode != dwLangCode)
				szResInfo = GetInfoFromVersion(pVerData, dwLangCode, pszFlds[i]);
			printf("%-16S: %S\n", pszFlds[i], szResInfo);
		}
	}
	catch (DWORD hr)
	{
		printf("Error occurred, code=0x08X\n", hr);
	}
	if (pVerData != NULL)
		delete[] pVerData;
}