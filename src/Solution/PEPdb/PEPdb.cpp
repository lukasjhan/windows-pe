#include "stdafx.h"
#include "PEPdb.h"
#include "Shlwapi.h"


PCWSTR PEPdb::GSZ_LOC_TYPES[] =
{
	L"(null)",
	L"Static",
	L"TLS",
	L"RegRel",
	L"ThisRel",
	L"Enregistered",
	L"BitField",
	L"Slot",
	L"IL Relative",
	L"In MetaData",
	L"Constant"
};

// Basic types
PCWSTR PEPdb::GSZ_BASE_TYPES[] =
{
	L"<NoType>",			// btNoType = 0,
	L"void",				// btVoid = 1,
	L"char",				// btChar = 2,
	L"wchar_t",				// btWChar = 3,
	L"signed char",
	L"unsigned char",
	L"int",					// btInt = 6,
	L"unsigned int",		// btUInt = 7,
	L"float",				// btFloat = 8,
	L"<BCD>",				// btBCD = 9,
	L"bool",				// btBool = 10,
	L"short",
	L"unsigned short",
	L"long",				// btLong = 13,
	L"unsigned long",		// btULong = 14,
	L"__int8",
	L"__int16",
	L"__int32",
	L"__int64",
	L"__int128",
	L"unsigned __int8",
	L"unsigned __int16",
	L"unsigned __int32",
	L"unsigned __int64",
	L"unsigned __int128",
	L"<currency>",			// btCurrency = 25,
	L"<date>",				// btDate = 26,
	L"VARIANT",				// btVariant = 27,
	L"<complex>",			// btComplex = 28,
	L"<bit>",				// btBit = 29,
	L"BSTR",				// btBSTR = 30,
	L"HRESULT"				// btHresult = 31
};

// Tags returned by Dia
PCWSTR PEPdb::GSZ_SYM_TAGS[] =
{
	L"(SymTagNull)",			// SymTagNull
	L"Executable",				// SymTagExe
	L"Compiland",				// SymTagCompiland
	L"CompilandDetails",		// SymTagCompilandDetails
	L"CompilandEnv",			// SymTagCompilandEnv
	L"Function",				// SymTagFunction
	L"Block",					// SymTagBlock
	L"Data",					// SymTagData
	L"Annotation",				// SymTagAnnotation
	L"Label",					// SymTagLabel
	L"PublicSymbol",			// SymTagPublicSymbol
	L"UDT",						// SymTagUDT
	L"Enum",					// SymTagEnum
	L"FunctionType",			// SymTagFunctionType
	L"PointerType",				// SymTagPointerType
	L"ArrayType",				// SymTagArrayType
	L"BaseType",				// SymTagBaseType
	L"Typedef",					// SymTagTypedef
	L"BaseClass",				// SymTagBaseClass
	L"Friend",					// SymTagFriend
	L"FunctionArgType",			// SymTagFunctionArgType
	L"FuncDebugStart",			// SymTagFuncDebugStart
	L"FuncDebugEnd",			// SymTagFuncDebugEnd
	L"UsingNamespace",			// SymTagUsingNamespace
	L"VTableShape",				// SymTagVTableShape
	L"VTable",					// SymTagVTable
	L"Custom",					// SymTagCustom
	L"Thunk",					// SymTagThunk
	L"CustomType",				// SymTagCustomType
	L"ManagedType",				// SymTagManagedType
	L"Dimension",				// SymTagDimension
	L"CallSite",				// SymTagCallSite
	L"InlineSite",				// SymTagInlineSite
	L"BaseInterface",			// SymTagBaseInterface
	L"VectorType",				// SymTagVectorType
	L"MatrixType",				// SymTagMatrixType
	L"HLSLType",				// SymTagHLSLType
	L"Caller",					// SymTagCaller,
	L"Callee"					// SymTagCallee,
};

// Processors
PCWSTR PEPdb::GSZ_FLOAT_PACKS[] =
{
	L"hardware processor (80x87 for Intel processors)",    // CV_CFL_NDP
	L"emulator",                                           // CV_CFL_EMU
	L"altmath",                                            // CV_CFL_ALT
	L"???"
};

//CV_CPU_TYPE_e
PCWSTR PEPdb::GSZ_CPU_TYPES[] = 
{
	L"8080",					//  CV_CFL_8080
	L"8086",					//  CV_CFL_8086
	L"80286",					//  CV_CFL_80286
	L"80386",					//  CV_CFL_80386
	L"80486",					//  CV_CFL_80486
	L"Pentium",					//  CV_CFL_PENTIUM
	L"Pentium Pro/Pentium II",	//  CV_CFL_PENTIUMII/CV_CFL_PENTIUMPRO
	L"Pentium III",				//  CV_CFL_PENTIUMIII
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"MIPS(Generic)",			//  CV_CFL_MIPSR4000
	L"MIPS16",					//  CV_CFL_MIPS16
	L"MIPS32",					//  CV_CFL_MIPS32
	L"MIPS64",					//  CV_CFL_MIPS64
	L"MIPS I",					//  CV_CFL_MIPSI
	L"MIPS II",					//  CV_CFL_MIPSII
	L"MIPS III",				//  CV_CFL_MIPSIII
	L"MIPS IV",					//  CV_CFL_MIPSIV
	L"MIPS V",					//  CV_CFL_MIPSV
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"M68000",					//  CV_CFL_M68000
	L"M68010",					//  CV_CFL_M68010
	L"M68020",					//  CV_CFL_M68020
	L"M68030",					//  CV_CFL_M68030
	L"M68040",					//  CV_CFL_M68040
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"Alpha 21064",				// CV_CFL_ALPHA, CV_CFL_ALPHA_21064
	L"Alpha 21164",				// CV_CFL_ALPHA_21164
	L"Alpha 21164A",			// CV_CFL_ALPHA_21164A
	L"Alpha 21264",				// CV_CFL_ALPHA_21264
	L"Alpha 21364",				// CV_CFL_ALPHA_21364
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"PPC 601",					// CV_CFL_PPC601
	L"PPC 603",					// CV_CFL_PPC603
	L"PPC 604",					// CV_CFL_PPC604
	L"PPC 620",					// CV_CFL_PPC620
	L"PPC w/FP",				// CV_CFL_PPCFP
	L"PPC (Big Endian)",		// CV_CFL_PPCBE
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"SH3",						// CV_CFL_SH3
	L"SH3E",					// CV_CFL_SH3E
	L"SH3DSP",					// CV_CFL_SH3DSP
	L"SH4",						// CV_CFL_SH4
	L"SHmedia",					// CV_CFL_SHMEDIA
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"ARM3",					// CV_CFL_ARM3
	L"ARM4",					// CV_CFL_ARM4
	L"ARM4T",					// CV_CFL_ARM4T
	L"ARM5",					// CV_CFL_ARM5
	L"ARM5T",					// CV_CFL_ARM5T
	L"ARM6",					// CV_CFL_ARM6
	L"ARM (XMAC)",				// CV_CFL_ARM_XMAC
	L"ARM (WMMX)",				// CV_CFL_ARM_WMMX
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"Omni",					// CV_CFL_OMNI
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"Itanium",					// CV_CFL_IA64, CV_CFL_IA64_1
	L"Itanium (McKinley)",		// CV_CFL_IA64_2
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"CEE",						// CV_CFL_CEE
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"AM33",					// CV_CFL_AM33
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"M32R",					// CV_CFL_M32R
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"TriCore",					// CV_CFL_TRICORE
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"AMD64",					// CV_CFL_X64
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"EBC",						// CV_CFL_EBC
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"Thumb (CE)",				// CV_CFL_THUMB
	L"???",
	L"???",
	L"???",
	L"ARM",						// CV_CFL_ARMNT
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"D3D11_SHADE",				// CV_CFL_D3D11_SHADER
};

