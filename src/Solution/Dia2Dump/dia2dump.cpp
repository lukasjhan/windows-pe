#include "stdafx.h"
#include "Dia2Dump.h"
#include "PrintSymbol.h"

#pragma warning (disable : 4100)

#define PDB_SEARCH_URL	L"SRV**\\\\symbols\\symbols"

void PrintSymbolsByAddress(IDiaSession* pISession);

////////////////////////////////////////////////////////////
//
void wmain(int argc, PWSTR argv[])
{
	if (argc < 2)
	{
		PrintHelpOptions();
		return;
	}
	CoInitialize(NULL);

	PCWSTR pszPdbPath = argv[argc - 1];
	WCHAR szExt[16];
	_wsplitpath_s(pszPdbPath, NULL, 0, NULL, 0, NULL, 0, szExt, 16);

	CComPtr<IDiaDataSource>	pIDataSrc;
	CComPtr<IDiaSession>	pISession;
	CComPtr<IDiaSymbol>		pIPEScope;

	try
	{
		HRESULT hr = CoCreateInstance
		(
			__uuidof(DiaSource), NULL, CLSCTX_INPROC_SERVER, 
			__uuidof(IDiaDataSource), (PVOID*)&pIDataSrc
		);
		if (FAILED(hr))
			throw hr;

		if (_wcsicmp(szExt, L".pdb") == 0)
			hr = pIDataSrc->loadDataFromPdb(pszPdbPath);
		else
			hr = pIDataSrc->loadDataForExe(pszPdbPath, PDB_SEARCH_URL, NULL);
		if (FAILED(hr))
			throw hr;

		hr = pIDataSrc->openSession(&pISession);
		if (FAILED(hr))
			throw hr;

		hr = pISession->get_globalScope(&pIPEScope);
		if (hr != S_OK)
			throw hr;

		DumpGlobalScope(pIPEScope);
		if (argc > 2)
		{
			if (!_wcsicmp(argv[1], L"-all"))
				DumpAllPdbInfo(pISession, pIPEScope);
			else
				ParseArg(argc - 2, &argv[1], pISession, pIPEScope);
		}
		else
		{
			PrintSymbolsByAddress(pISession);
		}
	}
	catch (HRESULT e)
	{
		wprintf(L"PDB Session load failed - HRESULT = 0x%08X\n", e);
	}

	if (pIPEScope)	pIPEScope = 0;
	if (pISession)	pISession = 0;
	if (pIDataSrc)	pIDataSrc = 0;
	CoUninitialize();
}

