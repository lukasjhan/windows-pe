#include "stdafx.h"
#include <windows.h>
#include <Shlobj.h>

#include <atlbase.h>
#include <atlstr.h>
#include "Dia2.h"
#pragma warning( disable : 4996 )

#include "StackWalkHelper.h"

#define IMAGE_PDB_SIGNATURE 0x53445352	// RSDS
typedef struct _IMAGE_PDB_INFO
{
	DWORD	Signature;
	BYTE	Guid[16];
	DWORD	Age;
	CHAR	PdbFileName[1];
} IMAGE_PDB_INFO, *PIMAGE_PDB_INFO;


HRESULT GetPdbPathFromModule(HMODULE hModuld, PWSTR pszPdbPath)
{
	PBYTE pImgBase = PBYTE(hModuld);
	PIMAGE_NT_HEADERS pnh = PIMAGE_NT_HEADERS(pImgBase + PIMAGE_DOS_HEADER(pImgBase)->e_lfanew);
	PIMAGE_DATA_DIRECTORY pdd = &pnh->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
	PIMAGE_DEBUG_DIRECTORY pdbgs = PIMAGE_DEBUG_DIRECTORY(pImgBase + pdd->VirtualAddress);

	int nItemCnt = pdd->Size / sizeof(IMAGE_DEBUG_DIRECTORY);
	for (int i = 0; i < nItemCnt; i++)
	{
		PIMAGE_DEBUG_DIRECTORY pdbg = &pdbgs[i];
		if (pdbg->Type == IMAGE_DEBUG_TYPE_CODEVIEW)
		{
			PIMAGE_PDB_INFO ppdb = PIMAGE_PDB_INFO(pImgBase + pdbg->AddressOfRawData);
			USES_CONVERSION;
			PCWSTR pszPdbFile = A2CT(PSTR(ppdb->PdbFileName));
			if (PathIsFileSpec(pszPdbFile))
			{
				GUID guid = *LPGUID(ppdb->Guid);
				WCHAR wszGuid[65];
				int nChLen = wsprintf(wszGuid, L"%08X%04X%04X", guid.Data1, guid.Data2, guid.Data3);
				for (int j = 0; j < 8; j++)
					nChLen += wsprintf(wszGuid + nChLen, L"%02X", guid.Data4[j]);
				//StringFromGUID2(guid, wszGuid, 64);

				PWSTR pszAppData;
				SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &pszAppData);
				wsprintf(pszPdbPath, L"%s\\Temp\\SymbolCache\\%s\\%s%d\\%s",
					pszAppData, pszPdbFile, wszGuid, ppdb->Age, pszPdbFile);
			}
			else
				wsprintf(pszPdbPath, L"%s", pszPdbFile);
			if (!PathFileExists(pszPdbPath))
				return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
			return S_OK;
		}
	}
	return E_FAIL;
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

		printf("...Forwarded: 0x%08X, Time: 0x%08X\n", pids[i].ForwarderChain, pids[i].TimeDateStamp);
		DWORD_PTR ulDllBase = (DWORD_PTR)GetModuleHandleA((PCSTR)(pids[i].Name + ulImageBase));
		if (ulDllBase == 0)
			continue;

		pnh = PIMAGE_NT_HEADERS(ulDllBase + PIMAGE_DOS_HEADER(ulDllBase)->e_lfanew);
		if (ulCtrlPc > ulDllBase && ulCtrlPc < ulDllBase + pnh->OptionalHeader.SizeOfImage)
			return ulDllBase;
	}
	return 0;
}

struct DIA_INTF
{
	DWORD_PTR				IMG_BASE;
	CComPtr<IDiaSession>	DIA_SESS;
	CComPtr<IDiaDataSource> DIA_SRC;

	DIA_INTF()
	{
		IMG_BASE = 0;
	}
	DIA_INTF(DWORD_PTR ulImageBase)
	{
		IMG_BASE = ulImageBase;
	}
	~DIA_INTF()
	{
		DIA_SESS = 0;
		DIA_SRC = 0;
	}
};
typedef DIA_INTF* PDIA_INTF;


#include <map>
typedef std::map<DWORD64, PDIA_INTF> MOD_DIA_MAP;
MOD_DIA_MAP G_MD_MAP;

PDIA_INTF LoadInterface(DWORD_PTR ulImgBase)
{
	TCHAR szPdbPath[MAX_PATH];
	PDIA_INTF pdi = NULL;

	try
	{
		HRESULT hr = GetPdbPathFromModule((HMODULE)ulImgBase, szPdbPath);
		if (FAILED(hr))
			throw hr;

		pdi = new DIA_INTF(ulImgBase);
		hr = CoCreateInstance
			(
			__uuidof(DiaSource), NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IDiaDataSource), (PVOID*)&pdi->DIA_SRC
			);
		if (FAILED(hr))
			throw hr;
		hr = pdi->DIA_SRC->loadDataFromPdb(szPdbPath);
		if (FAILED(hr))
			throw hr;
		hr = pdi->DIA_SRC->openSession(&pdi->DIA_SESS);
		if (FAILED(hr))
			throw hr;
	}
	catch (HRESULT hr)
	{
		printf("Error occured, code=0x%08X\n", hr);
		if (pdi != NULL)
		{
			delete pdi;
			pdi = NULL;
		}
	}
	return pdi;
}

void UnloadInterface()
{
	for (MOD_DIA_MAP::iterator it = G_MD_MAP.begin(); it != G_MD_MAP.end(); it++)
		delete it->second;
}


