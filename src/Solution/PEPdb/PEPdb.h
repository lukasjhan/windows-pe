#pragma once
#include <dia2.h>

#define GSZ_CRLF L"\xd\xa"
typedef enum SymTagEnum DIA_SYMTAG;

class PEPdb
{
public:
	static PCWSTR GSZ_LOC_TYPES[];
	static PCWSTR GSZ_BASE_TYPES[];
	static PCWSTR GSZ_SYM_TAGS[];
	static PCWSTR GSZ_DATA_KINDS[];
	static PCWSTR GSZ_UDT_KINDS[];
	static PCWSTR GSZ_ACCESS_SPECS[];
	static PCWSTR GSZ_CALL_CONVS[];
	static PCWSTR GSZ_LANG_SPECS[];
	static PCWSTR GSZ_CPU_TYPES[];
	static PCWSTR GSZ_FLOAT_PACKS[];
	static PCWSTR GSZ_THUNK_ORDS[];

	CString					PDB_PATH;
	CComPtr<IDiaSession>	SESSION;
	CComPtr<IDiaDataSource>	SOURCE;
	CComPtr<IDiaSymbol>		PESCOPE;
	DWORD					MACHINE_TYPE;

public:
	PEPdb()
	{
	}
	PEPdb(PIMAGE_PDB_INFO ppi, DWORD dwTimestamp, PCWSTR pszSymPath = NULL)
	{
		HRESULT hr = LoadPdbFile(ppi, dwTimestamp, pszSymPath);
		if (FAILED(hr))
			throw hr;
	}
	~PEPdb()
	{
		if (PESCOPE)	PESCOPE = 0;
		if (SOURCE)		SOURCE  = 0;
		if (SESSION)	SESSION = 0;
	}

	bool Loaded() { return !PDB_PATH.IsEmpty(); }
	HRESULT LoadPdbFile(PIMAGE_PDB_INFO ppi, DWORD dwTimestamp, PCWSTR pszSymPath=NULL);

	static DWORD GetPlatormId(IDiaSymbol* pISymbol);

	static PCWSTR ToStr_C7REG(USHORT uReg, DWORD dwMachType);
	static String ToStr_Variant(LPVARIANT pv);
	static String ToStr_Name(IDiaSymbol* pISymbol);
	static String ToStr_Bound(IDiaSymbol* pISymbol);
	static String ToStr_Location(IDiaSymbol* pISymbol);
	static String ToStr_ArrayType(IDiaSymbol* pISymbol);
	static String ToStr_FunctionType(IDiaSymbol* pISymbol);
	static String ToStr_PointerType(IDiaSymbol* pISymbol);
	static String ToStr_BaseType(IDiaSymbol* pISymbol);
	static String ToStr_CustomType(IDiaSymbol* pISymbol);
	static String ToStr_Type(IDiaSymbol* pISymbol);
	static String ToStr_UndName(IDiaSymbol* pISymbol);
	static String ToStr_VTableSahpe(IDiaSymbol* pISymbol);

	static String ToStr_CompilandEnv(IDiaSymbol* pISymbol);
	static String ToStr_CompilandDetails(IDiaSymbol* pISymbol);
	static String ToStr_Thunk(IDiaSymbol* pISymbol);
	static String ToStr_CallSite(IDiaSymbol* pISymbol);
	static String ToStr_Symbol(IDiaSymbol* pISymbol, DWORD dwIndent);

	static String ToStr_Data(IDiaSymbol* pISymbol);
	static String ToStr_Function(IDiaSymbol* pISymbol);

};