PCWSTR PEPdb::GSZ_DATA_KINDS[] =
{
	L"Unknown",
	L"Local",
	L"Static Local",
	L"Param",
	L"Object Ptr",
	L"File Static",
	L"Global",
	L"Member",
	L"Static Member",
	L"Constant",
};

PCWSTR PEPdb::GSZ_UDT_KINDS[] =
{
	L"struct",
	L"class",
	L"union",
	L"interface",
};

PCWSTR PEPdb::GSZ_ACCESS_SPECS[] =
{
	L"",		// No access specifier
	L"private",
	L"protected",
	L"public"
};

PCWSTR PEPdb::GSZ_CALL_CONVS[] =
{
	L"_cdecl",
	L"far _cdecl",
	L"_pascal",
	L"far _pascal",
	L"_fastcall",
	L"far _fastcall",
	L"SKIPPED",
	L"_stdcall",
	L"far _stdcall",
	L"_syscall",
	L"far _syscall",
	L"_thiscall",
	L"MIPSCALL",
	L"GENERIC",
	L"ALPHACALL",
	L"PPCCALL",
	L"SHCALL",
	L"ARMCALL",
	L"AM33CALL",
	L"TRICALL",
	L"SH5CALL",
	L"M32RCALL",
	L"inline",
	L"_vector",
	L"_reserved"
};

PCWSTR PEPdb::GSZ_LANG_SPECS[] =
{
	L"C",                                // CV_CFL_C
	L"C++",                              // CV_CFL_CXX
	L"FORTRAN",                          // CV_CFL_FORTRAN
	L"MASM",                             // CV_CFL_MASM
	L"Pascal",                           // CV_CFL_PASCAL
	L"Basic",                            // CV_CFL_BASIC
	L"COBOL",                            // CV_CFL_COBOL
	L"LINK",                             // CV_CFL_LINK
	L"CVTRES",                           // CV_CFL_CVTRES
	L"CVTPGD",                           // CV_CFL_CVTPGD
	L"C#",                               // CV_CFL_CSHARP
	L"Visual Basic",                     // CV_CFL_VB
	L"ILASM",                            // CV_CFL_ILASM
	L"Java",                             // CV_CFL_JAVA
	L"JScript",                          // CV_CFL_JSCRIPT
	L"MSIL",                             // CV_CFL_MSIL
	L"HLSL",                             // CV_CFL_HLSL
};

PCWSTR PEPdb::GSZ_THUNK_ORDS[] =
{
	L"IMPORT",		// standard thunk
	L"THIS",		// "this" adjustor thunk
	L"VCALL",		// virtual call thunk
	L"PCODE",		// pcode thunk
	L"LOAD",		// thunk which loads the address to jump to
	L"INCREMENTAL",
	L"BRANCHISLAND"
};
////////////////////////////////////////////////////////////////////////////////////////////////////





HRESULT PEPdb::LoadPdbFile(PIMAGE_PDB_INFO ppi, DWORD dwTimestamp, PCWSTR pszSymPath)
{
	USES_CONVERSION;
	WCHAR szPath[MAX_PATH];
	PCSTR pszPdbFile = PCSTR(ppi->PdbFileName);
	if (PathIsFileSpecA(pszPdbFile))
	{
		if (pszSymPath == NULL)
			return E_INVALIDARG;

		GUID guid = *LPGUID(ppi->Guid);
		WCHAR szGuid[65];
		int nChLen = wsprintf(szGuid, L"%08X%04X%04X", guid.Data1, guid.Data2, guid.Data3);
		for (int j = 0; j < 8; j++)
			nChLen += wsprintf(szGuid + nChLen, L"%02X", guid.Data4[j]);
		wsprintf(szPath, L"%s\\%s\\%s%X\\%s", 
			pszSymPath, A2CW(pszPdbFile), szGuid, ppi->Age, A2CW(pszPdbFile));
	}
	else
		wsprintf(szPath, L"%s", A2CT(pszPdbFile));
	if (!PathFileExists(szPath))
		return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);

	HRESULT hr = S_OK;
	try
	{
		hr = CoCreateInstance
		(
			__uuidof(DiaSource), NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IDiaDataSource), (PVOID*)&SOURCE
		);
		if (FAILED(hr))
			throw hr;
		hr = SOURCE->loadAndValidateDataFromPdb(szPath,
						LPGUID(ppi->Guid), dwTimestamp, ppi->Age);
		if (FAILED(hr))
			throw hr;
		hr = SOURCE->openSession(&SESSION);
		if (FAILED(hr))
			throw hr;

		hr = SESSION->get_globalScope(&PESCOPE);
		if (hr != S_OK)
			throw hr;

		DWORD dwMachType = 0;
		if (PESCOPE->get_machineType(&dwMachType) == S_OK)
		{
			switch (dwMachType)
			{
			case IMAGE_FILE_MACHINE_I386: MACHINE_TYPE = CV_CFL_80386; break;
			case IMAGE_FILE_MACHINE_IA64: MACHINE_TYPE = CV_CFL_IA64; break;
			case IMAGE_FILE_MACHINE_AMD64: MACHINE_TYPE = CV_CFL_AMD64; break;
			}
		}
		PDB_PATH = szPath;
	}
	catch (HRESULT e)
	{
		if (SESSION) PESCOPE = 0;
		if (PESCOPE) SESSION = 0;
		if (SOURCE)	 SOURCE  = 0;
		hr = e;
	}
	return hr;
}