////////////////////////////////////////////////////////////
// Parse the arguments of the program
//
bool ParseArg(int argc, wchar_t *argv[], IDiaSession* pISession, IDiaSymbol* pIPEScope)
{
	int iCount = 0;
	bool bReturn = true;

	if (!argc)
		return true;

	if (!_wcsicmp(argv[0], L"-?"))
	{
		PrintHelpOptions();
		return true;
	}
	else if (!_wcsicmp(argv[0], L"-help"))
	{
		PrintHelpOptions();
		return true;
	}
	else if (!_wcsicmp(argv[0], L"-m"))
	{
		// -m                : print all the mods
		iCount = 1;
		bReturn = bReturn && DumpAllMods(pIPEScope);
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-p"))
	{
		// -p                : print all the publics
		iCount = 1;
		bReturn = bReturn && DumpAllPublics(pIPEScope);
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-s"))
	{
		// -s                : print symbols
		iCount = 1;
		bReturn = bReturn && DumpAllSymbols(pIPEScope);
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-g"))
	{
		// -g                : print all the globals
		iCount = 1;
		bReturn = bReturn && DumpAllGlobals(pIPEScope);
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-t")) 
	{
		// -t                : print all the types
		iCount = 1;
		bReturn = bReturn && DumpAllTypes(pIPEScope);
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-f"))
	{
		// -f                : print all the files
		iCount = 1;
		bReturn = bReturn && DumpAllFiles(pISession, pIPEScope);
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}

	else if (!_wcsicmp(argv[0], L"-l"))
	{
		if (argc > 1 && *argv[1] != L'-') 
		{
			// -l RVA [bytes]    : print line number info at RVA address in the bytes range
			DWORD dwRVA = 0;
			DWORD dwRange = MAX_RVA_LINES_BYTES_RANGE;
			swscanf_s(argv[1], L"%x", &dwRVA);
			if (argc > 2 && *argv[2] != L'-')
			{
				swscanf_s(argv[2], L"%d", &dwRange);
				iCount = 3;
			}
			else
				iCount = 2;
			bReturn = bReturn && DumpAllLines(pISession, dwRVA, dwRange);
		}
		else
		{
			// -l            : print line number info
			bReturn = bReturn && DumpAllLines(pISession, pIPEScope);
			iCount = 1;
		}
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-c")) 
	{
		// -c                : print section contribution info
		iCount = 1;
		bReturn = bReturn && DumpAllSecContribs(pISession);
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-dbg")) 
	{
		// -dbg              : dump debug streams
		iCount = 1;
		bReturn = bReturn && DumpAllDebugStreams(pISession);
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-injsrc"))
	{
		if (argc > 1 && *argv[1] != L'-')
		{
			// -injsrc filename          : dump injected source filename
			bReturn = bReturn && DumpInjectedSource(pISession, argv[1]);
			iCount = 2;
		}
		else
		{
			// -injsrc           : dump all injected source
			bReturn = bReturn && DumpAllInjectedSources(pISession);
			iCount = 1;
		}
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-sf"))
	{
		// -sf               : dump all source files
		iCount = 1;
		bReturn = bReturn && DumpAllSourceFiles(pISession, pIPEScope);
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-oem"))
	{
		// -oem              : dump all OEM specific types
		iCount = 1;
		bReturn = bReturn && DumpAllOEMs(pIPEScope);
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}

	else if (!_wcsicmp(argv[0], L"-fpo")) 
	{
		if (argc > 1 && *argv[1] != L'-')
		{
			DWORD dwRVA = 0;

			if (iswdigit(*argv[1]))
			{
				// -fpo [RVA]        : dump frame pointer omission information for a function address
				swscanf_s(argv[1], L"%x", &dwRVA);
				bReturn = bReturn && DumpFPO(pISession, dwRVA);
			}
			else
			{
				// -fpo [symbolname] : dump frame pointer omission information for a function symbol
				bReturn = bReturn && DumpFPO(pISession, pIPEScope, argv[1]);
			}
			iCount = 2;
		}
		else
		{
			bReturn = bReturn && DumpAllFPO(pISession);
			iCount = 1;
		}
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-compiland"))
	{
		if ((argc > 1) && (*argv[1] != L'-'))
		{
			// -compiland [name] : dump symbols for this compiland
			bReturn = bReturn && DumpCompiland(pIPEScope, argv[1]);
			argc -= 2;
		}
		else 
		{
			wprintf(L"ERROR - ParseArg(): missing argument for option '-line'");
			return false;
		}
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-lines")) 
	{
		if ((argc > 1) && (*argv[1] != L'-'))
		{
			DWORD dwRVA = 0;
			if (iswdigit(*argv[1])) 
			{
				// -lines <RVA>                  : dump line numbers for this address\n"
				swscanf_s(argv[1], L"%x", &dwRVA);
				bReturn = bReturn && DumpLines(pISession, dwRVA);
			}
			else
			{
				// -lines <symbolname>           : dump line numbers for this function
				bReturn = bReturn && DumpLines(pISession, pIPEScope, argv[1]);
			}
			iCount = 2;
		}
		else 
		{
			wprintf(L"ERROR - ParseArg(): missing argument for option '-compiland'");
			return false;
		}
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-type"))
	{
		// -type <symbolname>: dump this type in detail
		if ((argc > 1) && (*argv[1] != L'-'))
		{
			bReturn = bReturn && DumpType(pIPEScope, argv[1]);
			iCount = 2;
		}
		else
		{
			wprintf(L"ERROR - ParseArg(): missing argument for option '-type'");
			return false;
		}
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-label"))
	{
		// -label <RVA>      : dump label at RVA
		if ((argc > 1) && (*argv[1] != L'-')) 
		{
			DWORD dwRVA = 0;
			swscanf_s(argv[1], L"%x", &dwRVA);
			bReturn = bReturn && DumpLabel(pISession, dwRVA);
			iCount = 2;
		}
		else 
		{
			wprintf(L"ERROR - ParseArg(): missing argument for option '-label'");
			return false;
		}
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-sym"))
	{
		if ((argc > 1) && (*argv[1] != L'-'))
		{
			DWORD dwRVA = 0;
			PCWSTR szChildname = NULL;

			iCount = 2;
			if (argc > 2 && *argv[2] != L'-')
			{
				szChildname = argv[2];
				iCount = 3;
			}

			if (iswdigit(*argv[1]))
			{
				// -sym <RVA> [childname]        : dump child information of symbol at this address
				swscanf_s(argv[1], L"%x", &dwRVA);
				bReturn = bReturn && DumpSymbolWithRVA(pISession, dwRVA, szChildname);
			}
			else 
			{
				// -sym <symbolname> [childname] : dump child information of this symbol
				bReturn = bReturn && DumpSymbolsWithRegEx(pIPEScope, argv[1], szChildname);
			}
		}
		else
		{
			wprintf(L"ERROR - ParseArg(): missing argument for option '-sym'");
			return false;
		}
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-lsrc")) 
	{
		// -lsrc  <file> [line]          : dump line numbers for this source file
		if ((argc > 1) && (*argv[1] != L'-'))
		{
			DWORD dwLine = 0;
			iCount = 2;
			if (argc > 2 && *argv[2] != L'-')
			{
				swscanf_s(argv[1], L"%d", &dwLine);
				iCount = 3;
			}
			bReturn = bReturn && DumpLinesForSourceFile(pISession, argv[1], dwLine);
		}
		else 
		{
			wprintf(L"ERROR - ParseArg(): missing argument for option '-lsrc'");
			return false;
		}
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-ps"))
	{
		// -ps <RVA> [-n <number>]       : dump symbols after this address, default 16
		if ((argc > 1) && (*argv[1] != L'-'))
		{
			DWORD dwRVA = 0;
			DWORD dwRange;
			swscanf_s(argv[1], L"%x", &dwRVA);
			if (argc > 3 && !_wcsicmp(argv[2], L"-n"))
			{
				swscanf_s(argv[3], L"%d", &dwRange);
				iCount = 4;
			}
			else 
			{
				dwRange = 16;
				iCount = 2;
			}
			bReturn = bReturn && DumpPublicSymbolsSorted(pISession, dwRVA, dwRange, true);
		}
		else 
		{
			wprintf(L"ERROR - ParseArg(): missing argument for option '-ps'");
			return false;
		}
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-psr"))
	{
		// -psr <RVA> [-n <number>]       : dump symbols before this address, default 16
		if ((argc > 1) && (*argv[1] != L'-')) 
		{
			DWORD dwRVA = 0, dwRange;
			swscanf_s(argv[1], L"%x", &dwRVA);
			if (argc > 3 && !_wcsicmp(argv[2], L"-n"))
			{
				swscanf_s(argv[3], L"%d", &dwRange);
				iCount = 4;
			}
			else 
			{
				dwRange = 16;
				iCount = 2;
			}
			bReturn = bReturn && DumpPublicSymbolsSorted(pISession, dwRVA, dwRange, false);
		}
		else
		{
			wprintf(L"ERROR - ParseArg(): missing argument for option '-psr'");
			return false;
		}
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-annotations"))
	{
		// -annotations <RVA>: dump annotation symbol for this RVA
		if ((argc > 1) && (*argv[1] != L'-'))
		{
			DWORD dwRVA = 0;
			swscanf_s(argv[1], L"%x", &dwRVA);
			bReturn = bReturn && DumpAnnotations(pISession, dwRVA);
			iCount = 2;
		}
		else 
		{
			wprintf(L"ERROR - ParseArg(): missing argument for option '-maptosrc'");
			return false;
		}
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-maptosrc")) 
	{
		// -maptosrc <RVA>   : dump src RVA for this image RVA
		if ((argc > 1) && (*argv[1] != L'-'))
		{
			DWORD dwRVA = 0;
			swscanf_s(argv[1], L"%x", &dwRVA);
			bReturn = bReturn && DumpMapToSrc(pISession, dwRVA);
			iCount = 2;
		}
		else 
		{
			wprintf(L"ERROR - ParseArg(): missing argument for option '-maptosrc'");
			return false;
		}
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else if (!_wcsicmp(argv[0], L"-mapfromsrc"))
	{
		// -mapfromsrc <RVA> : dump image RVA for src RVA
		if ((argc > 1) && (*argv[1] != L'-'))
		{
			DWORD dwRVA = 0;
			swscanf_s(argv[1], L"%x", &dwRVA);
			bReturn = bReturn && DumpMapFromSrc(pISession, dwRVA);
			iCount = 2;
		}
		else
		{
			wprintf(L"ERROR - ParseArg(): missing argument for option '-mapfromsrc'");
			return false;
		}
		argc -= iCount;
		bReturn = bReturn && ParseArg(argc, &argv[iCount], pISession, pIPEScope);
	}
	else
	{
		wprintf(L"ERROR - unknown option %s\n", argv[0]);
		PrintHelpOptions();
		return false;
	}

	return bReturn;
}

////////////////////////////////////////////////////////////
// Display the usage
//
void PrintHelpOptions()
{
	static PCWSTR  const helpString = 
	L"usage: Dia2Dump.exe [ options ] <filename>\n"
	L"  -?                : print this help\n"
	L"  -all              : print all the debug info\n"
	L"  -m                : print all the mods\n"
	L"  -p                : print all the publics\n"
	L"  -g                : print all the globals\n"
	L"  -t                : print all the types\n"
	L"  -f                : print all the files\n"
	L"  -s                : print symbols\n"
	L"  -l [RVA [bytes]]  : print line number info at RVA address in the bytes range\n"
	L"  -c                : print section contribution info\n"
	L"  -dbg              : dump debug streams\n"
	L"  -injsrc [file]    : dump injected source\n"
	L"  -sf               : dump all source files\n"
	L"  -oem              : dump all OEM specific types\n"
	L"  -fpo [RVA]        : dump frame pointer omission information for a func addr\n"
	L"  -fpo [symbolname] : dump frame pointer omission information for a func symbol\n"
	L"  -compiland [name] : dump symbols for this compiland\n"
	L"  -lines <funcname> : dump line numbers for this function\n"
	L"  -lines <RVA>      : dump line numbers for this address\n"
	L"  -type <symbolname>: dump this type in detail\n"
	L"  -label <RVA>      : dump label at RVA\n"
	L"  -sym <symbolname> [childname] : dump child information of this symbol\n"
	L"  -sym <RVA> [childname]        : dump child information of symbol at this addr\n"
	L"  -lsrc  <file> [line]          : dump line numbers for this source file\n"
	L"  -ps <RVA> [-n <number>]       : dump symbols after this address, default 16\n"
	L"  -psr <RVA> [-n <number>]      : dump symbols before this address, default 16\n"
	L"  -annotations <RVA>: dump annotation symbol for this RVA\n"
	L"  -maptosrc <RVA>   : dump src RVA for this image RVA\n"
	L"  -mapfromsrc <RVA> : dump image RVA for src RVA\n";
	wprintf(helpString);
}



void DumpGlobalScope(IDiaSymbol* pIPEScope)
{
	CComBSTR bszName;
	if (pIPEScope->get_name(&bszName) == S_OK)
		wprintf(L"PDB Executable => %s\n", bszName);

	DWORD dwAge = 0;
	if (pIPEScope->get_age(&dwAge) == S_OK)
		wprintf(L"  Age            : %d\n", dwAge);

	GUID guid;
	if (pIPEScope->get_guid(&guid) == S_OK)
	{
		WCHAR wszGuid[65];
		StringFromGUID2(guid, wszGuid, 64);
		wprintf(L"  GUID           : %s\n", wszGuid);
	}

	DWORD dwSignature = 0;
	if (pIPEScope->get_signature(&dwSignature) == S_OK)
		wprintf(L"  Signature      : 0x%08X\n", dwSignature);

	DWORD dwMachine = 0;
	if (pIPEScope->get_machineType(&dwMachine) == S_OK)
	{
		CV_CPU_TYPE_e eCpu = (CV_CPU_TYPE_e)-1;
		switch (dwMachine)
		{
			case IMAGE_FILE_MACHINE_I386 : eCpu = CV_CFL_80386; break;
			case IMAGE_FILE_MACHINE_IA64 : eCpu = CV_CFL_IA64 ; break;
			case IMAGE_FILE_MACHINE_AMD64: eCpu = CV_CFL_AMD64; break;
		}
		if (eCpu >= 0)
			wprintf(L"  MachineType    : 0x%X (%s)\n", dwMachine, GSZ_CPU_TYPES[eCpu]);
		else
			wprintf(L"  MachineType    : %d\n", dwMachine);
	}

	CComBSTR bszSymFile;
	if (pIPEScope->get_symbolsFileName(&bszSymFile) == S_OK)
		wprintf(L"  SymbolsFileName: %s\n", bszSymFile);

	BOOL bFlags;
	wprintf(L"  Attributes     :");
	if (pIPEScope->get_isCTypes(&bFlags) == S_OK && bFlags)
		wprintf(L" CTypes");
	if (pIPEScope->get_isStripped(&bFlags) == S_OK && bFlags)
		wprintf(L" Stripped");
	wprintf(L"\n\n");
}

////////////////////////////////////////////////////////////
// Dump all the data stored in a PDB
//
void DumpAllPdbInfo(IDiaSession* pISession, IDiaSymbol* pIPEScope)
{
	DumpAllMods(pIPEScope);
	DumpAllPublics(pIPEScope);
	DumpAllSymbols(pIPEScope);
	DumpAllGlobals(pIPEScope);
	DumpAllTypes(pIPEScope);
	DumpAllFiles(pISession, pIPEScope);
	DumpAllLines(pISession, pIPEScope);
	DumpAllSecContribs(pISession);
	DumpAllDebugStreams(pISession);
	DumpAllInjectedSources(pISession);
	DumpAllFPO(pISession);
	DumpAllOEMs(pIPEScope);
}

////////////////////////////////////////////////////////////
// Dump all the modules information
//
bool DumpAllMods(IDiaSymbol* pIPEScope)
{
	wprintf(L"\n\n*** MODULES\n\n");

	// Retrieve all the compiland symbols

	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (FAILED(pIPEScope->findChildren(SymTagCompiland, NULL, nsNone, &pIEnumSyms)))
		return false;

	CComPtr<IDiaSymbol> pICompiland;
	ULONG celt = 0, iMod = 1;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pICompiland, &celt)) && (celt == 1))
	{
		CComBSTR bszName;
		if (pICompiland->get_name(&bszName) != S_OK)
		{
			wprintf(L"ERROR - Failed to get the compiland's name\n");
			pICompiland = 0;
			pIEnumSyms = 0;
			return false;
		}
		wprintf(L"%04X %s\n", iMod++, bszName);
		pICompiland = 0;
	}
	pIEnumSyms = 0;
	putwchar(L'\n');
	return true;
}

////////////////////////////////////////////////////////////
// Dump all the public symbols - SymTagPublicSymbol
//
bool DumpAllPublics(IDiaSymbol* pIPEScope)
{
	wprintf(L"\n\n*** PUBLICS\n\n");

	// Retrieve all the public symbols
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (FAILED(pIPEScope->findChildren(SymTagPublicSymbol, NULL, nsNone, &pIEnumSyms)))
		return false;

	ULONG celt = 0;
	CComPtr<IDiaSymbol> pISymbol;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pISymbol, &celt)) && (celt == 1))
	{
		PrintPublicSymbol(pISymbol);
		pISymbol = 0;
	}
	pIEnumSyms = 0;
	putwchar(L'\n');
	return true;
}

////////////////////////////////////////////////////////////
// Dump all the symbol information stored in the compilands
//
bool DumpAllSymbols(IDiaSymbol* pIPEScope)
{
	wprintf(L"\n\n*** SYMBOLS\n\n\n");

	// Retrieve the compilands first
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (FAILED(pIPEScope->findChildren(SymTagCompiland, NULL, nsNone, &pIEnumSyms)))
		return false;

	CComPtr<IDiaSymbol> pICompiland;
	ULONG celt = 0;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pICompiland, &celt)) && (celt == 1))
	{
		wprintf(L"\n** Module: ");

		// Retrieve the name of the module
		CComBSTR bszName;
		if (pICompiland->get_name(&bszName) != S_OK)
			wprintf(L"(???)\n\n");
		else
			wprintf(L"%s\n\n", bszName);

		// Find all the symbols defined in this compiland and print their info
		CComPtr<IDiaEnumSymbols> pIEnumChild;
		if (SUCCEEDED(pICompiland->findChildren(SymTagNull, NULL, nsNone, &pIEnumChild)))
		{
			CComPtr<IDiaSymbol> pISymbol;
			ULONG celtChildren = 0;
			while (SUCCEEDED(pIEnumChild->Next(1, &pISymbol, &celtChildren)) && (celtChildren == 1))
			{
				PrintSymbol(pISymbol, 0);
				pISymbol = 0;
			}
			pIEnumChild = 0;
		}
		pICompiland = 0;
	}
	pIEnumSyms = 0;
	putwchar(L'\n');
	return true;
}

////////////////////////////////////////////////////////////
// Dump all the global symbols - SymTagFunction,
//  SymTagThunk and SymTagData
//
bool DumpAllGlobals(IDiaSymbol* pIPEScope)
{
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	CComPtr<IDiaSymbol> pISymbol;
	enum SymTagEnum dwSymTags[] = { SymTagFunction, SymTagThunk, SymTagData };
	ULONG celt = 0;

	wprintf(L"\n\n*** GLOBALS\n\n");
	for (size_t i = 0; i < _countof(dwSymTags); i++, pIEnumSyms = NULL)
	{
		if (SUCCEEDED(pIPEScope->findChildren(dwSymTags[i], NULL, nsNone, &pIEnumSyms)))
		{
			while (SUCCEEDED(pIEnumSyms->Next(1, &pISymbol, &celt)) && (celt == 1))
			{
				PrintGlobalSymbol(pISymbol);
				pISymbol = 0;
			}
			pIEnumSyms = 0;
		}
		else
		{
			wprintf(L"ERROR - DumpAllGlobals() returned no symbols\n");
			return false;
		}
	}
	putwchar(L'\n');

	return true;
}

////////////////////////////////////////////////////////////
// Dump all the types information
//  (type symbols can be UDTs, enums or typedefs)
//
bool DumpAllTypes(IDiaSymbol* pIPEScope)
{
	wprintf(L"\n\n*** TYPES\n");
	bool f1 = DumpAllUDTs(pIPEScope);
	bool f2 = DumpAllEnums(pIPEScope);
	bool f3 = DumpAllTypedefs(pIPEScope);
	return f1 && f2 && f3;
}

////////////////////////////////////////////////////////////
// Dump all the user defined types (UDT)
//
bool DumpAllUDTs(IDiaSymbol* pIPEScope)
{
	wprintf(L"\n\n********* User Defined Types\n\n");

	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (FAILED(pIPEScope->findChildren(SymTagUDT, NULL, nsNone, &pIEnumSyms)))
	{
		wprintf(L"ERROR - DumpAllUDTs() returned no symbols\n");
		return false;
	}

	CComPtr<IDiaSymbol> pISymbol;
	ULONG celt = 0;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pISymbol, &celt)) && (celt == 1))
	{
		PrintTypeInDetail(pISymbol, 0);
		pISymbol = 0;
	}
	pIEnumSyms = 0;
	putwchar(L'\n');
	return true;
}

////////////////////////////////////////////////////////////
// Dump all the enum types from the pdb
//
bool DumpAllEnums(IDiaSymbol* pIPEScope)
{
	wprintf(L"\n\n********* ENUMERATES\n\n");

	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (FAILED(pIPEScope->findChildren(SymTagEnum, NULL, nsNone, &pIEnumSyms)))
	{
		wprintf(L"ERROR - DumpAllEnums() returned no symbols\n");
		return false;
	}

	ULONG celt = 0;
	CComPtr<IDiaSymbol> pISymbol;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pISymbol, &celt)) && (celt == 1))
	{
		PrintTypeInDetail(pISymbol, 0);
		pISymbol = 0;
	}
	pIEnumSyms = 0;
	putwchar(L'\n');

	return true;
}

////////////////////////////////////////////////////////////
// Dump all the typedef types from the pdb
//
bool DumpAllTypedefs(IDiaSymbol* pIPEScope)
{
	wprintf(L"\n\n********* TYPEDEFS\n\n");

	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (FAILED(pIPEScope->findChildren(SymTagTypedef, NULL, nsNone, &pIEnumSyms)))
	{
		wprintf(L"ERROR - DumpAllTypedefs() returned no symbols\n");
		return false;
	}

	CComPtr<IDiaSymbol> pISymbol;
	ULONG celt = 0;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pISymbol, &celt)) && (celt == 1))
	{
		PrintTypeInDetail(pISymbol, 0);
		pISymbol = 0;
	}
	pIEnumSyms = 0;
	putwchar(L'\n');

	return true;
}

////////////////////////////////////////////////////////////
// Dump OEM specific types
//
bool DumpAllOEMs(IDiaSymbol* pIPEScope)
{
	wprintf(L"\n\n*** OEM Specific types\n\n");

	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (FAILED(pIPEScope->findChildren(SymTagCustomType, NULL, nsNone, &pIEnumSyms)))
	{
		wprintf(L"ERROR - DumpAllOEMs() returned no symbols\n");
		return false;
	}

	ULONG celt = 0;
	CComPtr<IDiaSymbol> pISymbol;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pISymbol, &celt)) && (celt == 1))
	{
		PrintTypeInDetail(pISymbol, 0);
		pISymbol = 0;
	}
	pIEnumSyms = 0;
	putwchar(L'\n');

	return true;
}

