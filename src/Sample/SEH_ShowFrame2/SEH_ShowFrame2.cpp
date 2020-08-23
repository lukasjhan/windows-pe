#include "stdafx.h"
#include <windows.h>
#include <Shlobj.h>

#if !defined _M_IX86
#error	"Not support on X64"
#endif

#include <atlbase.h>
#include <atlstr.h>
#include "..\..\Solution\PEPlus\PEPlus.h"
#include "..\..\Solution\PEPdb\PEPdb.h"
#pragma warning( disable : 4996 )

#ifdef _DEBUG 
#	pragma comment(lib, "../../3.lib/x86/Debug/PEPlus.lib")
#	pragma comment(lib, "../../3.lib/x86/Debug/PEPdb.lib")
#else
#	pragma comment(lib, "../../3.lib/x86/Release/PEPlus.lib")
#	pragma comment(lib, "../../3.lib/x86/Release/PEPdb.lib")
#endif


#include <map>
typedef std::map<DWORD_PTR, PEPdb*> MOD_DIA_MAP;
MOD_DIA_MAP G_MD_MAP;

#define GSZ_APP_TMP_CACHE	_T("%s\\Temp\\SymbolCache")
TCHAR SYM_PATH[MAX_PATH];

PEPdb* LoadInterface(DWORD_PTR ulImgBase)
{
	if (SYM_PATH[0] == 0)
	{
		PWSTR pszAppData = NULL;
		SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &pszAppData);
		wsprintf(SYM_PATH, GSZ_APP_TMP_CACHE, pszAppData);
	}

	try
	{
		DWORD dwTimeStamp = 0;
		PIMAGE_PDB_INFO ppi = PEPlus::GetPdbInfo((PBYTE)ulImgBase, &dwTimeStamp, false);
		if (ppi == NULL)
			return NULL;
		return new PEPdb(ppi, dwTimeStamp, SYM_PATH);
	}
	catch (HRESULT hr)
	{
		printf("Error occured, code=0x%08X\n", hr);
		return NULL;
	}
}

void UnloadInterface()
{
	for (MOD_DIA_MAP::iterator it = G_MD_MAP.begin(); it != G_MD_MAP.end(); it++)
		delete it->second;
}

DWORD_PTR FindImageBase(DWORD_PTR ulCtrlPc)
{
	DWORD_PTR ulImageBase = (DWORD_PTR)GetModuleHandle(L"ntdll");
	PIMAGE_NT_HEADERS pnh = PIMAGE_NT_HEADERS(ulImageBase + PIMAGE_DOS_HEADER(ulImageBase)->e_lfanew);
	if (ulCtrlPc > ulImageBase && ulCtrlPc < ulImageBase + pnh->OptionalHeader.SizeOfImage)
		return ulImageBase;

	ulImageBase = (DWORD_PTR)GetModuleHandle(NULL);
	pnh = PIMAGE_NT_HEADERS(ulImageBase + PIMAGE_DOS_HEADER(ulImageBase)->e_lfanew);
	if (ulCtrlPc > ulImageBase && ulCtrlPc < ulImageBase + pnh->OptionalHeader.SizeOfImage)
		return ulImageBase;

	PIMAGE_DATA_DIRECTORY pdd = &pnh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	PIMAGE_IMPORT_DESCRIPTOR pids = (PIMAGE_IMPORT_DESCRIPTOR)(ulImageBase + pdd->VirtualAddress);
	int nItemCnt = pdd->Size / sizeof(IMAGE_IMPORT_DESCRIPTOR);

	for (int i = 0; i < nItemCnt; i++)
	{
		if (pids[i].Name == 0)
			continue;

		DWORD_PTR ulDllBase = (DWORD_PTR)GetModuleHandleA((PCSTR)(pids[i].Name + ulImageBase));
		if (ulDllBase == 0)
			continue;

		pnh = PIMAGE_NT_HEADERS(ulDllBase + PIMAGE_DOS_HEADER(ulDllBase)->e_lfanew);
		if (ulCtrlPc > ulDllBase && ulCtrlPc < ulDllBase + pnh->OptionalHeader.SizeOfImage)
			return ulDllBase;
	}
	return 0;
}


// Data structure(s) pointed to by Visual C++ extended exception frame
typedef struct _SCOPETABLE_ENTRY
{
	DWORD	PrevLevel;
	PVOID	FilterFunc;
	PVOID	HandlerFunc;
} SCOPETABLE_ENTRY, *PSCOPETABLE_ENTRY;