#include <map>
typedef std::map<DWORD, PCSTR> BP_FNC_MAP;
BP_FNC_MAP g_bfMap;


void DumpStackFrames(IDiaStackWalker* pStkWlkr, IDiaStackWalkHelper* pStkHlpr, CV_CPU_TYPE_e cpuType)
{
	if (pStkWlkr == NULL)// || pStkHlpr == NULL)
		return;

	CComPtr<IDiaEnumStackFrames> pSFEnums;
	if (SUCCEEDED(pStkWlkr->getEnumFrames2(cpuType, pStkHlpr, &pSFEnums)) && pSFEnums != NULL)
	{
		DWORD uCelt = 0;
		CComPtr<IDiaStackFrame> pISF;
		HRESULT hr = E_FAIL;
		while ((hr = pSFEnums->Next(1, &pISF, &uCelt)) == S_OK)
		{
			ULONGLONG ulBottom = 0, ulTop = 0;

			if (pISF->get_base(&ulBottom) == S_OK &&
				pISF->get_registerValue(CV_REG_ESP, &ulTop) == S_OK)
			{
				printf("range = 0x%08I64x - 0x%08I64x\n", ulBottom, ulTop);
			}

			ULONGLONG ulRetAddr = 0;
			if (pISF->get_returnAddress(&ulRetAddr) == S_OK)
			{
				printf("return address = 0x%08I64x\n", ulRetAddr);
			}

			DWORD dwLenFrame = 0;
			DWORD dwLenLocals = 0;
			DWORD dwLenParams = 0;
			DWORD dwLenProlog = 0;
			DWORD dwLenSavedRegs = 0;
			if (pISF->get_size(&dwLenFrame) == S_OK &&
				pISF->get_lengthLocals(&dwLenLocals) == S_OK &&
				pISF->get_lengthParams(&dwLenParams) == S_OK &&
				pISF->get_lengthProlog(&dwLenProlog) == S_OK &&
				pISF->get_lengthSavedRegisters(&dwLenSavedRegs) == S_OK)
			{
				printf("stack frame size          = 0x%08lx bytes\n", dwLenFrame);
				printf("length of locals          = 0x%08lx bytes\n", dwLenLocals);
				printf("length of parameters      = 0x%08lx bytes\n", dwLenParams);
				printf("length of prolog          = 0x%08lx bytes\n", dwLenProlog);
				printf("length of saved registers = 0x%08lx bytes\n", dwLenSavedRegs);
			}
			pISF = 0;
		}
		pSFEnums = 0;
	}
}

void WalkCallStack(PBYTE pCtrlPtr)
{
	CComPtr<IDiaSymbol> pSymFunc;
	DWORD count = 0;
	BSTR szFuncName;
	ULONGLONG ip = 0;

	CComPtr<IDiaStackWalker> pIStkWlkr;
	HRESULT hr = CoCreateInstance(__uuidof(DiaStackWalker), NULL,
		CLSCTX_INPROC_SERVER, __uuidof(IDiaStackWalker), (PVOID*)&pIStkWlkr);
	if (FAILED(hr))
	{
		printf("Get IDiaStackWalker failed: 0x%08X.", hr);
		return;
	}

	YhdStkWlkHlp* pHlpr = new YhdStkWlkHlp();
	DumpStackFrames(pIStkWlkr, pHlpr, CV_CFL_AMD64);
}


void F3()
{
	PBYTE pPEBase = (PBYTE)GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS32 pnh = (PIMAGE_NT_HEADERS32)(pPEBase + ((PIMAGE_DOS_HEADER)pPEBase)->e_lfanew);
	DWORD dwEntPos = (DWORD)(pPEBase + pnh->OptionalHeader.AddressOfEntryPoint);

	DWORD dwEBP = 0, dwBase = 0, dwPEB;
	__asm
	{
		mov dwEBP, ebp
		mov eax, fs:[04h]
		mov dwBase, eax
		mov eax, fs : [30h]
		mov dwPEB, eax
	}
	g_bfMap.insert(std::make_pair(dwEBP, "F3"));
	printf("Function \"F3\" called, EBP=0x%08X\n\n\n",  dwEBP);

	printf("EntryPtr : 0x%08X, PEB      : 0x%08X\n", dwEntPos, dwPEB);
	printf("ImageBase: 0x%p, StackBase: 0x%08X\n", pPEBase, dwBase);
	printf("Call Stack :\n");
	printf("Index\tFunc\tBeginAddr\tEBP(FP)\t\tPrevEBP\t\tRetAddr\n");

	WalkCallStack(0);
}

void F2()
{
	DWORD dwEBP = 0;
	__asm mov dwEBP, ebp
	g_bfMap.insert(std::make_pair(dwEBP, "F2"));
	printf("Function \"F2\" called, EBP=0x%08X\n", dwEBP);

	F3();
}


void F1()
{
	DWORD dwEBP = 0;
	__asm mov dwEBP, ebp
	g_bfMap.insert(std::make_pair(dwEBP, "F1"));
	printf("Function \"F1\" called, EBP=0x%08X\n", dwEBP);

	F2();
}

void _tmain()
{
	CoInitialize(NULL);

	DWORD dwEBP = 0;
	__asm mov dwEBP, ebp
	g_bfMap.insert(std::make_pair(dwEBP, "main"));
	printf("Function \"main\" called, EBP=0x%08X\n", dwEBP);

	F1();

	UnloadInterface();
	CoUninitialize();
}