////////////////////////////////////////////////////////////
// For each compiland in the PDB dump all the source files
//
bool DumpAllFiles(IDiaSession* pISession, IDiaSymbol* pIPEScope)
{
	wprintf(L"\n\n*** FILES\n\n");

	// In order to find the source files, we have to look at the image's compilands/modules
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	//if (FAILED(pIPEScope->findChildren(SymTagCompiland, L"*DAsmTest.obj", nsfRegularExpression, &pIEnumSyms))) {
	if (FAILED(pIPEScope->findChildren(SymTagCompiland, NULL, nsNone, &pIEnumSyms)))
		return false;

	CComPtr<IDiaSymbol> pICompiland;
	ULONG celt = 0;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pICompiland, &celt)) && (celt == 1))
	{
		CComBSTR bszName;
		if (pICompiland->get_name(&bszName) == S_OK)
			wprintf(L"\nCompiland = %s\n\n", bszName);

		// Every compiland could contain multiple references to the source files which were used to build it
		// Retrieve all source files by compiland by passing NULL for the name of the source file
		CComPtr<IDiaEnumSourceFiles> pIEnumSrcFile;
		if (SUCCEEDED(pISession->findFile(pICompiland, NULL, nsNone, &pIEnumSrcFile)))
		{
			CComPtr<IDiaSourceFile> pISrcFile;
			while (SUCCEEDED(pIEnumSrcFile->Next(1, &pISrcFile, &celt)) && (celt == 1))
			{
				PrintSourceFile(pISrcFile);
				putwchar(L'\n');
				pISrcFile = 0;
			}
			pIEnumSrcFile = 0;
		}
		putwchar(L'\n');
		pICompiland = 0;
	}
	pIEnumSyms = 0;

	return true;
}

