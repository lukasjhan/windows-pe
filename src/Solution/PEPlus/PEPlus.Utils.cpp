#include "stdafx.h"
#include "PEPlus.h"
#include "Shlwapi.h"


String PEPlus::GetErrMsg(HRESULT hr)
{
	PVOID pMsgBuf = NULL;
	BOOL bIsOK = FormatMessage
	(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(PTSTR)&pMsgBuf, 0, NULL
	);
	if (!bIsOK)
	{
		HMODULE hMod = GetModuleHandle(_T("ntdll"));
		if (hMod != NULL)
		{
			bIsOK = FormatMessage
			(
				FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_HMODULE |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				hMod, hr,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(PTSTR)&pMsgBuf, 0, NULL
			);
		}
	}

	String szMsg;
	if (bIsOK)
	{
		szMsg.Format(_T("0x%08X: %s"), hr, pMsgBuf);
		szMsg.Replace(_T("\xd\xa"), _T(" "));
		LocalFree(pMsgBuf);
	}
	else
		szMsg.Format(_T("0x%08X: 알 수 없는 에러 코드입니다."), hr);
	return szMsg;
}


String PEPlus::GetAddrForm(bool bIs32, PBYTE pAddr)
{
	String sz;
	if (bIs32)
		sz.Format(L"%08X", pAddr);
	else
	{
		sz.Format(L"%016I64X", pAddr);
		sz.Insert(8, L'`');
	}
	return sz;
}

PBYTE PEPlus::AddrStr2Value(bool bIs32, PCWSTR pszAddr)
{
	int nMaxLen = (bIs32) ? 4 : 8;
	CString sz = pszAddr;

	if (sz.GetLength() > 2 && _tcsnicmp(sz, _T("0x"), 2) == 0)
		sz = sz.Mid(2);
	int nLen = sz.GetLength();
	if (nLen > nMaxLen * 2)
	{
		nLen = nMaxLen * 2;
		sz = sz.Left(nLen);
	}
	else if (nLen < nMaxLen * 2 && (nLen & 1) != 0)
	{
		sz.Insert(0, _T('0'));
		nLen++;
	}

	BYTE arCode[8];
	int nValSize = 0;
	BYTE code = 0;
	bool bInit = false;
	for (int i = 0; i < nLen; i++)
	{
		TCHAR bt = sz.GetAt(i);
		if (bt == 0x20)
			continue;
		if (bt >= 0x30 && bt <= 0x39)
			bt -= 0x30;
		else if (bt >= L'a' && bt <= L'f')
			bt -= (L'a' - 0x0a);
		else if (bt >= L'A' && bt <= L'F')
			bt -= (L'A' - 0x0A);
		else
			throw E_INVALIDARG;

		if (!bInit)
		{
			code = (BYTE)(bt << 4);
			bInit = true;
		}
		else
		{
			code |= (BYTE)bt;
			arCode[nValSize++] = code;
			code = 0;
			bInit = false;
		}
	}

	int nPos = nMaxLen - nValSize;
	memmove(arCode + nPos, arCode, nValSize);
	memset(arCode, 0, nPos);
	for (int i = 0; i < (nMaxLen >> 1); i++)
	{
		code = arCode[i];
		arCode[i] = arCode[nMaxLen - 1 - i];
		arCode[nMaxLen - 1 - i] = code;
	}

	PBYTE pAddr = NULL;
	if (nMaxLen == 4)
		pAddr = (PBYTE)(*((PDWORD)arCode));
	else
		pAddr = (PBYTE)(*((PDWORD64)arCode));
	return pAddr;
}

String PEPlus::Size2Units(UINT64 llSize)
{
	static PCWSTR SIZE_UNITS[] = { L"B", L"KB", L"MB", L"GB", L"TB", L"PB", L"EB", L"ZB", L"YB" };

	int i = 0;
	bool bNoRem = false;
	double srcSize = (double)llSize;
	while (true)
	{
		double preSize = srcSize;
		preSize /= 1024.0;
		if (preSize < 1.0)
		{
			double remain = srcSize - (double)(int)srcSize;
			bNoRem = (remain < 0.001);
			break;
		}
		srcSize = preSize;
		i++;
	}
	//srcSize *= (1000.0 / 1024.0);
	String sz;
	if (bNoRem)
		sz.Format(L"%d%s", (int)srcSize, SIZE_UNITS[i]);
	else
		sz.Format(L"%.3f%s", srcSize, SIZE_UNITS[i]);
	return sz;
}

String PEPlus::Val2CommaStr(UINT64 llSize)
{
	String sz;
	while (true)
	{
		UINT64 llr = llSize % 1000;
		llSize /= 1000;
		CString szr;
		if (llSize == 0)
		{
			szr.Format(L"%d", llr);
			sz.Insert(0, szr);
			break;
		}
		szr.Format(L",%03d", llr);
		sz.Insert(0, szr);
	}
	return sz;
}

