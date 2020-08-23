#include <Windows.h>
#define REVERSE_FILEIO_API extern "C" __declspec(dllexport)
#include "ReverseFileIO.h"

#define	FSZ_MODULE	"KERNEL32.DLL"
#define	FSZ_READ	"ReadFile"
#define	FSZ_WRITE	"WriteFile"


FARPROC  g_pfnOrigRead  = NULL;		// Original function address in callee
FARPROC  g_pfnHookRead  = NULL;		// Hook function address
FARPROC  g_pfnOrigWrite = NULL;		// Original function address in callee
FARPROC  g_pfnHookWrite = NULL;		// Hook function address


// Prototypes for the hooked functions
typedef BOOL (WINAPI *PFN_FILEIO)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
BOOL WINAPI ReverseReadFile (HANDLE, LPVOID,  DWORD, LPDWORD, LPOVERLAPPED);
BOOL WINAPI ReverseWriteFile(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
HRESULT ReplaceIATEntry(PCSTR, FARPROC, FARPROC, HMODULE);
HRESULT ReplaceIATEntry2(FARPROC pfnCurFunc, FARPROC pfnNewFunc, HINSTANCE hModInst);


BOOL APIENTRY DllMain(HANDLE hModule, DWORD  dwReason, LPVOID)
{
	HRESULT hr = E_FAIL;
	HMODULE  hImgBase = GetModuleHandle(NULL);
	if(dwReason == DLL_PROCESS_ATTACH)
	{
		try
		{
			HMODULE hReplaseDll = GetModuleHandleA(FSZ_MODULE);
			if (!hReplaseDll)
				throw HRESULT_FROM_WIN32(GetLastError());

			////////////////////////////////////////////////////////////////////////
			// ReadFile Hooking...
			g_pfnHookRead = (FARPROC)ReverseReadFile;
			g_pfnOrigRead = GetProcAddress(hReplaseDll, FSZ_READ);
			if (!g_pfnOrigRead)
				throw HRESULT_FROM_WIN32(GetLastError());

			hr = ReplaceIATEntry(FSZ_MODULE, g_pfnOrigRead, g_pfnHookRead, hImgBase);
			//hr = ReplaceIATEntry2(g_pfnOrigRead, g_pfnHookRead, hImgBase);
			if (FAILED(hr))
				throw hr;
			////////////////////////////////////////////////////////////////////////


			////////////////////////////////////////////////////////////////////////
			// WriteFile Hooking...
			g_pfnHookWrite = (FARPROC)ReverseWriteFile;
			g_pfnOrigWrite = GetProcAddress(hReplaseDll, FSZ_WRITE);
			if (!g_pfnOrigWrite)
				throw HRESULT_FROM_WIN32(GetLastError());

			hr = ReplaceIATEntry(FSZ_MODULE, g_pfnOrigWrite, g_pfnHookWrite, hImgBase);
			//hr = ReplaceIATEntry2(g_pfnOrigWrite, g_pfnHookWrite, hImgBase);
			if (FAILED(hr))
				throw hr;
			////////////////////////////////////////////////////////////////////////
		}
		catch (HRESULT e)
		{
			TCHAR szMsg[256];
			wsprintf(szMsg, L"Error occurred, code=0x%08X.", e);
			MessageBox(NULL, szMsg, L"ReverseFileIO Error", MB_SYSTEMMODAL);
			return FALSE;
		}
	}
	else
	if(dwReason == DLL_PROCESS_DETACH)
	{
		if (g_pfnOrigRead)
			//ReplaceIATEntry(FSZ_MODULE, g_pfnHookRead, g_pfnOrigRead, hImgBase);
			ReplaceIATEntry2(g_pfnHookRead, g_pfnOrigRead, hImgBase);
		if (g_pfnOrigWrite)
			//ReplaceIATEntry(FSZ_MODULE, g_pfnHookWrite, g_pfnOrigWrite, hImgBase);
			ReplaceIATEntry2(g_pfnHookWrite, g_pfnOrigWrite, hImgBase);
	}

	return TRUE;
}

BOOL WINAPI ReverseReadFile(HANDLE hFile, 
							LPVOID lpBuffer, 
							DWORD nNumberOfBytesToRead, 
							LPDWORD lpNumberOfBytesRead, 
							LPOVERLAPPED lpOverlapped)
{
	BOOL bIsOK =  ReadFile(hFile, lpBuffer, nNumberOfBytesToRead, 
						   lpNumberOfBytesRead, lpOverlapped);
	if(!bIsOK)
		return FALSE;

	PBYTE lpSrc = (PBYTE)lpBuffer;
	DWORD  dwNumRev = ((*lpNumberOfBytesRead) >> 1);
	for(DWORD i=0; i<dwNumRev; i++)
	{
		BYTE ch = lpSrc[i];
		lpSrc[i] = lpSrc[(*lpNumberOfBytesRead) - i - 1];
		lpSrc[*lpNumberOfBytesRead - i - 1] = ch;
	}
	return TRUE;
}

BOOL WINAPI ReverseWriteFile(HANDLE hFile, 
							 LPCVOID lpBuffer, 
							 DWORD nNumberOfBytesToWrite, 
							 LPDWORD lpNumberOfBytesWritten, 
							 LPOVERLAPPED lpOverlapped)
{
	PBYTE lpSrc = (PBYTE)lpBuffer;
	if(*((LPWORD)lpSrc) != IMAGE_DOS_SIGNATURE)
	{
		DWORD  dwNumRev = (nNumberOfBytesToWrite>>1);
		for(DWORD i=0; i<dwNumRev; i++)
		{
			BYTE ch = lpSrc[i];
			lpSrc[i] = lpSrc[nNumberOfBytesToWrite - i - 1];
			lpSrc[nNumberOfBytesToWrite - i - 1] = ch;
		}
	}

	return WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, 
					 lpNumberOfBytesWritten,  lpOverlapped);
}
///////////////////////////////////////////////////////////////////////////////