////////////////////////////////////////////////////////////
// Dump all the line numbering information contained in the PDB
//  Only function symbols have corresponding line numbering information
bool DumpAllLines(IDiaSession* pISession, IDiaSymbol* pIPEScope)
{
	wprintf(L"\n\n*** LINES\n\n");
	// First retrieve the compilands/modules
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (FAILED(pIPEScope->findChildren(SymTagCompiland, NULL, nsNone, &pIEnumSyms)))
		return false;

	CComPtr<IDiaSymbol> pICompiland;
	ULONG celt = 0;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pICompiland, &celt)) && (celt == 1))
	{
		CComPtr<IDiaEnumSymbols> pIEnumFunc;
		// For every function symbol defined in the compiland, retrieve and print the line numbering info
		if (SUCCEEDED(pICompiland->findChildren(SymTagFunction, NULL, nsNone, &pIEnumFunc)))
		{
			CComPtr<IDiaSymbol> pIFunc;
			ULONG celt = 0;
			while (SUCCEEDED(pIEnumFunc->Next(1, &pIFunc, &celt)) && (celt == 1))
			{
				PrintLines(pISession, pIFunc);
				pIFunc = 0;
			}
			pIEnumFunc = 0;
		}
		pICompiland = 0;
	}
	pIEnumSyms = 0;
	putwchar(L'\n');
	return true;
}