String PEPlus::Int2TimeStr(DWORD dwVal)
{
	SYSTEMTIME st; FILETIME ft; ULARGE_INTEGER li;
	::memset(&st, 0x00, sizeof(st));
	st.wYear = 1970, st.wMonth = 1, st.wDay = 1, st.wHour = 9;
	::SystemTimeToFileTime(&st, &ft);
	li.HighPart = ft.dwHighDateTime;
	li.LowPart = ft.dwLowDateTime;
	li.QuadPart += ((LONGLONG)dwVal * 10000000LL);
	ft.dwHighDateTime = li.HighPart;
	ft.dwLowDateTime = li.LowPart;
	::FileTimeToSystemTime(&ft, &st);

	CString sz;
	sz.Format(_T("%04d/%02d/%02d-%02d:%02d:%02d"),
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	return sz;
}

String PEPlus::Bin2GuidStr(PBYTE pBin)
{
	GUID guid = *LPGUID(pBin);
	WCHAR wszGuid[65];
	StringFromGUID2(guid, wszGuid, 64);
	return CString(wszGuid);
}


#include <TlHelp32.h>

HMODULE PEPlus::FindModuleInSnapshot(PCWSTR pszDllName, DWORD dwProcId)
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcId);
	if (hSnapshot == INVALID_HANDLE_VALUE)
		return NULL;

	HMODULE hModule = NULL;
	MODULEENTRY32 me = { sizeof(me) };
	BOOL bMoreMods = Module32First(hSnapshot, &me);
	for (; bMoreMods; bMoreMods = Module32Next(hSnapshot, &me))
	{
		if (_tcsicmp(me.szModule, pszDllName) == 0)
		{
			hModule = (HMODULE)me.modBaseAddr;
			break;
		}
	}
	CloseHandle(hSnapshot);
	return hModule;
}

HRESULT PEPlus::InjectModule(DWORD dwProcId, PCTSTR pszLibFile)
{
	HRESULT hr = S_OK;
	HANDLE hProcess = NULL, hThread = NULL;
	PWSTR pszProcPrms = NULL;

	try
	{
		HMODULE hKerMod = FindModuleInSnapshot(L"Kernel32.dll", dwProcId);
		if (hKerMod == NULL)
			throw HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);

		hProcess = OpenProcess
		(
			PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD |
			PROCESS_VM_OPERATION | PROCESS_VM_WRITE,
			FALSE, dwProcId
		);
		if (hProcess == NULL)
			throw HRESULT_FROM_WIN32(GetLastError());

		SIZE_T nNameLen = (_tcslen(pszLibFile) + 1) * sizeof(TCHAR);
		pszProcPrms = (PWSTR)VirtualAllocEx(hProcess,
			NULL, nNameLen, MEM_COMMIT, PAGE_READWRITE);
		if (pszProcPrms == NULL)
			throw HRESULT_FROM_WIN32(GetLastError());

		if (!WriteProcessMemory
			(
				hProcess, pszProcPrms, pszLibFile, nNameLen, NULL
			))
			throw HRESULT_FROM_WIN32(GetLastError());

		HMODULE hThisMod = GetModuleHandle(L"KERNEL32");
		PTHREAD_START_ROUTINE pfnThreadProc = (PTHREAD_START_ROUTINE)
			GetProcAddress(hThisMod, "LoadLibraryW");
		if (!pfnThreadProc)
			throw HRESULT_FROM_WIN32(GetLastError());

		hThread = CreateRemoteThread
		(
			hProcess, NULL, 0,
			pfnThreadProc, pszProcPrms, 0, NULL
		);
		if (hThread == NULL)
			throw HRESULT_FROM_WIN32(GetLastError());
		WaitForSingleObject(hThread, INFINITE);
	}
	catch (HRESULT e)
	{
		hr = e;
	}
	if (pszProcPrms != NULL)
		VirtualFreeEx(hProcess, pszProcPrms, 0, MEM_RELEASE);
	if (hThread != NULL)
		CloseHandle(hThread);
	if (hProcess != NULL)
		CloseHandle(hProcess);
	return hr;
}

HRESULT PEPlus::EjectModule(DWORD dwProcId, PCTSTR pszLibFile)
{
	HRESULT hr = S_OK;
	HANDLE hProcess = NULL, hThread = NULL;

	try
	{
		HMODULE hKerMod = FindModuleInSnapshot(pszLibFile, dwProcId);
		if (hKerMod == NULL)
			throw HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);

		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
			PROCESS_CREATE_THREAD |
			PROCESS_VM_OPERATION,
			FALSE, dwProcId);
		if (!hProcess)
			throw HRESULT_FROM_WIN32(GetLastError());

		PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)
			GetProcAddress(GetModuleHandle(L"KERNEL32"), "FreeLibrary");
		if (!pfnThreadRtn)
			throw HRESULT_FROM_WIN32(GetLastError());

		hThread = CreateRemoteThread(hProcess, NULL, 0,
			pfnThreadRtn, hKerMod, 0, NULL);
		if (!hThread)
			throw HRESULT_FROM_WIN32(GetLastError());

		WaitForSingleObject(hThread, INFINITE);
	}
	catch (HRESULT e)
	{
		hr = e;
	}
	if (hThread != NULL)
		CloseHandle(hThread);
	if (hProcess != NULL)
		CloseHandle(hProcess);
	return hr;
}