HRESULT ReplaceIATEntry(PCSTR pszRepDllName, FARPROC pfnCurFunc,
					 FARPROC pfnNewFunc, HINSTANCE hModInst) 
{
	PBYTE pImgBase = (PBYTE)hModInst;
	if(!pImgBase)
		return E_INVALIDARG;
	PIMAGE_DOS_HEADER pdh = (PIMAGE_DOS_HEADER)pImgBase;
	if (pdh->e_magic != IMAGE_DOS_SIGNATURE)
		return E_INVALIDARG;
	PIMAGE_NT_HEADERS pnh = (PIMAGE_NT_HEADERS)(pImgBase + pdh->e_lfanew);
	if (pnh->Signature != IMAGE_NT_SIGNATURE)
		return E_INVALIDARG;

	PIMAGE_DATA_DIRECTORY pdd = &pnh->
		OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR pid = 
		(PIMAGE_IMPORT_DESCRIPTOR)(pImgBase + pdd->VirtualAddress);

	for (; pid->OriginalFirstThunk; pid++)
	{
		PSTR pszModName = (PSTR)(pImgBase + pid->Name);
		if(!_stricmp(pszModName, pszRepDllName)) 
			break;
	}
	if (pid->OriginalFirstThunk == 0)
		return E_INVALIDARG;

	PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)(pImgBase + pid->FirstThunk);
	for (; pThunk->u1.Function > 0; pThunk++)
	{
		FARPROC* ppfn = (FARPROC*)&pThunk->u1.Function;
		if (*ppfn != pfnCurFunc)
			continue;

		MEMORY_BASIC_INFORMATION mbi;
		DWORD dwOldProtect = 0;
		VirtualQuery(&pThunk->u1.Function, &mbi, sizeof(mbi));
		if ((mbi.Protect & PAGE_READWRITE) == 0)
		{
			DWORD dwNewProtect = mbi.Protect;
			dwNewProtect &= ~(PAGE_READONLY | PAGE_EXECUTE_READ);
			dwNewProtect |= (PAGE_READWRITE);
			VirtualProtect(&pThunk->u1.Function,
				sizeof(PVOID), dwNewProtect, &dwOldProtect);
		}

		*ppfn = pfnNewFunc;

		if (dwOldProtect > 0)
		{
			DWORD dwDontCare = 0;
			VirtualProtect(&pThunk->u1.Function,
				sizeof(PVOID), dwOldProtect, &dwDontCare);
		}
		return S_OK;
	}
	return E_INVALIDARG;
}

HRESULT ReplaceIATEntry2(FARPROC pfnCurFunc, FARPROC pfnNewFunc, HINSTANCE hModInst)
{
	PBYTE pImgBase = (PBYTE)hModInst;
	if (!pImgBase)
		return E_INVALIDARG;
	PIMAGE_DOS_HEADER pdh = (PIMAGE_DOS_HEADER)pImgBase;
	if (pdh->e_magic != IMAGE_DOS_SIGNATURE)
		return E_INVALIDARG;
	PIMAGE_NT_HEADERS pnh = (PIMAGE_NT_HEADERS)(pImgBase + pdh->e_lfanew);
	if (pnh->Signature != IMAGE_NT_SIGNATURE)
		return E_INVALIDARG;

	PIMAGE_DATA_DIRECTORY pdd = &pnh->
		OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT];
	PIMAGE_THUNK_DATA pThunk = (PIMAGE_THUNK_DATA)(pImgBase + pdd->VirtualAddress);
	DWORD dwSize = pdd->Size;
	for (; dwSize > 0; pThunk++)
	{
		FARPROC* ppfn = (FARPROC*)&pThunk->u1.Function;
		if (*ppfn != pfnCurFunc)
		{
			dwSize -= sizeof(IMAGE_THUNK_DATA);
			continue;
		}

		// 현재 메모리 보호속성을 획득한다.
		MEMORY_BASIC_INFORMATION mbi;
		DWORD dwOldProtect = 0;
		VirtualQuery(&pThunk->u1.Function, &mbi, sizeof(mbi));
		if ((mbi.Protect & PAGE_READWRITE) == 0)
		{
			// 현재 메모리속성 중에 ReadOnly와 ExecuteRead를 지우고
			// ReadWrite 속성을 추가한다.
			DWORD dwNewProtect = mbi.Protect;
			dwNewProtect &= ~(PAGE_READONLY | PAGE_EXECUTE_READ);
			dwNewProtect |= (PAGE_READWRITE);
			VirtualProtect(&pThunk->u1.Function,
				sizeof(PVOID), dwNewProtect, &dwOldProtect);
		}

		*ppfn = pfnNewFunc;

		if (dwOldProtect > 0)
		{
			// 이제 메모리 속성을 원래대로 되돌려놓는다.
			DWORD dwDontCare = 0;
			VirtualProtect(&pThunk->u1.Function,
				sizeof(PVOID), dwOldProtect, &dwDontCare);
		}
		return S_OK;
	}
	return E_INVALIDARG;
}