////////////////////////////////////////////////////////////
// Dump all the line numbering information for a given RVA
// and a given range
//
bool DumpAllLines(IDiaSession* pISession, DWORD dwRVA, DWORD dwRange)
{
	// Retrieve and print the lines that corresponds to a specified RVA
	IDiaEnumLineNumbers *pLines;
	if (FAILED(pISession->findLinesByRVA(dwRVA, dwRange, &pLines)))
		return false;
	PrintLines(pLines);
	pLines->Release();
	putwchar(L'\n');
	return true;
}

////////////////////////////////////////////////////////////
// Dump all the section contributions from the PDB
//
//  Section contributions are stored in a table which will
//  be retrieved via IDiaSession->getEnumTables through
//  QueryInterface()using the REFIID of the IDiaEnumSectionContribs
//
bool DumpAllSecContribs(IDiaSession* pISession)
{
	wprintf(L"\n\n*** SECTION CONTRIBUTION\n\n");
	CComPtr<IDiaEnumSectionContribs> pIEnumSectCtrb;
	if (FAILED(GetTable(pISession, __uuidof(IDiaEnumSectionContribs), (PVOID*)&pIEnumSectCtrb)))
		return false;
	wprintf(L"    RVA        Address       Size    Module\n");

	ULONG celt = 0;
	CComPtr<IDiaSectionContrib> pISectCtrb;
	while (SUCCEEDED(pIEnumSectCtrb->Next(1, &pISectCtrb, &celt)) && (celt == 1))
	{
		PrintSecContribs(pISession, pISectCtrb);
		pISectCtrb = 0;
	}
	pIEnumSectCtrb = 0;
	putwchar(L'\n');

	return true;
}

////////////////////////////////////////////////////////////
// Dump all debug data streams contained in the PDB
//
bool DumpAllDebugStreams(IDiaSession* pISession)
{
	wprintf(L"\n\n*** DEBUG STREAMS\n\n");

	// Retrieve an enumerated sequence of debug data streams
	CComPtr<IDiaEnumDebugStreams> pIEnumStream;
	if (FAILED(pISession->getEnumDebugStreams(&pIEnumStream)))
		return false;

	ULONG celt = 0;
	CComPtr<IDiaEnumDebugStreamData> pIStream;
	for (; SUCCEEDED(pIEnumStream->Next(1, &pIStream, &celt)) && (celt == 1); pIStream = NULL)
	{
		PrintStreamData(pIStream);
		pIStream = 0;
	}
	pIEnumStream = 0;
	putwchar(L'\n');

	return true;
}

////////////////////////////////////////////////////////////
// Dump all the injected source from the PDB
//
//  Injected sources data is stored in a table which will
//  be retrieved via IDiaSession->getEnumTables through
//  QueryInterface()using the REFIID of the IDiaEnumSectionContribs
//
bool DumpAllInjectedSources(IDiaSession* pISession)
{
	wprintf(L"\n\n*** INJECTED SOURCES TABLE\n\n");

	CComPtr<IDiaEnumInjectedSources> pIEnumInjSrc;
	if (SUCCEEDED(GetTable(pISession, __uuidof(IDiaEnumInjectedSources), (PVOID*)&pIEnumInjSrc)))
		return false;

	ULONG celt = 0;
	CComPtr<IDiaInjectedSource> pIInjSrc;
	while (SUCCEEDED(pIEnumInjSrc->Next(1, &pIInjSrc, &celt)) && (celt == 1))
	{
		PrintGeneric(pIInjSrc);
		pIInjSrc = 0;
	}
	pIEnumInjSrc = 0;
	putwchar(L'\n');

	return true;
}

////////////////////////////////////////////////////////////
// Dump info corresponing to a given injected source filename
//
bool DumpInjectedSource(IDiaSession* pISession, PCWSTR pszName)
{
	// Retrieve a source that has been placed into the symbol store by attribute providers or
	//  other components of the compilation process
	CComPtr<IDiaEnumInjectedSources> pIEnumInjSrc;
	if (FAILED(pISession->findInjectedSource(pszName, &pIEnumInjSrc)))
	{
		wprintf(L"ERROR - DumpInjectedSources() could not find %s\n", pszName);
		return false;
	}

	ULONG celt = 0;
	CComPtr<IDiaInjectedSource> pIInjSrc;
	while (SUCCEEDED(pIEnumInjSrc->Next(1, &pIInjSrc, &celt)) && (celt == 1))
	{
		PrintGeneric(pIInjSrc);
		pIInjSrc = 0;
	}
	pIEnumInjSrc = 0;

	return true;
}

////////////////////////////////////////////////////////////
// Dump all the source file information stored in the PDB
// We have to go through every compiland in order to retrieve
//   all the information otherwise checksums for instance
//   will not be available
// Compilands can have multiple source files with the same
//   name but different content which produces diffrent
//   checksums
//
bool DumpAllSourceFiles(IDiaSession* pISession, IDiaSymbol* pIPEScope)
{
	wprintf(L"\n\n*** SOURCE FILES\n\n");

	// To get the complete source file info we must go through the compiland first
	// by passing NULL instead all the source file names only will be retrieved
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (FAILED(pIPEScope->findChildren(SymTagCompiland, NULL, nsNone, &pIEnumSyms)))
		return false;

	CComPtr<IDiaSymbol> pICompiland;
	ULONG celt = 0;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pICompiland, &celt)) && (celt == 1))
	{
		CComBSTR bszName;
		if (pICompiland->get_name(&bszName) == S_OK)
			wprintf(L"\nCompiland = %s\n\n", bszName);

		// Every compiland could contain multiple references to the source files which were used to build it
		// Retrieve all source files by compiland by passing NULL for the name of the source file
		CComPtr<IDiaEnumSourceFiles> pIEnumSrcFile;
		if (SUCCEEDED(pISession->findFile(pICompiland, NULL, nsNone, &pIEnumSrcFile)))
		{
			CComPtr<IDiaSourceFile> pISrcFile;
			while (SUCCEEDED(pIEnumSrcFile->Next(1, &pISrcFile, &celt)) && (celt == 1))
			{
				PrintSourceFile(pISrcFile);
				putwchar(L'\n');
				pISrcFile = 0;
			}
			pIEnumSrcFile = 0;
		}
		putwchar(L'\n');
		pICompiland = 0;
	}
	pIEnumSyms = 0;

	return true;
}