typedef struct _EH4_SCOPETABLE 
{
	DWORD				GSCookieOffset;
	DWORD				GSCookieXOROffset;
	DWORD				EHCookieOffset;
	DWORD				EHCookieXOROffset;
	SCOPETABLE_ENTRY	ScopeRecord[1];
} EH4_SCOPETABLE, *PEH4_SCOPETABLE;

struct VC_EXCEPTION_REGISTRATION : EXCEPTION_REGISTRATION_RECORD
{
	PVOID	ScopeTable;
	DWORD	TryLevel;
	DWORD	SavedEbp;
};
typedef VC_EXCEPTION_REGISTRATION* PVC_EXCEPTION_REGISTRATION;

extern "C" int _except_handler3(PEXCEPTION_RECORD, PEXCEPTION_REGISTRATION_RECORD,
	PCONTEXT, PEXCEPTION_RECORD);
extern "C" int _except_handler4(PEXCEPTION_RECORD, PEXCEPTION_REGISTRATION_RECORD,
	PCONTEXT, PEXCEPTION_RECORD);


DWORD GetSecurityCookie(HMODULE hModuld)
{
	PBYTE pImgBase = PBYTE(hModuld);
	PIMAGE_NT_HEADERS pnh = PIMAGE_NT_HEADERS(pImgBase + PIMAGE_DOS_HEADER(pImgBase)->e_lfanew);
	PIMAGE_DATA_DIRECTORY pdd = &pnh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG];
	PIMAGE_LOAD_CONFIG_DIRECTORY  plcd = PIMAGE_LOAD_CONFIG_DIRECTORY(pImgBase + pdd->VirtualAddress);
	return *((PDWORD)plcd->SecurityCookie);
}


DWORD GetCookieFromFrame(PVC_EXCEPTION_REGISTRATION pVER)
{
	DWORD dwSavedESP = *PDWORD(PBYTE(pVER) - 8);
	DWORD dwCookie   = *PDWORD(PBYTE(dwSavedESP));
	DWORD dwFramePtr = (DWORD)&pVER->SavedEbp;

	dwCookie ^= dwFramePtr;
	return dwCookie;
}



void ShowSEHFrame(PVC_EXCEPTION_REGISTRATION pVER)
{
	DWORD_PTR dwImageBase = FindImageBase((DWORD_PTR)pVER->Handler);
	if (dwImageBase == 0)
	{
		printf("----> Invalid Handler: 0x%08X\n", pVER->Handler);
		return;
	}

	PEPdb* pdi = NULL;
	MOD_DIA_MAP::iterator it = G_MD_MAP.find(dwImageBase);
	if (it == G_MD_MAP.end())
	{
		pdi = LoadInterface(dwImageBase);
		if (pdi != NULL)
			G_MD_MAP.insert(std::make_pair(dwImageBase, pdi));
	}
	else
		pdi = it->second;

	CComBSTR bszFunc, bszHandler;
	int nHdrType = -1;
	if (pdi != NULL)
	{
		CComPtr<IDiaSymbol> pISymb;
		DWORD dwRVA = (DWORD)pVER->Handler - dwImageBase;
		HRESULT	hr = pdi->SESSION->findSymbolByRVA(dwRVA, SymTagNull, &pISymb);
		if (hr == S_OK)
		{
			pISymb->get_name(&bszHandler);
			pISymb = 0;

			if (_wcsicmp(bszHandler, L"_except_handler3") == 0)
				nHdrType = 0;
			else if (_wcsicmp(bszHandler, L"_except_handler4") == 0 || 
				_wcsicmp(bszHandler, L"__except_handler4") == 0 )
				nHdrType = 1;

			if ((INT)pVER->Next != -1)
			{
				DWORD dwRetAddr = *PDWORD((PBYTE)pVER + 20);
				INT nRelVal = *PINT((PBYTE)dwRetAddr - 4);
				dwRetAddr += nRelVal;

				dwRVA = dwRetAddr - dwImageBase;
				hr = pdi->SESSION->findSymbolByRVA(dwRVA, SymTagNull, &pISymb);
				if (hr == S_OK)
				{
					pISymb->get_name(&bszFunc);
					pISymb = 0;
				}
			}
		}
	}


	if ((INT)pVER->Next != -1)
		printf("==> Function \"%S\" has SEH Frame\n", bszFunc);
	else
		printf("==> LAST SEH Frame\n");
	printf("    Frame Address=0x%08X, Previous Frame=0x%08X\n", pVER, pVER->Next);
	printf("    Handler=0x%08X (%S)\n", pVER->Handler, bszHandler);

	if (nHdrType >= 0)
	{
		printf("    * ScopeTable=0x%08X, TryLevel=%d, EBP=0x%08X\n", 
			pVER->ScopeTable, pVER->TryLevel, pVER->SavedEbp);
		PSCOPETABLE_ENTRY pstes = NULL;
		if (nHdrType)
		{
			DWORD dwCookie = GetCookieFromFrame(pVER);
			//DWORD dwCookie = GetSecurityCookie(GetModuleHandle(NULL));
			DWORD dwScopeTable = ((DWORD)pVER->ScopeTable) ^ dwCookie;
			PEH4_SCOPETABLE pste4 = PEH4_SCOPETABLE(dwScopeTable);
			pstes = pste4->ScopeRecord;
		}
		else
			pstes = PSCOPETABLE_ENTRY(pVER->ScopeTable);
		for (int i = 0; i <= (int)pVER->TryLevel; i++)
		{
			printf("      -> ScopeTable[%u] : PrevLevel=%2d, Filter=0x%08X, %s=0x%08X\n",
				i, pstes[i].PrevLevel, pstes[i].FilterFunc, 
				pstes[i].FilterFunc ? "Except" : "Finally", pstes[i].HandlerFunc);
		}
	}
	printf("\n");
}