DWORD PEPdb::GetPlatormId(IDiaSymbol* pISymbol)
{
	CComPtr<IDiaSymbol> pIIter = pISymbol;
	for (;;)
	{
		DWORD dwTag = 0;
		pIIter->get_symTag(&dwTag);
		if (dwTag == SymTagExe)
		{
			DWORD dwMachine = 0, dwCpuId = 0;
			pIIter->get_machineType(&dwMachine);
			switch (dwMachine)
			{
			case IMAGE_FILE_MACHINE_AMD64: dwCpuId = CV_CFL_AMD64; break;
			case IMAGE_FILE_MACHINE_I386 : dwCpuId = CV_CFL_80386; break;
			case IMAGE_FILE_MACHINE_IA64 : dwCpuId = CV_CFL_IA64; break;
			}
			pIIter = 0;
			return dwCpuId;
		}

		CComPtr<IDiaSymbol> pILexP;
		if (pIIter->get_lexicalParent(&pILexP) != S_OK)
			break;
		pIIter = pILexP;
		pILexP = 0;
	}
	pIIter = 0;
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// STATIC FUNCTION DEFINITIONS...
//
String PEPdb::ToStr_Variant(LPVARIANT pv)
{
	String szVal;
	switch (pv->vt)
	{
	case VT_UI1: case VT_I1:
		szVal.Format(L" 0x%X", pv->bVal);
		break;
	case VT_I2: case VT_UI2: case VT_BOOL:
		szVal.Format(L" 0x%X", pv->iVal);
		break;
	case VT_I4: case VT_UI4:
	case VT_INT: case VT_UINT: case VT_ERROR:
		szVal.Format(L" 0x%X", pv->lVal);
		break;
	case VT_I8: case VT_UI8:
		szVal.Format(L" 0x%I64X", pv->lVal);
		break;
	case VT_R4:
		szVal.Format(L" %g", pv->fltVal);
		break;
	case VT_R8:
		szVal.Format(L" %g", pv->dblVal);
		break;
	case VT_BSTR:
		szVal.Format(L" \"%s\"", pv->bstrVal);
		break;
	default:
		szVal = L" ??";
		break;
	}
	return szVal;
}

String PEPdb::ToStr_Name(IDiaSymbol* pISymbol)
{
	CComBSTR bszName, bszUndName;
	if (pISymbol->get_name(&bszName) != S_OK)
		return String(L"(none)");

	String szOut;
	if (pISymbol->get_undecoratedName(&bszUndName) == S_OK)
	{
		if (wcscmp(bszName, bszUndName) == 0)
			szOut = bszName;
		else
			szOut.Format(L"%s [%s]", bszName, bszUndName);
	}
	else
		szOut = bszName;
	return szOut;
}

String PEPdb::ToStr_CompilandEnv(IDiaSymbol* pISymbol)
{
	String szEnv;
	szEnv = ToStr_Name(pISymbol);
	szEnv.Append(L" =");
	CComVariant vt;
	if (pISymbol->get_value(&vt) == S_OK)
		szEnv.Append(ToStr_Variant(&vt));
	return szEnv;
}

String PEPdb::ToStr_Bound(IDiaSymbol* pISymbol)
{
	DWORD dwTag = 0, dwKind;
	if (pISymbol->get_symTag(&dwTag) != S_OK)
		return L"Unknown Symbol.";

	if (pISymbol->get_locationType(&dwKind) != S_OK)
		throw L"Unknown Location.";

	String szOut;
	if (dwTag == SymTagData && dwKind == LocIsConstant)
	{
		CComVariant v;
		if (pISymbol->get_value(&v) == S_OK)
			szOut = ToStr_Variant(&v);
	}
	else
		szOut = ToStr_Name(pISymbol);
	return szOut;
}

String PEPdb::ToStr_Location(IDiaSymbol* pISymbol)
{
	DWORD dwLocType;
	if (pISymbol->get_locationType(&dwLocType) != S_OK)
		return L"Optmized Symbol.";

	DWORD dwRVA, dwSect, dwOff, dwReg, dwBitPos, dwSlot;
	LONG  lOffset;
	ULONGLONG ulLen;
	String	szLoc;
	switch (dwLocType)
	{
	case LocIsStatic:
		if ((pISymbol->get_relativeVirtualAddress(&dwRVA) == S_OK) &&
			(pISymbol->get_addressSection(&dwSect) == S_OK) &&
			(pISymbol->get_addressOffset(&dwOff) == S_OK))
			szLoc.AppendFormat(L"%s, [%08X][%04X:%08X]",
			GSZ_LOC_TYPES[dwLocType], dwRVA, dwSect, dwOff);
		break;
	case LocIsTLS:
	case LocInMetaData:
	case LocIsIlRel:
		if ((pISymbol->get_relativeVirtualAddress(&dwRVA) == S_OK) &&
			(pISymbol->get_addressSection(&dwSect) == S_OK) &&
			(pISymbol->get_addressOffset(&dwOff) == S_OK))
			szLoc.AppendFormat(L"%s, [%08X][%04X:%08X]",
			GSZ_LOC_TYPES[dwLocType], dwRVA, dwSect, dwOff);
		break;
	case LocIsRegRel:
		if ((pISymbol->get_registerId(&dwReg) == S_OK) &&
			(pISymbol->get_offset(&lOffset) == S_OK))
			szLoc.AppendFormat(L"%s, %s + %08X",
			GSZ_LOC_TYPES[dwLocType], ToStr_C7REG((USHORT)dwReg, GetPlatormId(pISymbol)), lOffset);
		break;
	case LocIsThisRel:
		if (pISymbol->get_offset(&lOffset) == S_OK)
			szLoc.AppendFormat(L"%s, this + 0x%X", GSZ_LOC_TYPES[dwLocType], lOffset);
		break;
	case LocIsBitField:
		if ((pISymbol->get_offset(&lOffset) == S_OK) &&
			(pISymbol->get_bitPosition(&dwBitPos) == S_OK) &&
			(pISymbol->get_length(&ulLen) == S_OK))
			szLoc.AppendFormat(L"%s, this(bf)+0x%X:0x%X len(0x%X)",
			GSZ_LOC_TYPES[dwLocType], lOffset, dwBitPos, ulLen);
		break;
	case LocIsEnregistered:
		if (pISymbol->get_registerId(&dwReg) == S_OK)
			szLoc.AppendFormat(L"%s, %s",
			GSZ_LOC_TYPES[dwLocType], ToStr_C7REG((USHORT)dwReg, GetPlatormId(pISymbol)));
		break;
	case LocIsSlot:
		if (pISymbol->get_slot(&dwSlot) == S_OK)
			szLoc.AppendFormat(L"%s, [%08X]", GSZ_LOC_TYPES[dwLocType], dwSlot);
		break;
	case LocIsConstant:
	{
		szLoc.Append(GSZ_LOC_TYPES[dwLocType]);
		CComVariant vt;
		if (pISymbol->get_value(&vt) == S_OK)
			szLoc.Append(ToStr_Variant(&vt));
	}
	break;
	case LocIsNull:
		szLoc.Append(GSZ_LOC_TYPES[dwLocType]);
	default:
		throw L"Unknown Location.";
	}
	return szLoc;
}

String PEPdb::ToStr_Data(IDiaSymbol* pISymbol)
{
	String szData = ToStr_Location(pISymbol);
	DWORD dwDataKind = 0;
	if (pISymbol->get_dataKind(&dwDataKind) == S_OK)
	{
		szData.AppendFormat(L", %s", GSZ_DATA_KINDS[dwDataKind]);
		CComPtr<IDiaSymbol> pIType;
		if (pISymbol->get_type(&pIType) == S_OK)
		{
			szData.AppendFormat(L", Type: %s", ToStr_Type(pIType));
			pIType = 0;
		}
		szData.AppendFormat(L", %s", ToStr_Name(pISymbol));
	}
	return szData;
}

String PEPdb::ToStr_ArrayType(IDiaSymbol* pISymbol)
{
	CComPtr<IDiaSymbol> pIBaseType;
	if (pISymbol->get_type(&pIBaseType) != S_OK)
		return String();
	String szOut = ToStr_Type(pIBaseType);

	ULONG uCelt = 0;
	LONG  lCount = 0;
	DWORD dwRank = 0;
	CComPtr<IDiaEnumSymbols> pIEnumChild;
	if (pISymbol->get_rank(&dwRank) == S_OK)	//Fortran
	{
		if (SUCCEEDED(pISymbol->findChildren
			(SymTagDimension, NULL, nsNone, &pIEnumChild)) && (pIEnumChild != NULL))
		{
			CComPtr<IDiaSymbol> pIChild;
			while (SUCCEEDED(pIEnumChild->Next(1, &pIChild, &uCelt)) && (uCelt == 1))
			{
				szOut.Append(L"[");
				CComPtr<IDiaSymbol> pIBound;
				if (pIChild->get_lowerBound(&pIBound) == S_OK)
				{
					szOut.Append(ToStr_Bound(pIBound));
					pIBound = 0;
					szOut.Append(L"..");
				}
				if (pIChild->get_upperBound(&pIBound) == S_OK)
				{
					szOut.Append(ToStr_Bound(pIBound));
					pIBound = 0;
				}
				pIChild = 0;
				szOut.Append(L"]");
			}
			pIEnumChild = 0;
		}
	}
	else if (SUCCEEDED(pISymbol->findChildren
		(SymTagCustomType, NULL, nsNone, &pIEnumChild)) && (pIEnumChild != NULL) &&
		(pIEnumChild->get_Count(&lCount) == S_OK) && (lCount > 0))
	{
		CComPtr<IDiaSymbol> pIChild;
		while (SUCCEEDED(pIEnumChild->Next(1, &pIChild, &uCelt)) && (uCelt == 1))
		{
			szOut.Append(L"[");
			szOut.Append(ToStr_Type(pIChild));
			szOut.Append(L"]");
			pIChild = 0;
		}
		pIEnumChild = 0;
	}
	else
	{
		DWORD dwCountElems;
		ULONGLONG ulLenArr, ulLenElem;
		if (pISymbol->get_count(&dwCountElems) == S_OK)
			szOut.AppendFormat(L"[%d]", dwCountElems);
		else if ((pISymbol->get_length(&ulLenArr) == S_OK) &&
			(pIBaseType->get_length(&ulLenElem) == S_OK))
		{
			if (ulLenElem == 0)
				szOut.AppendFormat(L"[%d]", ulLenArr);
			else
				szOut.AppendFormat(L"[%d]", ulLenArr / ulLenElem);
		}
	}
	pIBaseType = 0;
	return szOut;
}

String PEPdb::ToStr_PointerType(IDiaSymbol* pISymbol)
{
	CComPtr<IDiaSymbol> pIBase;
	if (pISymbol->get_type(&pIBase) != S_OK)
		return L"Unknown BaseType.";
	String szOut = ToStr_Type(pIBase);
	pIBase = 0;

	BOOL bSet;
	if ((pISymbol->get_reference(&bSet) == S_OK) && bSet)
		szOut.Append(L"&");
	else
		szOut.Append(L"*");

	if ((pISymbol->get_constType(&bSet) == S_OK) && bSet)
		szOut.Append(L" const");
	if ((pISymbol->get_volatileType(&bSet) == S_OK) && bSet)
		szOut.Append(L" volatile");
	if ((pISymbol->get_unalignedType(&bSet) == S_OK) && bSet)
		szOut.Append(L" __unaligned");
	return szOut;
}

String PEPdb::ToStr_BaseType(IDiaSymbol* pISymbol)
{
	DWORD dwInfo;
	if (pISymbol->get_baseType(&dwInfo) != S_OK)
		return String(L"Unknown-BasicType");
	ULONGLONG ulLen;
	pISymbol->get_length(&ulLen);

	String szOut;
	switch (dwInfo)
	{
	case btUInt: szOut.Append(L"unsigned ");
	case btInt:
		switch (ulLen)
		{
		case 1:	szOut.Append(L"char");	  break;
		case 2: szOut.Append(L"short");	  break;
		case 4: szOut.Append(L"int");	  break;
		case 8: szOut.Append(L"__int64"); break;
		}
		dwInfo = 0xFFFFFFFF;
		break;
	case btFloat:
		switch (ulLen)
		{
		case 4: szOut.Append(L"float");  break;
		case 8: szOut.Append(L"double"); break;
		}
		dwInfo = 0xFFFFFFFF;
		break;
	}
	if (dwInfo == 0xFFFFFFFF)
		return szOut;
	szOut.Append(GSZ_BASE_TYPES[dwInfo]);
	return szOut;
}

String PEPdb::ToStr_FunctionType(IDiaSymbol* pISymbol)
{
	String sz;
	CComPtr<IDiaSymbol> pIRetType;
	if (pISymbol->get_type(&pIRetType) != S_OK)
		return sz;

	sz.Append(ToStr_Type(pIRetType));
	sz.Append(L" ");

	//CV_call_e ecv;
	//if (pISymbol->get_callingConvention((PDWORD)&ecv) == S_OK)
	//{
	//	sz.Append(GSZ_CALL_CONVS[ecv]);
	//	sz.Append(L" ");
	//}

	CComBSTR bszName;
	if (pISymbol->get_name(&bszName) == S_OK)
		sz.Append(bszName);

	CComPtr<IDiaSymbol> pIObj;
	if (pISymbol->get_objectPointerType(&pIObj) == S_OK)
	{
		sz.Append(L", => ");
		ToStr_Symbol(pIObj, 0);
		pIObj = 0;
	}
	LONG llThis;
	if (pISymbol->get_thisAdjust(&llThis) == S_OK)
	{
		sz.AppendFormat(L",    %d", llThis);
	}

	CComPtr<IDiaEnumSymbols> pIEnumPrms;
	if (SUCCEEDED(pISymbol->findChildren(SymTagNull, NULL, nsNone, &pIEnumPrms)))
	{
		CComPtr<IDiaSymbol> pIPrm;
		ULONG uCelt = 0, nParam = 0;

		sz.Append(L"(");
		while (SUCCEEDED(pIEnumPrms->Next(1, &pIPrm, &uCelt)) && (uCelt == 1))
		{
			CComPtr<IDiaSymbol> pIType;
			if (pIPrm->get_type(&pIType) == S_OK)
			{
				if (nParam++)
					sz.Append(L", ");
				sz.Append(ToStr_Type(pIType));
				pIType = 0;
			}
			pIPrm = 0;
		}
		pIEnumPrms = 0;
		if (nParam == 0)
			sz.Append(L"void");
		sz.Append(L") ");
	}
	pIRetType = 0;

	BOOL bSet;
	if (pISymbol->get_constType(&bSet) == S_OK && bSet)
		sz.Append(L"const ");
	return sz;
}

String PEPdb::ToStr_CustomType(IDiaSymbol* pISymbol)
{
	String szOut;
	DWORD dwOemId, dwOemSym;
	DWORD cbData = 0, dwCount;

	if (pISymbol->get_oemId(&dwOemId) == S_OK)
		szOut.AppendFormat(L"OEMId = %X, ", dwOemId);
	if (pISymbol->get_oemSymbolId(&dwOemSym) == S_OK)
		szOut.AppendFormat(L"SymbolId = %X, ", dwOemSym);

	if (pISymbol->get_types(0, &dwCount, NULL) == S_OK)
	{
		IDiaSymbol** ppIDiaSyms =
			(IDiaSymbol**)_alloca(sizeof(IDiaSymbol*) * dwCount);
		if (pISymbol->get_types(dwCount, &dwCount, ppIDiaSyms) == S_OK)
		{
			for (ULONG i = 0; i < dwCount; i++)
			{
				szOut.Append(ToStr_Type(ppIDiaSyms[i]));
				ppIDiaSyms[i]->Release();
			}
		}
	}
	if ((pISymbol->get_dataBytes(cbData, &cbData, NULL) == S_OK) && (cbData != 0))
	{
		szOut.Append(L", Data: ");
		PBYTE pbData = new BYTE[cbData];
		pISymbol->get_dataBytes(cbData, &cbData, pbData);
		for (ULONG i = 0; i < cbData; i++)
			szOut.AppendFormat(L"02X ", pbData[i]);
		delete[] pbData;
	}
	return szOut;
}

String PEPdb::ToStr_Type(IDiaSymbol* pISymbol)
{
	DWORD dwTag;
	if (pISymbol->get_symTag(&dwTag) != S_OK)
		return L"SymTag-Unknown";

	String szType;
	CComBSTR bszName;
	pISymbol->get_name(&bszName);
	if (dwTag != SymTagPointerType)
	{
		BOOL bSet;
		if ((pISymbol->get_constType(&bSet) == S_OK) && bSet)
			szType.Append(L"const ");
		if ((pISymbol->get_volatileType(&bSet) == S_OK) && bSet)
			szType.Append(L"volatile ");
		if ((pISymbol->get_unalignedType(&bSet) == S_OK) && bSet)
			szType.Append(L"__unaligned ");
	}
	switch (dwTag)
	{
		case SymTagUDT:
		case SymTagEnum:
		case SymTagTypedef:
			szType.Append(bszName);
		break;
		case SymTagFunctionType:
			szType.Append(ToStr_FunctionType(pISymbol));
		break;
		case SymTagPointerType:
			szType.Append(ToStr_PointerType(pISymbol));
		break;
		case SymTagArrayType:
			szType.Append(ToStr_ArrayType(pISymbol));
		break;
		case SymTagBaseType:
			szType.Append(ToStr_BaseType(pISymbol));
		break;
		case SymTagVTableShape:
			szType.Append(ToStr_VTableSahpe(pISymbol));
		break;
		case SymTagCustomType:
			szType.Append(ToStr_CustomType(pISymbol));
		break;
		case SymTagData: // This really is member data, just print its location
			szType.Append(ToStr_Location(pISymbol));
		break;
	}
	return szType;
}

String PEPdb::ToStr_VTableSahpe(IDiaSymbol* pISymbol)
{
	String sz = L"__vfptr";
	DWORD dwCount = 0;
	if (pISymbol->get_count(&dwCount) == S_OK)
		sz.AppendFormat(L"[%d]", dwCount);
	else
		sz.Append(L"[Unknown]");
	return sz;
}

String PEPdb::ToStr_CallSite(IDiaSymbol* pISymbol)
{
	String sz;
	DWORD dwISect, dwOffset;
	if (pISymbol->get_addressSection(&dwISect) == S_OK &&
		pISymbol->get_addressOffset(&dwOffset) == S_OK)
		sz.AppendFormat(L"[0x%04x:0x%08x]  ", dwISect, dwOffset);

	DWORD rva;
	if (pISymbol->get_relativeVirtualAddress(&rva) == S_OK)
		sz.AppendFormat(L"0x%08X  ", rva);

	CComPtr<IDiaSymbol> pIFuncType;
	if (pISymbol->get_type(&pIFuncType) == S_OK)
	{
		DWORD tag;
		if (pIFuncType->get_symTag(&tag) == S_OK)
		{
			switch (tag)
			{
			case SymTagFunctionType:
				sz.Append(PEPdb::ToStr_FunctionType(pIFuncType));
				break;
			case SymTagPointerType:
				sz.Append(PEPdb::ToStr_Type(pIFuncType));
				break;
			default:
				sz.Append(L"???\xd\xa");
				break;
			}
		}
		pIFuncType = 0;
	}
	return sz;
}

String PEPdb::ToStr_UndName(IDiaSymbol* pISymbol)
{
	String sz;
	CComBSTR bszName;
	if (pISymbol->get_undecoratedName(&bszName) != S_OK)
	{
		if (pISymbol->get_name(&bszName) == S_OK)
			sz.AppendFormat(L"%s", (bszName[0] != L'\0') ? bszName : L"(none)");
		else
			sz.Append(L"(none)");
		return sz;
	}
	if (bszName[0] != L'\0')
		sz.AppendFormat(L"%s", bszName);
	return sz;
}

String PEPdb::ToStr_Thunk(IDiaSymbol* pISymbol)
{
	String sz;
	DWORD dwRVA, dwSect, dwOffset;
	if (pISymbol->get_relativeVirtualAddress(&dwRVA) == S_OK &&
		pISymbol->get_addressSection(&dwSect) == S_OK &&
		pISymbol->get_addressOffset(&dwOffset) == S_OK)
		sz.AppendFormat(L"[%08X][%04X:%08X]", dwRVA, dwSect, dwOffset);

	if (pISymbol->get_targetSection(&dwSect) == S_OK &&
		pISymbol->get_targetOffset(&dwOffset) == S_OK &&
		pISymbol->get_targetRelativeVirtualAddress(&dwRVA) == S_OK)
		sz.AppendFormat(L", target [%08X][%04X:%08X] ", dwRVA, dwSect, dwOffset);
	else
	{
		CComBSTR bszName;
		if (pISymbol->get_name(&bszName) != S_OK)
			bszName = L"(none)";
		sz.AppendFormat(L", target : %s", bszName);
	}
	return sz;
}

String PEPdb::ToStr_Symbol(IDiaSymbol* pISymbol, DWORD dwIndent)
{
	DWORD dwSymTag;
	if (pISymbol->get_symTag(&dwSymTag) != S_OK)
		return L"Unknown Symbol.";

	String sz;
	if (dwSymTag == SymTagFunction)
		sz.Append(GSZ_CRLF);
	for (DWORD i = 0; i < dwIndent; i++)
		sz.Append(L" ");
	sz.Append(GSZ_SYM_TAGS[dwSymTag]);

	ULONGLONG ulLen;
	CComPtr<IDiaSymbol> pIType;
	CComPtr<IDiaEnumSymbols> pIEnumSym;
	switch (dwSymTag)
	{
	case SymTagCompilandDetails:
		sz.Append(ToStr_CompilandDetails(pISymbol));
		break;
	case SymTagCompilandEnv:
		sz.Append(ToStr_CompilandEnv(pISymbol));
		break;
	case SymTagData:
		sz.Append(ToStr_Data(pISymbol));
		break;
	case SymTagFunction:
	case SymTagBlock:
		sz.Append(ToStr_Location(pISymbol));
		if (pISymbol->get_length(&ulLen) == S_OK)
			sz.AppendFormat(L", len = %08X, ", ulLen);

		if (dwSymTag == SymTagFunction)
		{
			DWORD dwCall;
			if (pISymbol->get_callingConvention(&dwCall) == S_OK)
				sz.AppendFormat(L", %s", GSZ_CALL_CONVS[dwCall]);
		}
		ToStr_UndName(pISymbol);
		sz.Append(GSZ_CRLF);
		if (dwSymTag == SymTagFunction)
		{
			BOOL f;
			//for (DWORD i = 0; i < dwIndent; i++)
			//	putwchar(L' ');
			sz.Append(L"    Function attribute:");
			if ((pISymbol->get_isCxxReturnUdt(&f) == S_OK) && f)
				sz.Append(L" return user defined type (C++ style)");
			if ((pISymbol->get_constructor(&f) == S_OK) && f)
				sz.Append(L" instance constructor");
			if ((pISymbol->get_isConstructorVirtualBase(&f) == S_OK) && f)
				sz.Append(L" instance constructor of a class with virtual base");
			sz.Append(L"\xd\xa");

			//for (DWORD i = 0; i < dwIndent; i++)
			//	putwchar(L' ');
			sz.Append(L"    Function info:");
			if ((pISymbol->get_hasAlloca(&f) == S_OK) && f)
				sz.Append(L" alloca");
			if ((pISymbol->get_hasSetJump(&f) == S_OK) && f)
				sz.Append(L" setjmp");
			if ((pISymbol->get_hasLongJump(&f) == S_OK) && f)
				sz.Append(L" longjmp");
			if ((pISymbol->get_hasInlAsm(&f) == S_OK) && f)
				sz.Append(L" inlasm");
			if ((pISymbol->get_hasEH(&f) == S_OK) && f)
				sz.Append(L" eh");
			if ((pISymbol->get_inlSpec(&f) == S_OK) && f)
				sz.Append(L" inl_specified");
			if ((pISymbol->get_hasSEH(&f) == S_OK) && f)
				sz.Append(L" seh");
			if ((pISymbol->get_isNaked(&f) == S_OK) && f)
				sz.Append(L" naked");
			if ((pISymbol->get_hasSecurityChecks(&f) == S_OK) && f)
				sz.Append(L" gschecks");
			if ((pISymbol->get_isSafeBuffers(&f) == S_OK) && f)
				sz.Append(L" safebuffers");
			if ((pISymbol->get_hasEHa(&f) == S_OK) && f)
				sz.Append(L" asyncheh");
			if ((pISymbol->get_noStackOrdering(&f) == S_OK) && f)
				sz.Append(L" gsnostackordering");
			if ((pISymbol->get_wasInlined(&f) == S_OK) && f)
				sz.Append(L" wasinlined");
			if ((pISymbol->get_strictGSCheck(&f) == S_OK) && f)
				sz.Append(L" strict_gs_check");
			sz.Append(L"\xd\xa");
		}

		if (SUCCEEDED(pISymbol->findChildren(SymTagNull, NULL, nsNone, &pIEnumSym)))
		{
			ULONG celt = 0;
			CComPtr<IDiaSymbol> pIChild;
			while (SUCCEEDED(pIEnumSym->Next(1, &pIChild, &celt)) && (celt == 1))
			{
				sz.Append(ToStr_Symbol(pIChild, dwIndent + 2));
				pIChild = 0;
			}
			pIEnumSym = 0;
		}
		return sz;
	case SymTagAnnotation:
		sz.Append(ToStr_Location(pISymbol));
		sz.Append(GSZ_CRLF);
		break;
	case SymTagLabel:
		sz.Append(ToStr_Location(pISymbol));
		sz.Append(L", ");
		ToStr_Name(pISymbol);
		break;
	case SymTagEnum:
	case SymTagTypedef:
	case SymTagUDT:
	case SymTagBaseClass:
	{
		sz.Append(ToStr_Name(pISymbol));
		//ToStr_SymbolType(pSymbol);
		CComPtr<IDiaSymbol> pIType;
		if (pISymbol->get_type(&pIType) == S_OK)
		{
			sz.AppendFormat(L", Type: ", ToStr_Type(pISymbol));
			pIType = 0;
		}
	}
	break;
	case SymTagFuncDebugStart:
	case SymTagFuncDebugEnd:
		sz.Append(ToStr_Location(pISymbol));
		break;

	case SymTagFunctionArgType:
	case SymTagFunctionType:
	case SymTagPointerType:
	case SymTagArrayType:
	case SymTagBaseType:
		if (pISymbol->get_type(&pIType) == S_OK)
		{
			sz.Append(ToStr_Type(pIType));
			pIType = 0;
		}
		sz.Append(GSZ_CRLF);
		break;

	case SymTagThunk:
		sz.Append(ToStr_Thunk(pISymbol));
		break;

	case SymTagCallSite:
		sz.Append(ToStr_CallSite(pISymbol));
		break;

	default:
		ToStr_Name(pISymbol);
		if (pISymbol->get_type(&pIType) == S_OK)
		{
			sz.AppendFormat(L" has type %s", ToStr_Type(pIType));
			pIType = 0;
		}
		break;
	}

	if ((dwSymTag == SymTagUDT) || (dwSymTag == SymTagAnnotation))
	{
		IDiaEnumSymbols* pEnumChildren;
		sz.Append(GSZ_CRLF);
		if (SUCCEEDED(pISymbol->findChildren(SymTagNull, NULL, nsNone, &pEnumChildren)))
		{
			IDiaSymbol *pChild;
			ULONG celt = 0;
			while (SUCCEEDED(pEnumChildren->Next(1, &pChild, &celt)) && (celt == 1))
			{
				sz.Append(ToStr_Symbol(pChild, dwIndent + 2));
				pChild->Release();
			}
			pEnumChildren->Release();
		}
	}
	sz.Append(GSZ_CRLF);
	return sz;
}

String PEPdb::ToStr_Function(IDiaSymbol *pISymbol)
{
	String szFunc;

	DWORD dwAccess = 0;
	if (pISymbol->get_access(&dwAccess) == S_OK)
	{
		szFunc.Append(GSZ_ACCESS_SPECS[dwAccess]);
		szFunc.Append(L" ");
	}

	BOOL bIsSet = FALSE;
	if ((pISymbol->get_isStatic(&bIsSet) == S_OK) && bIsSet)
		szFunc.Append(L"static ");
	if (pISymbol->get_virtual(&bIsSet) == S_OK && bIsSet)
		szFunc.Append(L"virtual ");

	CComPtr<IDiaSymbol> pIFuncType;
	if (pISymbol->get_type(&pIFuncType) == S_OK)
	{
		CComPtr<IDiaSymbol> pIRetType;
		if (pIFuncType->get_type(&pIRetType) == S_OK)
		{
			szFunc.Append(ToStr_Type(pIRetType));
			szFunc.Append(L" ");

			CComBSTR bszName;
			if (pISymbol->get_name(&bszName) == S_OK)
				szFunc.Append(bszName);

			CComPtr<IDiaEnumSymbols> pIEnumPrms;
			if (SUCCEEDED(pIFuncType->findChildren(SymTagFunctionArgType, NULL, nsNone, &pIEnumPrms)))
			{
				CComPtr<IDiaSymbol> pIPrm;
				ULONG uCelt = 0, nParam = 0;
				szFunc.Append(L"(");
				while (SUCCEEDED(pIEnumPrms->Next(1, &pIPrm, &uCelt)) && (uCelt == 1))
				{
					CComPtr<IDiaSymbol> pIType;
					if (pIPrm->get_type(&pIType) == S_OK)
					{
						if (nParam++) szFunc.Append(L", ");
						szFunc.Append(ToStr_Type(pIType));
						pIType = 0;
					}
					pIPrm = 0;
				}
				pIEnumPrms = 0;
				if (nParam == 0)
					szFunc.Append(L"void");
				szFunc.Append(L")");
			}
			pIRetType = 0;
		}
		pIFuncType = 0;

		if (pISymbol->get_pure(&bIsSet) == S_OK && bIsSet)
			szFunc.Append(L" = 0");
		szFunc.Append(L";");
	}
	return szFunc;
}

String PEPdb::ToStr_CompilandDetails(IDiaSymbol* pISymbol)
{
	String sz;
	DWORD dwLanguage;
	if (pISymbol->get_language(&dwLanguage) == S_OK)
		sz.AppendFormat(L"Language: %s\xd\xa", PEPdb::GSZ_LANG_SPECS[dwLanguage]);

	DWORD dwPlatform;
	if (pISymbol->get_platform(&dwPlatform) == S_OK)
		sz.AppendFormat(L"Target processor: %s\xd\xa", PEPdb::GSZ_CPU_TYPES[dwPlatform]);

	BOOL bSet;
	if (pISymbol->get_editAndContinueEnabled(&bSet) == S_OK)
		sz.AppendFormat(L"Compiled for edit and continue: %s\xd\xa", (bSet) ? L"yes" : L"no");
	if (pISymbol->get_hasDebugInfo(&bSet) == S_OK)
		sz.AppendFormat(L"Compiled without debugging info: %s\xd\xa", (bSet) ? L"yes" : L"no");
	if (pISymbol->get_isLTCG(&bSet) == S_OK)
		sz.AppendFormat(L"Compiled with LTCG: %s\xd\xa", (bSet) ? L"yes" : L"no");
	if (pISymbol->get_isDataAligned(&bSet) == S_OK)
		sz.AppendFormat(L"Compiled with /bzalign: %s\xd\xa", (bSet) ? L"yes" : L"no");
	if (pISymbol->get_hasManagedCode(&bSet) == S_OK)
		sz.AppendFormat(L"Managed code present: %s\xd\xa", (bSet) ? L"yes" : L"no");
	if (pISymbol->get_hasSecurityChecks(&bSet) == S_OK)
		sz.AppendFormat(L"Compiled with /GS: %s\xd\xa", (bSet) ? L"yes" : L"no");
	if (pISymbol->get_isSdl(&bSet) == S_OK)
		sz.AppendFormat(L"Compiled with /sdl: %s\xd\xa", (bSet) ? L"yes" : L"no");
	if (pISymbol->get_isHotpatchable(&bSet) == S_OK)
		sz.AppendFormat(L"Compiled with /hotpatch: %s\xd\xa", (bSet) ? L"yes" : L"no");
	if (pISymbol->get_isCVTCIL(&bSet) == S_OK)
		sz.AppendFormat(L"Converted by CVTCIL: %s\xd\xa", (bSet) ? L"yes" : L"no");
	if (pISymbol->get_isMSILNetmodule(&bSet) == S_OK)
		sz.AppendFormat(L"MSIL module: %s\xd\xa", (bSet) ? L"yes" : L"no");

	DWORD dwVerMajor, dwVerMinor;
	DWORD dwVerBuild, dwVerQFE;
	if ((pISymbol->get_frontEndMajor(&dwVerMajor) == S_OK) &&
		(pISymbol->get_frontEndMinor(&dwVerMinor) == S_OK) &&
		(pISymbol->get_frontEndBuild(&dwVerBuild) == S_OK))
	{
		sz.AppendFormat(L"Frontend Version: Major = %u, Minor = %u, Build = %u",
			dwVerMajor, dwVerMinor, dwVerBuild);
		if (pISymbol->get_frontEndQFE(&dwVerQFE) == S_OK)
			sz.AppendFormat(L", QFE = %u", dwVerQFE);
		sz.Append(L"\xd\xa");
	}

	if ((pISymbol->get_backEndMajor(&dwVerMajor) == S_OK) &&
		(pISymbol->get_backEndMinor(&dwVerMinor) == S_OK) &&
		(pISymbol->get_backEndBuild(&dwVerBuild) == S_OK))
	{
		sz.AppendFormat(L"Backend Version: Major = %u, Minor = %u, Build = %u",
			dwVerMajor, dwVerMinor, dwVerBuild);
		if (pISymbol->get_backEndQFE(&dwVerQFE) == S_OK)
			sz.AppendFormat(L", QFE = %u", dwVerQFE);
		sz.Append(L"\xd\xa");
	}

	CComBSTR bszCompilerName;
	if (pISymbol->get_compilerName(&bszCompilerName) == S_OK)
		sz.AppendFormat(L"Version string: %s\xd\xa", bszCompilerName);
	sz.Append(L"\xd\xa");

	return sz;
}