////////////////////////////////////////////////////////////
// Dump all the FPO info
//
//  FPO data stored in a table which will be retrieved via
//    IDiaSession->getEnumTables through QueryInterface()
//    using the REFIID of the IDiaEnumFrameData
//
bool DumpAllFPO(IDiaSession* pISession)
{
	wprintf(L"\n\n*** FPO\n\n");

	CComPtr<IDiaEnumFrameData> pIEnumFrameData;
	if (FAILED(GetTable(pISession, __uuidof(IDiaEnumFrameData), (PVOID*)&pIEnumFrameData)))
		return false;

	ULONG celt = 0;
	CComPtr<IDiaFrameData> pIFrameData;
	while (SUCCEEDED(pIEnumFrameData->Next(1, &pIFrameData, &celt)) && (celt == 1))
	{
		PrintFrameData(pIFrameData);
		pIFrameData = 0;
	}
	pIEnumFrameData = 0;
	putwchar(L'\n');

	return true;
}

////////////////////////////////////////////////////////////
// Dump FPO info for a function at the specified RVA
//
bool DumpFPO(IDiaSession* pISession, DWORD dwRVA)
{
	CComPtr<IDiaEnumFrameData> pIEnumFrameData;
	// Retrieve first the table holding all the FPO info
	if ((dwRVA != 0) && SUCCEEDED(GetTable(pISession, __uuidof(IDiaEnumFrameData), (PVOID*)&pIEnumFrameData)))
	{
		CComPtr<IDiaFrameData> pIFrameData;
		// Retrieve the frame data corresponding to the given RVA
		if (SUCCEEDED(pIEnumFrameData->frameByRVA(dwRVA, &pIFrameData)))
		{
			PrintGeneric(pIFrameData);
			pIFrameData = 0;
		}
		else
		{
			// Some function might not have FPO data available (see ASM funcs like strcpy)
			wprintf(L"ERROR - DumpFPO() frameByRVA invalid RVA: 0x%08X\n", dwRVA);
			pIEnumFrameData = 0;
			return false;
		}
		pIEnumFrameData = 0;
	}
	else 
	{
		wprintf(L"ERROR - DumpFPO() GetTable\n");
		return false;
	}
	putwchar(L'\n');

	return true;
}

////////////////////////////////////////////////////////////
// Dump FPO info for a specified function symbol using its
//  name (a regular expression string is used for the search)
//
bool DumpFPO(IDiaSession* pISession, IDiaSymbol* pIPEScope, PCWSTR szSymbolName)
{
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	CComPtr<IDiaSymbol> pISymbol;
	ULONG celt = 0;
	DWORD dwRVA;

	// Find first all the function symbols that their names matches the search criteria
	if (FAILED(pIPEScope->findChildren(SymTagFunction, szSymbolName, nsRegularExpression, &pIEnumSyms)))
	{
		wprintf(L"ERROR - DumpFPO() findChildren could not find symol %s\n", szSymbolName);
		return false;
	}

	while (SUCCEEDED(pIEnumSyms->Next(1, &pISymbol, &celt)) && (celt == 1))
	{
		if (pISymbol->get_relativeVirtualAddress(&dwRVA) == S_OK)
		{
			PrintPublicSymbol(pISymbol);
			DumpFPO(pISession, dwRVA);
		}
		pISymbol = 0;
	}
	pIEnumSyms = 0;
	putwchar(L'\n');

	return true;
}

////////////////////////////////////////////////////////////
// Dump a specified compiland and all the symbols defined in it
//
bool DumpCompiland(IDiaSymbol* pIPEScope, PCWSTR pszCompName)
{
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (FAILED(pIPEScope->findChildren(SymTagCompiland, pszCompName, nsCaseInRegularExpression, &pIEnumSyms)))
		return false;

	ULONG celt = 0;
	CComPtr<IDiaSymbol> pICompiland;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pICompiland, &celt)) && (celt == 1))
	{
		wprintf(L"\n** Module: ");

		CComBSTR bszName;
		if (pICompiland->get_name(&bszName) != S_OK)
			wprintf(L"(???)\n");
		else
			wprintf(L"%s\n", bszName);

		CComBSTR bszLibName;
		if (pICompiland->get_libraryName(&bszLibName) == S_OK)
		{
			if (wcscmp(bszName, bszLibName) != 0)
				wprintf(L"   LibraryName: %s\n", bszLibName);
		}

		CComPtr<IDiaEnumSymbols> pIEnumChild;
		if (SUCCEEDED(pICompiland->findChildren(SymTagNull, NULL, nsNone, &pIEnumChild)))
		{
			ULONG celt_ = 0;
			CComPtr<IDiaSymbol> pISymbol;
			while (SUCCEEDED(pIEnumChild->Next(1, &pISymbol, &celt_)) && (celt_ == 1))
			{
				PrintSymbol(pISymbol, 0);
				pISymbol = 0;
			}
			pIEnumChild = 0;
		}
		pICompiland = 0;
	}
	pIEnumSyms = 0;

	return true;
}

////////////////////////////////////////////////////////////
// Dump the line numbering information for a specified RVA
//
bool DumpLines(IDiaSession* pISession, DWORD dwRVA)
{
	CComPtr<IDiaEnumLineNumbers> pIEnumLine;
	if (FAILED(pISession->findLinesByRVA(dwRVA, MAX_RVA_LINES_BYTES_RANGE, &pIEnumLine)))
		return false;
	PrintLines(pIEnumLine);
	pIEnumLine = 0;
	return true;
}

////////////////////////////////////////////////////////////
// Dump the all line numbering information for a specified
//  function symbol name (as a regular expression string)
//
bool DumpLines(IDiaSession* pISession, IDiaSymbol* pIPEScope, PCWSTR pszFuncName)
{
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (FAILED(pIPEScope->findChildren(SymTagFunction, pszFuncName, nsRegularExpression, &pIEnumSyms)))
		return false;

	ULONG celt = 0;
	CComPtr<IDiaSymbol> pIFunc;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pIFunc, &celt)) && (celt == 1))
	{
		PrintLines(pISession, pIFunc);
		pIFunc = 0;
	}
	pIEnumSyms = 0;
	return true;
}

////////////////////////////////////////////////////////////
// Dump the symbol information corresponding to a specified RVA
//
bool DumpSymbolWithRVA(IDiaSession* pISession, DWORD dwRVA, PCWSTR pszChildname)
{
	LONG lDisplacement;
	CComPtr<IDiaSymbol> pISymbol;
	if (FAILED(pISession->findSymbolByRVAEx(dwRVA, SymTagNull, &pISymbol, &lDisplacement)))
		return false;

	wprintf(L"Displacement = 0x%X\n", lDisplacement);
	PrintGeneric(pISymbol);

	bool bReturn = DumpSymbolWithChildren(pISymbol, pszChildname);
	while (pISymbol != NULL)
	{
		CComPtr<IDiaSymbol> pIParent;
		if ((pISymbol->get_lexicalParent(&pIParent) == S_OK) && pIParent)
		{
			wprintf(L"\nParent\n");

			PrintSymbol(pIParent, 0);
			pISymbol = 0;
			pISymbol = pIParent;
		}
		else
		{
			pISymbol = 0;
			break;
		}
	}
	return true;
}