void WalkSEHFrames()
{
	PVC_EXCEPTION_REGISTRATION pVER = NULL;
	__asm
	{
		mov  eax, fs:[0]
		mov [pVER], eax
	}

	while (0xFFFFFFFF != (unsigned)pVER)
	{
		ShowSEHFrame(pVER);
		pVER = (PVC_EXCEPTION_REGISTRATION)pVER->Next;
	}
}

void TestTryExcept()
{
	__try
	{
		__try
		{
			__try
			{
				WalkSEHFrames();
			}
			__except (EXCEPTION_CONTINUE_SEARCH)
			{
			}
		}
		//__except (EXCEPTION_CONTINUE_SEARCH)
		__finally
		{
		}
	}
	__except (EXCEPTION_CONTINUE_SEARCH)
	{
	}
}

void _tmain()
{
	CoInitialize(NULL);

	////////////////////////////////////////////////////////////////////////////////////////////////
	//
	__try
	{
		TestTryExcept();
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		printf("Caught Exception in main\n");
	}
	////////////////////////////////////////////////////////////////////////////////////////////////

	UnloadInterface();
	CoUninitialize();
}


//void Function1(void)
//{
//	// Set up 3 nested __try levels (thereby forcing 3 scopetable entries)
//	__try
//	{
//		__try
//		{
//			__try
//			{
//				WalkSEHFrames();    // Now show all the exception frames
//			}
//			__except (EXCEPTION_CONTINUE_SEARCH)
//			{
//			}
//		}
//		__except (EXCEPTION_CONTINUE_SEARCH)
//		{
//		}
//	}
//	__except (EXCEPTION_CONTINUE_SEARCH)
//	{
//	}
//
//	__try
//	{
//		WalkSEHFrames();    // Now show all the exception frames
//	}
//	__except (EXCEPTION_EXECUTE_HANDLER)
//	{
//		// Should never get here, since we aren't expecting an exception
//		printf("Caught Exception in main\n");
//	}
//
//}
//
//int main()
//{
//	int i;
//
//	// Use two (non-nested) __try blocks.  This causes two scopetable entries
//	// to be generated for the function.
//
//	__try
//	{
//		__try
//		{
//			i = 0x1234;     // Do nothing in particular
//			WalkSEHFrames();    // Now show all the exception frames
//		}
//		__except (EXCEPTION_CONTINUE_SEARCH)
//		{
//			i = 0x4321;     // Do nothing (in reverse)
//		}
//	}
//	__finally
//	{
//	}
//
//	__try
//	{
//		Function1();    // Call a function that sets up more exception frames
//	}
//	__except (EXCEPTION_EXECUTE_HANDLER)
//	{
//		// Should never get here, since we aren't expecting an exception
//		printf("Caught Exception in main\n");
//	}
//
//	return 0;
//}