////////////////////////////////////////////////////////////
// Dump the symbols information where their names matches a
//  specified regular expression string
//
bool DumpSymbolsWithRegEx(IDiaSymbol* pIPEScope, PCWSTR pszRegEx, PCWSTR pszChildname)
{
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (FAILED(pIPEScope->findChildren(SymTagNull, pszRegEx, nsRegularExpression, &pIEnumSyms)))
		return false;

	bool bReturn = true;
	ULONG celt = 0;
	CComPtr<IDiaSymbol> pISymbol;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pISymbol, &celt)) && (celt == 1))
	{
		PrintGeneric(pISymbol);
		bReturn = DumpSymbolWithChildren(pISymbol, pszChildname);
		pISymbol = 0;
	}
	pIEnumSyms = 0;
	return bReturn;
}

////////////////////////////////////////////////////////////
// Dump the information corresponding to a symbol name which
//  is a children of the specified parent symbol
//
bool DumpSymbolWithChildren(IDiaSymbol* pISymbol, PCWSTR pszChildname)
{
	if (pszChildname != NULL)
	{
		CComPtr<IDiaEnumSymbols> pIEnumSyms;
		if (FAILED(pISymbol->findChildren(SymTagNull, pszChildname, nsRegularExpression, &pIEnumSyms)))
			return false;

		DWORD celt = 1;
		CComPtr<IDiaSymbol> pIChild;
		while (SUCCEEDED(pIEnumSyms->Next(celt, &pIChild, &celt)) && (celt == 1))
		{
			PrintGeneric(pIChild);
			PrintSymbol(pIChild, 0);
			pIChild = 0;
		}
		pIEnumSyms = 0;
	}
	else
	{
		// If the specified name is NULL then only the parent symbol data is displayed
		DWORD dwSymTag;
		if ((pISymbol->get_symTag(&dwSymTag) == S_OK) && (dwSymTag == SymTagPublicSymbol))
			PrintPublicSymbol(pISymbol);
		else
			PrintSymbol(pISymbol, 0);
	}
	return true;
}

////////////////////////////////////////////////////////////
// Dump all the type symbols information that matches their
//  names to a specified regular expression string
//
bool DumpType(IDiaSymbol* pIPEScope, PCWSTR pszRegEx)
{
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	if (FAILED(pIPEScope->findChildren(SymTagUDT, pszRegEx, nsRegularExpression, &pIEnumSyms)))
		return false;

	ULONG celt = 0;
	CComPtr<IDiaSymbol> pISymbol;
	while (SUCCEEDED(pIEnumSyms->Next(1, &pISymbol, &celt)) && (celt == 1)) 
	{
		PrintTypeInDetail(pISymbol, 0);
		pISymbol = 0;
	}
	pIEnumSyms = 0;
	return true;
}

////////////////////////////////////////////////////////////
// Dump line numbering information for a given file name and
//  an optional line number
//
bool DumpLinesForSourceFile(IDiaSession* pISession, PCWSTR pszFileName, DWORD dwLine)
{
	CComPtr<IDiaEnumSourceFiles> pIEnumSrcFile;
	if (FAILED(pISession->findFile(NULL, pszFileName, nsFNameExt, &pIEnumSrcFile)))
		return false;

	CComPtr<IDiaSourceFile> pISrcFile;
	ULONG celt = 0;
	while (SUCCEEDED(pIEnumSrcFile->Next(1, &pISrcFile, &celt)) && (celt == 1))
	{
		CComPtr<IDiaEnumSymbols> pIEnumCompiland;
		if (pISrcFile->get_compilands(&pIEnumCompiland) == S_OK)
		{
			CComPtr<IDiaSymbol> pICompiland;

			celt = 0;
			while (SUCCEEDED(pIEnumCompiland->Next(1, &pICompiland, &celt)) && (celt == 1))
			{
				CComBSTR bszName;
				if (pICompiland->get_name(&bszName) == S_OK) 
					wprintf(L"Compiland = %s\n", bszName);
				else
					wprintf(L"Compiland = (???)\n");

				CComPtr<IDiaEnumLineNumbers> pIEnumLine;
				if (dwLine != 0)
				{
					if (SUCCEEDED(pISession->findLinesByLinenum(pICompiland, pISrcFile, dwLine, 0, &pIEnumLine)))
					{
						PrintLines(pIEnumLine);
						pIEnumLine = 0;
					}
				}
				else 
				{
					if (SUCCEEDED(pISession->findLines(pICompiland, pISrcFile, &pIEnumLine)))
					{
						PrintLines(pIEnumLine);
						pIEnumLine = 0;
					}
				}
				pICompiland = 0;
			}
			pIEnumCompiland = 0;
		}
		pISrcFile = 0;
	}
	pIEnumSrcFile = 0;

  return true;
}

////////////////////////////////////////////////////////////
// Dump public symbol information for a given number of
//  symbols around a given RVA address
//
bool DumpPublicSymbolsSorted(IDiaSession* pISession, DWORD dwRVA, DWORD dwRange, bool bReverse)
{
	CComPtr<IDiaEnumSymbolsByAddr> pIEnumSymsByAddr;
	if (FAILED(pISession->getSymbolsByAddr(&pIEnumSymsByAddr)))
		return false;

	CComPtr<IDiaSymbol> pISymbol;
	if (SUCCEEDED(pIEnumSymsByAddr->symbolByRVA(dwRVA, &pISymbol)))
	{
		if (dwRange == 0)
			PrintPublicSymbol(pISymbol);

		ULONG celt;
		if (bReverse)
		{
			pISymbol = 0;

			ULONG i = 0;
			for (pISymbol = NULL; (i < dwRange) && 
				 SUCCEEDED(pIEnumSymsByAddr->Next(1, &pISymbol, &celt)) && (celt == 1); i++)
			{
				PrintPublicSymbol(pISymbol);
				pISymbol = 0;
			}
		}
		else
		{
			PrintPublicSymbol(pISymbol);
			pISymbol = 0;

			ULONG i = 1;
			for (pISymbol = NULL; (i < dwRange) && 
				 SUCCEEDED(pIEnumSymsByAddr->Prev(1, &pISymbol, &celt)) && (celt == 1); i++)
				PrintPublicSymbol(pISymbol);
		}
	}
	pIEnumSymsByAddr = 0;

	return true;
}

////////////////////////////////////////////////////////////
// Dump label symbol information at a given RVA
//
bool DumpLabel(IDiaSession* pISession, DWORD dwRVA)
{
	LONG lDisplacement;
	CComPtr<IDiaSymbol> pISymbol;
	if (FAILED(pISession->findSymbolByRVAEx(dwRVA, 
		SymTagLabel, &pISymbol, &lDisplacement)) || (pISymbol == NULL))
		return false;

	wprintf(L"Displacement = 0x%X\n", lDisplacement);
	PrintGeneric(pISymbol);
	pISymbol = 0;
	return true;
}

////////////////////////////////////////////////////////////
// Dump annotation symbol information at a given RVA
//
bool DumpAnnotations(IDiaSession* pISession, DWORD dwRVA)
{
	LONG lDisplacement;
	CComPtr<IDiaSymbol> pISymbol;
	if (FAILED(pISession->findSymbolByRVAEx(dwRVA, 
		SymTagAnnotation, &pISymbol, &lDisplacement)) || (pISymbol == NULL))
		return false;

	wprintf(L"Displacement = 0x%X\n", lDisplacement);
	PrintGeneric(pISymbol);
	pISymbol = 0;
	return true;
}

struct OMAP_DATA
{
	DWORD dwRVA;
	DWORD dwRVATo;
};

////////////////////////////////////////////////////////////
//
bool DumpMapToSrc(IDiaSession* pISession, DWORD dwRVA)
{
	CComPtr<IDiaEnumDebugStreams> pIEnumStream;
	if (FAILED(pISession->getEnumDebugStreams(&pIEnumStream)))
		return false;

	ULONG celt = 0;
	CComPtr<IDiaEnumDebugStreamData> pIStream;
	for (; SUCCEEDED(pIEnumStream->Next(1, &pIStream, &celt)) && (celt == 1); pIStream = NULL)
	{
		CComBSTR bszName;
		pIStream->get_name(&bszName);
		if (bszName.m_str && wcscmp(bszName, L"OMAPTO") == 0)
		{
			OMAP_DATA data, datasav;
			DWORD cbData, celt;
			DWORD dwRVATo = 0;

			datasav.dwRVATo = 0;
			datasav.dwRVA = 0;
			while (SUCCEEDED(pIStream->Next(1, sizeof(data), &cbData, (PBYTE)&data, &celt)) && (celt == 1))
			{
				if (dwRVA > data.dwRVA)
				{
					datasav = data;
					continue;
				}
				else if (dwRVA == data.dwRVA)
					dwRVATo = data.dwRVATo;
				else if (datasav.dwRVATo)
					dwRVATo = datasav.dwRVATo + (dwRVA - datasav.dwRVA);
				break;
			}
			wprintf(L"image rva = %08X ==> source rva = %08X\n\nRelated OMAP entries:\n", dwRVA, dwRVATo);
			wprintf(L"image rva ==> source rva\n");
			wprintf(L"%08X  ==> %08X\n", datasav.dwRVA, datasav.dwRVATo);

			int i = 0;
			do
			{
				wprintf(L"%08X  ==> %08X\n", data.dwRVA, data.dwRVATo);
			}
			while ((++i) < 5 && SUCCEEDED(pIStream->Next(1, 
					sizeof(data), &cbData, (PBYTE)&data, &celt)) && (celt == 1));
		}
		pIStream = 0;
	}
	pIEnumStream = 0;

	return true;
}

////////////////////////////////////////////////////////////
//
bool DumpMapFromSrc(IDiaSession* pISession, DWORD dwRVA)
{
	CComPtr<IDiaEnumDebugStreams> pIEnumStream;
	if (FAILED(pISession->getEnumDebugStreams(&pIEnumStream)))
		return false;

	ULONG celt = 0;
	CComPtr<IDiaEnumDebugStreamData> pIStream;
	for (; SUCCEEDED(pIEnumStream->Next(1, &pIStream, &celt)) && (celt == 1); pIStream = NULL)
	{
		CComBSTR bszName;
		pIStream->get_name(&bszName);
		if (bszName.m_str && wcscmp(bszName, L"OMAPFROM") == 0)
		{
			OMAP_DATA data, datasav;
			DWORD cbData, celt, dwRVATo = 0;

			datasav.dwRVATo = 0;
			datasav.dwRVA = 0;
			while (SUCCEEDED(pIStream->Next(1, sizeof(data), &cbData, (PBYTE) &data, &celt)) && (celt == 1))
			{
				if (dwRVA > data.dwRVA)
				{
					datasav = data;
					continue;
				}
				else if (dwRVA == data.dwRVA)
					dwRVATo = data.dwRVATo;
				else if (datasav.dwRVATo)
					dwRVATo = datasav.dwRVATo + (dwRVA - datasav.dwRVA);
				break;
			}
			wprintf(L"source rva = %08X ==> image rva = %08X\n\nRelated OMAP entries:\n", dwRVA, dwRVATo);
			wprintf(L"source rva ==> image rva\n");
			wprintf(L"%08X  ==> %08X\n", datasav.dwRVA, datasav.dwRVATo);

			unsigned int i = 0;
			do
			{
				wprintf(L"%08X  ==> %08X\n", data.dwRVA, data.dwRVATo);
			}
			while ((++i) < 5 && SUCCEEDED(pIStream->Next(1, sizeof(data),
				&cbData, (PBYTE) &data, &celt)) && (celt == 1));
		}
		pIStream = 0;
	}
	pIEnumStream = 0;

	return true;
}

////////////////////////////////////////////////////////////
// Retreive the table that matches the given iid
//
//  A PDB table could store the section contributions, the frame data,
//  the injected sources
//
HRESULT GetTable(IDiaSession* pISession, REFIID iid, PVOID* ppIUnk)
{
	CComPtr<IDiaEnumTables> pIEnumTable;
	if (FAILED(pISession->getEnumTables(&pIEnumTable)))
	{
		wprintf(L"ERROR - GetTable() getEnumTables\n");
		return E_FAIL;
	}

	ULONG celt = 0;
	CComPtr<IDiaTable> pITable;
	while (SUCCEEDED(pIEnumTable->Next(1, &pITable, &celt)) && (celt == 1))
	{
		// There's only one table that matches the given IID
		if (SUCCEEDED(pITable->QueryInterface(iid, (PVOID*)ppIUnk)))
		{
			pITable = 0;
			pIEnumTable = 0;
			return S_OK;
		}
		pITable = 0;
	}
	pIEnumTable = 0;

	return E_FAIL;
}

void PrintSymbolsByAddress(IDiaSession* pISession)
{
	CComPtr<IDiaEnumSymbolsByAddr> pIEnumByAddr;
	if (FAILED(pISession->getSymbolsByAddr(&pIEnumByAddr)))
	{
		wprintf(L"ERROR - getSymbolsByAddr\n");
		return;
	}

	CComPtr<IDiaSymbol> pISymbol;
	if (FAILED(pIEnumByAddr->symbolByAddr(1, 0, &pISymbol)))
	{
		wprintf(L"ERROR - symbolByAddr\n");
		return;
	}

	ULONG celt = 0;
	DWORD rvaLast = 0;
	if (pISymbol->get_relativeVirtualAddress(&rvaLast) == S_OK)
	{
		pISymbol = 0;
		if (FAILED(pIEnumByAddr->symbolByRVA(rvaLast, &pISymbol)))
		{
			wprintf(L"ERROR - symbolByAddr\n");
			return;
		}

		printf("Symbols in order\n");
		do
		{
			CComBSTR bszName;
			if (pISymbol->get_name(&bszName) != S_OK)
			{
				printf("\t0x%08X (%ws) <no name>\n", rvaLast);
			}
			else
			{
				printf("\t0x%08X %ws\n", rvaLast, bszName);
			}
			pISymbol = 0;
			
			celt = 0;
			if (FAILED(pIEnumByAddr->Next(1, &pISymbol, &celt)))
			{
				break;
			}
		} while (celt == 1);
	}
	pIEnumByAddr = 0;
}