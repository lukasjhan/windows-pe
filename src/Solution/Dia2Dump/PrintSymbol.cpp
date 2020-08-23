#include "stdafx.h"
#include <malloc.h>
#include "dia2.h"
#include "regs.h"
#include "PrintSymbol.h"

// Basic types
PCWSTR  const GSZ_BASE_TYPES[] =
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
PCWSTR  const GSZ_SYM_TAGS[] =
{
	  L"(SymTagNull)",			// SymTagNull
	  L"Executable (Global)",	// SymTagExe
	  L"Compiland",				// SymTagCompiland
	  L"CompilandDetails",		// SymTagCompilandDetails
	  L"CompilandEnv",			// SymTagCompilandEnv
	  L"Function",				// SymTagFunction
	  L"Block",					// SymTagBlock
	  L"Data",					// SymTagData
	  L"Annotation",			// SymTagAnnotation
	  L"Label",					// SymTagLabel
	  L"PublicSymbol",			// SymTagPublicSymbol
	  L"UserDefinedType",		// SymTagUDT
	  L"Enum",					// SymTagEnum
	  L"FunctionType",			// SymTagFunctionType
	  L"PointerType",			// SymTagPointerType
	  L"ArrayType",				// SymTagArrayType
	  L"BaseType",				// SymTagBaseType
	  L"Typedef",				// SymTagTypedef
	  L"BaseClass",				// SymTagBaseClass
	  L"Friend",				// SymTagFriend
	  L"FunctionArgType",		// SymTagFunctionArgType
	  L"FuncDebugStart",		// SymTagFuncDebugStart
	  L"FuncDebugEnd",			// SymTagFuncDebugEnd
	  L"UsingNamespace",		// SymTagUsingNamespace
	  L"VTableShape",			// SymTagVTableShape
	  L"VTable",				// SymTagVTable
	  L"Custom",				// SymTagCustom
	  L"Thunk",					// SymTagThunk
	  L"CustomType",			// SymTagCustomType
	  L"ManagedType",			// SymTagManagedType
	  L"Dimension",				// SymTagDimension
	  L"CallSite",				// SymTagCallSite
	  L"InlineSite",			// SymTagInlineSite
	  L"BaseInterface",			// SymTagBaseInterface
	  L"VectorType",			// SymTagVectorType
	  L"MatrixType",			// SymTagMatrixType
	  L"HLSLType",				// SymTagHLSLType
	  L"Caller",				// SymTagCaller,
	  L"Callee"					// SymTagCallee,
};

// Processors
PCWSTR  const GSZ_FLOAT_PACKS[] =
{
	  L"hardware processor (80x87 for Intel processors)",    // CV_CFL_NDP
	  L"emulator",                                           // CV_CFL_EMU
	  L"altmath",                                            // CV_CFL_ALT
	  L"???"
};

PCWSTR  const GSZ_CPU_TYPES[] =
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
	L"MIPS (Generic)",		//  CV_CFL_MIPSR4000
	L"MIPS16",				//  CV_CFL_MIPS16
	L"MIPS32",				//  CV_CFL_MIPS32
	L"MIPS64",				//  CV_CFL_MIPS64
	L"MIPS I",				//  CV_CFL_MIPSI
	L"MIPS II",				//  CV_CFL_MIPSII
	L"MIPS III",			//  CV_CFL_MIPSIII
	L"MIPS IV",				//  CV_CFL_MIPSIV
	L"MIPS V",				//  CV_CFL_MIPSV
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"M68000",				//  CV_CFL_M68000
	L"M68010",				//  CV_CFL_M68010
	L"M68020",				//  CV_CFL_M68020
	L"M68030",				//  CV_CFL_M68030
	L"M68040",				//  CV_CFL_M68040
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
	L"Alpha 21064",			// CV_CFL_ALPHA, CV_CFL_ALPHA_21064
	L"Alpha 21164",			// CV_CFL_ALPHA_21164
	L"Alpha 21164A",		// CV_CFL_ALPHA_21164A
	L"Alpha 21264",			// CV_CFL_ALPHA_21264
	L"Alpha 21364",			// CV_CFL_ALPHA_21364
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
	L"PPC 601",				// CV_CFL_PPC601
	L"PPC 603",				// CV_CFL_PPC603
	L"PPC 604",				// CV_CFL_PPC604
	L"PPC 620",				// CV_CFL_PPC620
	L"PPC w/FP",			// CV_CFL_PPCFP
	L"PPC (Big Endian)",	// CV_CFL_PPCBE
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
	L"SH3",					// CV_CFL_SH3
	L"SH3E",				// CV_CFL_SH3E
	L"SH3DSP",				// CV_CFL_SH3DSP
	L"SH4",					// CV_CFL_SH4
	L"SHmedia",				// CV_CFL_SHMEDIA
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
	L"ARM3",				// CV_CFL_ARM3
	L"ARM4",				// CV_CFL_ARM4
	L"ARM4T",				// CV_CFL_ARM4T
	L"ARM5",				// CV_CFL_ARM5
	L"ARM5T",				// CV_CFL_ARM5T
	L"ARM6",				// CV_CFL_ARM6
	L"ARM (XMAC)",			// CV_CFL_ARM_XMAC
	L"ARM (WMMX)",			// CV_CFL_ARM_WMMX
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"???",
	L"Omni",				// CV_CFL_OMNI
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
	L"Itanium",				// CV_CFL_IA64, CV_CFL_IA64_1
	L"Itanium (McKinley)",	// CV_CFL_IA64_2
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
	L"CEE",			// CV_CFL_CEE
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
	L"AM33",		// CV_CFL_AM33
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
	L"M32R",			// CV_CFL_M32R
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
	L"TriCore",			// CV_CFL_TRICORE
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
	L"AMD64",			// CV_CFL_X64
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
	L"EBC",				// CV_CFL_EBC
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
	L"Thumb (CE)",		// CV_CFL_THUMB
	L"???",
	L"???",
	L"???",
	L"ARM",				// CV_CFL_ARMNT
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
	L"D3D11_SHADE",		// CV_CFL_D3D11_SHADER
};

PCWSTR  const GSZ_DATA_KINDS[] =
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

PCWSTR  const GSZ_UDT_KINDS[] =
{
	L"struct",
	L"class",
	L"union",
	L"interface",
};

PCWSTR  const GSZ_ACCESS_SPECS[] =
{
	L"",	// No access specifier
	L"private",
	L"protected",
	L"public"
};

PCWSTR  const GSZ_CALL_CONVS[] =
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

PCWSTR  const GSZ_LANG_SPECS[] =
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

PCWSTR const GSZ_LOC_TYPES[] =
{
	L"NULL",
	L"static",
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

PCWSTR const GSZ_THUNK_ORDS[] =
{
	L"NOTYPE",		// standard thunk
	L"THIS",		// "this" adjustor thunk
	L"VCALL",		// virtual call thunk
	L"PCODE",		// pcode thunk
	L"LOAD",		// thunk which loads the address to jump to
	L"INCREMENTAL",
	L"BRANCHISLAND"
};


////////////////////////////////////////////////////////////
// Print a public symbol info: name, VA, RVA, SEG:OFF
//
void PrintPublicSymbol(IDiaSymbol* pISymbol)
{
	DWORD dwSymTag, dwRVA, dwSeg, dwOff;
	CComBSTR bszName;
  
	if (pISymbol->get_symTag(&dwSymTag) != S_OK)
		return;
	if (pISymbol->get_relativeVirtualAddress(&dwRVA) != S_OK)
		dwRVA = 0xFFFFFFFF;

	pISymbol->get_addressSection(&dwSeg);
	pISymbol->get_addressOffset(&dwOff);
	wprintf(L"%s: [%08X][%04X:%08X] ", GSZ_SYM_TAGS[dwSymTag], dwRVA, dwSeg, dwOff);
  
	if (dwSymTag == SymTagThunk)
	{
		if (pISymbol->get_name(&bszName) == S_OK) 
			wprintf(L"%s\n", bszName);
		else 
		{
			if (pISymbol->get_targetRelativeVirtualAddress(&dwRVA) != S_OK)
				dwRVA = 0xFFFFFFFF;
			pISymbol->get_targetSection(&dwSeg);
			pISymbol->get_targetOffset(&dwOff);
			wprintf(L"target -> [%08X][%04X:%08X]\n", dwRVA, dwSeg, dwOff);
		}
	}
	else
	{
		// must be a function or a data symbol
		CComBSTR bstrUndname;
		if (pISymbol->get_name(&bszName) == S_OK)
		{
			if (pISymbol->get_undecoratedName(&bstrUndname) == S_OK)
				wprintf(L"%s(%s)\n", bszName, bstrUndname);
			else
				wprintf(L"%s\n", bszName);
		}
	}
}

////////////////////////////////////////////////////////////
// Print a global symbol info: name, VA, RVA, SEG:OFF
//
void PrintGlobalSymbol(IDiaSymbol* pISymbol)
{
	DWORD dwSymTag, dwRVA, dwSeg, dwOff;
	if (pISymbol->get_symTag(&dwSymTag) != S_OK)
		return;

	if (pISymbol->get_relativeVirtualAddress(&dwRVA) != S_OK)
		dwRVA = 0xFFFFFFFF;

	pISymbol->get_addressSection(&dwSeg);
	pISymbol->get_addressOffset(&dwOff);
	wprintf(L"%s: [%08X][%04X:%08X] ", GSZ_SYM_TAGS[dwSymTag], dwRVA, dwSeg, dwOff);
  
	if (dwSymTag == SymTagThunk)
	{
		CComBSTR bszName;
		if (pISymbol->get_name(&bszName) == S_OK)
			wprintf(L"%s\n", bszName);
		else
		{
			if (pISymbol->get_targetRelativeVirtualAddress(&dwRVA) != S_OK)
				dwRVA = 0xFFFFFFFF;
			pISymbol->get_targetSection(&dwSeg);
			pISymbol->get_targetOffset(&dwOff);
			wprintf(L"target -> [%08X][%04X:%08X]\n", dwRVA, dwSeg, dwOff);
		}
	}
	else
	{
		CComBSTR bszName, bszUndname;
		if (pISymbol->get_name(&bszName) == S_OK) 
		{
			if (pISymbol->get_undecoratedName(&bszUndname) == S_OK)
				wprintf(L"%s(%s)\n", bszName, bszUndname);
			else
				wprintf(L"%s\n", bszName);
		}
	}
}

////////////////////////////////////////////////////////////
// Print a callsite symbol info: SEG:OFF, RVA, type
//
void PrintCallSiteInfo(IDiaSymbol* pISymbol)
{
	DWORD dwISect, dwOffset;
	if (pISymbol->get_addressSection(&dwISect) == S_OK &&
		pISymbol->get_addressOffset(&dwOffset) == S_OK)
		wprintf(L"[0x%04x:0x%08x]  ", dwISect, dwOffset);

	DWORD rva;
	if (pISymbol->get_relativeVirtualAddress(&rva) == S_OK)
		wprintf(L"0x%08X  ", rva);

	CComPtr<IDiaSymbol> pIFuncType;
	if (pISymbol->get_type(&pIFuncType) == S_OK)
	{
		DWORD tag;
		if (pIFuncType->get_symTag(&tag) == S_OK)
		{
			switch(tag)
			{
				case SymTagFunctionType :
					PrintFunctionType(pISymbol); break;
				case SymTagPointerType  :
					PrintFunctionType(pIFuncType); break;
				default :
					wprintf(L"???\n"); break;
			}
		}
		pIFuncType = 0;
	}
}

void PrintFunctionOrBlock(IDiaSymbol* pISymbol, DWORD dwSymTag, DWORD dwIndent)
{
	ULONGLONG ulLen;

	PrintLocation(pISymbol);
	if (pISymbol->get_length(&ulLen) == S_OK)
		wprintf(L", len = %08X, ", ulLen);

	if (dwSymTag == SymTagFunction)
	{
		DWORD dwCall;
		if (pISymbol->get_callingConvention(&dwCall) == S_OK)
			wprintf(L", %s", SafeDRef(GSZ_CALL_CONVS, dwCall));
	}
	PrintUndName(pISymbol);
	putwchar(L'\n');
	if (dwSymTag == SymTagFunction)
	{
		for (DWORD i = 0; i < dwIndent; i++)
			putwchar(L' ');

		wprintf(L"                 Function Type:");	// YHD
		PrintFunctionType(pISymbol);
		putwchar(L'\n');

		//wprintf(L"                 Function attribute:");
		//if ((pISymbol->get_isCxxReturnUdt(&f) == S_OK) && f)
		//	wprintf(L" return user defined type (C++ style)");
		//if ((pISymbol->get_constructor(&f) == S_OK) && f)
		//	wprintf(L" instance constructor");
		//if ((pISymbol->get_isConstructorVirtualBase(&f) == S_OK) && f)
		//	wprintf(L" instance constructor of a class with virtual base");
		//putwchar(L'\n');

		for (DWORD i = 0; i < dwIndent; i++)
			putwchar(L' ');
		wprintf(L"                 Function info:");

		BOOL f;
		if ((pISymbol->get_hasAlloca(&f) == S_OK) && f)
			wprintf(L" alloca");
		if ((pISymbol->get_hasSetJump(&f) == S_OK) && f)
			wprintf(L" setjmp");
		if ((pISymbol->get_hasLongJump(&f) == S_OK) && f)
			wprintf(L" longjmp");
		if ((pISymbol->get_hasInlAsm(&f) == S_OK) && f)
			wprintf(L" inlasm");
		if ((pISymbol->get_hasEH(&f) == S_OK) && f)
			wprintf(L" eh");
		if ((pISymbol->get_inlSpec(&f) == S_OK) && f)
			wprintf(L" inl_specified");
		if ((pISymbol->get_hasSEH(&f) == S_OK) && f)
			wprintf(L" seh");
		if ((pISymbol->get_isNaked(&f) == S_OK) && f)
			wprintf(L" naked");
		if ((pISymbol->get_hasSecurityChecks(&f) == S_OK) && f)
			wprintf(L" gschecks");
		if ((pISymbol->get_isSafeBuffers(&f) == S_OK) && f)
			wprintf(L" safebuffers");
		if ((pISymbol->get_hasEHa(&f) == S_OK) && f)
			wprintf(L" asyncheh");
		if ((pISymbol->get_noStackOrdering(&f) == S_OK) && f)
			wprintf(L" gsnostackordering");
		if ((pISymbol->get_wasInlined(&f) == S_OK) && f)
			wprintf(L" wasinlined");
		if ((pISymbol->get_strictGSCheck(&f) == S_OK) && f)
			wprintf(L" strict_gs_check");
		putwchar(L'\n');
	}

	CComPtr<IDiaEnumSymbols> pIEnumChild;
	if (SUCCEEDED(pISymbol->findChildren(SymTagNull, NULL, nsNone, &pIEnumChild)))
	{
		CComPtr<IDiaSymbol> pIChild;
		ULONG celt = 0;
		while (SUCCEEDED(pIEnumChild->Next(1, &pIChild, &celt)) && (celt == 1))
		{
			PrintSymbol(pIChild, dwIndent + 2);
			pIChild = 0;
		}
		pIEnumChild = 0;
	}
}

////////////////////////////////////////////////////////////
// Print a symbol info: name, type etc.
//
void PrintSymbol(IDiaSymbol* pISymbol, DWORD dwIndent)
{
	DWORD dwSymTag = 0;
	if (pISymbol->get_symTag(&dwSymTag) != S_OK)
	{
		wprintf(L"ERROR - PrintSymbol get_symTag() failed\n");
		return;
	}
	if (dwSymTag == SymTagFunction)
		putwchar(L'\n');
	PrintSymTag(dwSymTag);

	for (DWORD i = 0; i < dwIndent; i++)
		putwchar(L' ');

	CComPtr<IDiaSymbol> pIType;
	switch (dwSymTag)
	{
		case SymTagCompilandDetails:
			PrintCompilandDetails(pISymbol);
		break;
		case SymTagCompilandEnv:
			PrintCompilandEnv(pISymbol);
		break;
		case SymTagData:
			PrintData(pISymbol, dwIndent + 2);
		break;
		case SymTagFunction:
		case SymTagBlock:
			PrintFunctionOrBlock(pISymbol, dwSymTag, dwIndent);
		return;
		case SymTagAnnotation:
			PrintLocation(pISymbol);
			putwchar(L'\n');
		break;
		case SymTagLabel:
			PrintLocation(pISymbol);
			wprintf(L", ");
			PrintName(pISymbol);
		break;
		case SymTagEnum:
		case SymTagTypedef:
		case SymTagUDT:
		case SymTagBaseClass:
			PrintUDT(pISymbol);
		break;
		case SymTagFuncDebugStart:
		case SymTagFuncDebugEnd:
			PrintLocation(pISymbol);
		break;

		case SymTagFunctionArgType:
		case SymTagFunctionType:
		case SymTagPointerType:
		case SymTagArrayType:
		case SymTagBaseType:
			if (pISymbol->get_type(&pIType) == S_OK)
			{
				PrintType(pIType);
				pIType = 0;
			}
			putwchar(L'\n');
		break;

		case SymTagThunk:
			PrintThunk(pISymbol);
		break;

		case SymTagCallSite:
			PrintCallSiteInfo(pISymbol);
		break;
        
		default:
			PrintName(pISymbol);
			CComPtr<IDiaSymbol> pIType;
			if (pISymbol->get_type(&pIType) == S_OK)
			{
				wprintf(L" has type ");
				PrintType(pIType);
				pIType = 0;
			}
		break;
	}

	if ((dwSymTag == SymTagUDT) || (dwSymTag == SymTagAnnotation))
	{
		CComPtr<IDiaEnumSymbols> pIEnumChild;
		putwchar(L'\n');
		if (SUCCEEDED(pISymbol->findChildren(SymTagNull, NULL, nsNone, &pIEnumChild)))
		{
			CComPtr<IDiaSymbol> pIChild;
			ULONG celt = 0;
			while (SUCCEEDED(pIEnumChild->Next(1, &pIChild, &celt)) && (celt == 1))
			{
				PrintSymbol(pIChild, dwIndent + 2);
				pIChild = 0;
			}
			pIEnumChild = 0;
		}
	}
	putwchar(L'\n');
}

////////////////////////////////////////////////////////////
// Print the string coresponding to the symbol's tag property
//
void PrintSymTag(DWORD dwSymTag)
{
	wprintf(L"%-15s: ", SafeDRef(GSZ_SYM_TAGS, dwSymTag));
}

////////////////////////////////////////////////////////////
// Print the name of the symbol
//
void PrintName(IDiaSymbol* pISymbol)
{
	CComBSTR bszName, bstrUndName;
	if (pISymbol->get_name(&bszName) != S_OK)
	{
		wprintf(L"(none)");
		return;
	}

	if (pISymbol->get_undecoratedName(&bstrUndName) == S_OK)
	{
		if (wcscmp(bszName, bstrUndName) == 0)
			wprintf(L"%s", bszName);
		else 
			wprintf(L"%s(%s)", bstrUndName, bszName);
	}
	else 
		wprintf(L"%s", bszName);
}

////////////////////////////////////////////////////////////
// Print the undecorated name of the symbol
//  - only SymTagFunction, SymTagData and SymTagPublicSymbol
//    can have this property set
//
void PrintUndName(IDiaSymbol* pISymbol)
{
	CComBSTR bszName;
	if (pISymbol->get_undecoratedName(&bszName) != S_OK)
	{
		if (pISymbol->get_name(&bszName) == S_OK)
			wprintf(L"%s", (bszName[0] != L'\0') ? bszName : L"(none)");
		else
			wprintf(L"(none)");
		return;
	}
	if (bszName[0] != L'\0')
		wprintf(L"%s", bszName);
}

////////////////////////////////////////////////////////////
// Print a SymTagThunk symbol's info
//
void PrintThunk(IDiaSymbol* pISymbol)
{
	DWORD dwRVA, dwISect, dwOffset;
	if ((pISymbol->get_relativeVirtualAddress(&dwRVA) == S_OK) &&
		(pISymbol->get_addressSection(&dwISect) == S_OK) &&
		(pISymbol->get_addressOffset(&dwOffset) == S_OK))
		wprintf(L"[%08X][%04X:%08X]", dwRVA, dwISect, dwOffset);

	if ((pISymbol->get_targetSection(&dwISect) == S_OK) &&
		(pISymbol->get_targetOffset(&dwOffset) == S_OK) &&
		(pISymbol->get_targetRelativeVirtualAddress(&dwRVA) == S_OK))
		wprintf(L", target [%08X][%04X:%08X] ", dwRVA, dwISect, dwOffset);
	else
	{
		wprintf(L", target ");
		PrintName(pISymbol);
	}
}

////////////////////////////////////////////////////////////
// Print the compiland/module details: language, platform...
//
void PrintCompilandDetails(IDiaSymbol* pISymbol)
{
	DWORD dwLanguage;
	if (pISymbol->get_language(&dwLanguage) == S_OK)
		wprintf(L"\n\tLanguage: %s\n", SafeDRef(GSZ_LANG_SPECS, dwLanguage));

	DWORD dwPlatform;
	if (pISymbol->get_platform(&dwPlatform) == S_OK)
		wprintf(L"\tTarget processor: %s\n", SafeDRef(GSZ_CPU_TYPES, dwPlatform));

	BOOL fEC;
	if (pISymbol->get_editAndContinueEnabled(&fEC) == S_OK)
	{
		if (fEC)
			wprintf(L"\tCompiled for edit and continue: yes\n");
		else
			wprintf(L"\tCompiled for edit and continue: no\n");
	}

	BOOL fDbgInfo;
	if (pISymbol->get_hasDebugInfo(&fDbgInfo) == S_OK)
	{
		if (fDbgInfo)
			wprintf(L"\tCompiled without debugging info: no\n");
		else
			wprintf(L"\tCompiled without debugging info: yes\n");
	}

	BOOL fLTCG;
	if (pISymbol->get_isLTCG(&fLTCG) == S_OK)
	{
		if (fLTCG)
			wprintf(L"\tCompiled with LTCG: yes\n");
		else
			wprintf(L"\tCompiled with LTCG: no\n");
	}

	BOOL fDataAlign;
	if (pISymbol->get_isDataAligned(&fDataAlign) == S_OK)
	{
		if (fDataAlign) 
			wprintf(L"\tCompiled with /bzalign: no\n");
		else 
			wprintf(L"\tCompiled with /bzalign: yes\n");
	}

	BOOL fManagedPresent;
	if (pISymbol->get_hasManagedCode(&fManagedPresent) == S_OK)
	{
		if (fManagedPresent)
			wprintf(L"\tManaged code present: yes\n");
		else
			wprintf(L"\tManaged code present: no\n");
	}

	BOOL fSecurityChecks;
	if (pISymbol->get_hasSecurityChecks(&fSecurityChecks) == S_OK)
	{
		if (fSecurityChecks)
			wprintf(L"\tCompiled with /GS: yes\n");
		else
			wprintf(L"\tCompiled with /GS: no\n");
	}

	BOOL fSdl;
	if (pISymbol->get_isSdl(&fSdl) == S_OK)
	{
		if (fSdl)
			wprintf(L"\tCompiled with /sdl: yes\n");
		else
			wprintf(L"\tCompiled with /sdl: no\n");
	}

	BOOL fHotPatch;
	if (pISymbol->get_isHotpatchable(&fHotPatch) == S_OK)
	{
		if (fHotPatch)
			wprintf(L"\tCompiled with /hotpatch: yes\n");
		else
			wprintf(L"\tCompiled with /hotpatch: no\n");
	}

	BOOL fCVTCIL;
	if (pISymbol->get_isCVTCIL(&fCVTCIL) == S_OK)
	{
		if (fCVTCIL)
			wprintf(L"\tConverted by CVTCIL: yes\n");
		else 
			wprintf(L"\tConverted by CVTCIL: no\n");
	}

	BOOL fMSILModule;
	if (pISymbol->get_isMSILNetmodule(&fMSILModule) == S_OK)
	{
		if (fMSILModule)
			wprintf(L"\tMSIL module: yes\n");
		else
			wprintf(L"\tMSIL module: no\n");
	}

	DWORD dwVerMajor, dwVerMinor;
	DWORD dwVerBuild, dwVerQFE;
	if ((pISymbol->get_frontEndMajor(&dwVerMajor) == S_OK) &&
		(pISymbol->get_frontEndMinor(&dwVerMinor) == S_OK) &&
		(pISymbol->get_frontEndBuild(&dwVerBuild) == S_OK))
	{
		wprintf(L"\tFrontend Version: Major = %u, Minor = %u, Build = %u",
			dwVerMajor, dwVerMinor, dwVerBuild);
		if (pISymbol->get_frontEndQFE(&dwVerQFE) == S_OK)
			wprintf(L", QFE = %u", dwVerQFE);
		putwchar(L'\n');
	}

	if ((pISymbol->get_backEndMajor(&dwVerMajor) == S_OK) &&
		(pISymbol->get_backEndMinor(&dwVerMinor) == S_OK) &&
		(pISymbol->get_backEndBuild(&dwVerBuild) == S_OK)) 
	{
		wprintf(L"\tBackend Version: Major = %u, Minor = %u, Build = %u",
			dwVerMajor, dwVerMinor, dwVerBuild);
		if (pISymbol->get_backEndQFE(&dwVerQFE) == S_OK)
			wprintf(L", QFE = %u", dwVerQFE);
		putwchar(L'\n');
	}

	CComBSTR bstrCompilerName;
	if (pISymbol->get_compilerName(&bstrCompilerName) == S_OK) 
	{
		if (bstrCompilerName != NULL)
			wprintf(L"\tVersion string: %s", bstrCompilerName);
	}
	putwchar(L'\n');
}

////////////////////////////////////////////////////////////
// Print the compilan/module env
//
void PrintCompilandEnv(IDiaSymbol* pISymbol)
{
	PrintName(pISymbol);
	wprintf(L" =");

	CComVariant vt;
	if (pISymbol->get_value(&vt) == S_OK)
		PrintVariant(vt);
}

////////////////////////////////////////////////////////////
// Print a string corespondig to a location type
//
void PrintLocation(IDiaSymbol* pISymbol)
{
	DWORD dwLocType;
	DWORD dwRVA, dwSect, dwOff, dwReg, dwBitPos, dwSlot;
	LONG lOffset;
	ULONGLONG ulLen;

	if (pISymbol->get_locationType(&dwLocType) != S_OK)
	{
		wprintf(L"symbol in optmized code");
		return;
	}

	switch (dwLocType)
	{
		case LocIsStatic:
			if ((pISymbol->get_relativeVirtualAddress(&dwRVA) == S_OK) &&
				(pISymbol->get_addressSection(&dwSect) == S_OK) &&
				(pISymbol->get_addressOffset(&dwOff) == S_OK))
				wprintf(L"%s, [%08X][%04X:%08X]", 
					SafeDRef(GSZ_LOC_TYPES, dwLocType), dwRVA, dwSect, dwOff);
		break;
		case LocIsTLS:
			if ((pISymbol->get_addressSection(&dwSect) == S_OK) &&
				(pISymbol->get_addressOffset(&dwOff) == S_OK))
				wprintf(L"%s, [%04X:%08X]", 
					SafeDRef(GSZ_LOC_TYPES, dwLocType), dwRVA, dwSect, dwOff);
		break;
		case LocIsRegRel:
			if ((pISymbol->get_registerId(&dwReg) == S_OK) &&
				(pISymbol->get_offset(&lOffset) == S_OK))
				wprintf(L"%s, %S+0x%X", SafeDRef(GSZ_LOC_TYPES, dwLocType), 
					SzNameC7Reg((USHORT)dwReg, GetPlatormId(pISymbol)), lOffset);
		break;
		case LocIsThisRel:
			if (pISymbol->get_offset(&lOffset) == S_OK)
				wprintf(L"%s, this+0x%X", SafeDRef(GSZ_LOC_TYPES, dwLocType), lOffset);
		break;
		case LocIsBitField:
			if ((pISymbol->get_offset(&lOffset) == S_OK) &&
				(pISymbol->get_bitPosition(&dwBitPos) == S_OK) &&
				(pISymbol->get_length(&ulLen) == S_OK)) 
			wprintf(L"%s, this(bf)+0x%X, BitPos=%d, BitLen=%d", 
				SafeDRef(GSZ_LOC_TYPES, dwLocType), lOffset, dwBitPos, ulLen);
		break;
		case LocIsEnregistered:
			if (pISymbol->get_registerId(&dwReg) == S_OK)
				wprintf(L"%s, Register=%s", SafeDRef(GSZ_LOC_TYPES, dwLocType),
					SzNameC7Reg((USHORT)dwReg, GetPlatormId(pISymbol)));
		break;
		case LocIsConstant:
		{
			wprintf(L"%s, Value=", SafeDRef(GSZ_LOC_TYPES, dwLocType));
			CComVariant vt;
			if (pISymbol->get_value(&vt) == S_OK)
				PrintVariant(vt);
		}
		break;
		case LocIsSlot:
			if (pISymbol->get_slot(&dwSlot) == S_OK)
				wprintf(L"%s, Slot=%d", SafeDRef(GSZ_LOC_TYPES, dwLocType), dwSlot);
			break;
		case LocIsIlRel:
			if (pISymbol->get_offset(&lOffset) == S_OK)
				wprintf(L"%s, Offset=%d", SafeDRef(GSZ_LOC_TYPES, dwLocType), lOffset);
		break;
		case LocInMetaData:
			if (pISymbol->get_token(&dwSlot) == S_OK)
				wprintf(L"%s, Toke=0x%08X", SafeDRef(GSZ_LOC_TYPES, dwLocType), dwSlot);
		break;
		case LocIsNull:
		break;
		default :
			wprintf(L"Error - invalid location type: 0x%X", dwLocType);
		break;
	}
}

////////////////////////////////////////////////////////////
// Print the type, value and the name of a const symbol
//
void PrintConst(IDiaSymbol* pISymbol)
{
	PrintSymbolType(pISymbol);

	CComVariant vt;
	if (pISymbol->get_value(&vt) == S_OK) 
		PrintVariant(vt);
	PrintName(pISymbol);
}

////////////////////////////////////////////////////////////
// Print the name and the type of an user defined type
//
void PrintUDT(IDiaSymbol* pISymbol)
{
	PrintName(pISymbol);
	PrintSymbolType(pISymbol);
}

////////////////////////////////////////////////////////////
// Print a string representing the type of a symbol
//
void PrintSymbolType(IDiaSymbol* pISymbol)
{
	CComPtr<IDiaSymbol> pIType;
	if (pISymbol->get_type(&pIType) == S_OK)
	{
		wprintf(L", Type: ");
		PrintType(pIType);
		pIType = 0;
	}
}

////////////////////////////////////////////////////////////
// Print the information details for a type symbol
//
void PrintType(IDiaSymbol* pISymbol)
{
	CComPtr<IDiaSymbol> pIBaseType;
	CComPtr<IDiaEnumSymbols> pIEnumSym;
	CComPtr<IDiaSymbol> pISubSym;
	DWORD dwInfo;
	BOOL bSet;
	DWORD dwRank;
	LONG lCount = 0;
	ULONG celt = 1;

	DWORD dwTag = 0;
	if (pISymbol->get_symTag(&dwTag) != S_OK)
	{
		wprintf(L"ERROR - can't retrieve the symbol's SymTag\n");
		return;
	}

	CComBSTR bszName;
	pISymbol->get_name(&bszName);
	if (dwTag != SymTagPointerType)
	{
		if ((pISymbol->get_constType(&bSet) == S_OK) && bSet)
			wprintf(L"const ");
		if ((pISymbol->get_volatileType(&bSet) == S_OK) && bSet)
			wprintf(L"volatile ");
		if ((pISymbol->get_unalignedType(&bSet) == S_OK) && bSet)
			wprintf(L"__unaligned ");
	}

	ULONGLONG ulLen;
	pISymbol->get_length(&ulLen);
	switch (dwTag) 
	{
		case SymTagUDT:
			PrintUdtKind(pISymbol);
			PrintName(pISymbol);
		break;

		case SymTagEnum:
			wprintf(L"enum ");
			PrintName(pISymbol);
		break;

		case SymTagFunctionType:
			wprintf(L"function ");
		break;

		case SymTagPointerType:
			if (pISymbol->get_type(&pIBaseType) != S_OK)
			{
				wprintf(L"ERROR - SymTagPointerType get_type");
				return;
			}
			PrintType(pIBaseType);
			pIBaseType = 0;

			if ((pISymbol->get_reference(&bSet) == S_OK) && bSet)
				wprintf(L" &");
			else
				wprintf(L" *");

			if ((pISymbol->get_constType(&bSet) == S_OK) && bSet)
				wprintf(L" const");
			if ((pISymbol->get_volatileType(&bSet) == S_OK) && bSet)
				wprintf(L" volatile");
			if ((pISymbol->get_unalignedType(&bSet) == S_OK) && bSet)
				wprintf(L" __unaligned");
		break;

		case SymTagArrayType:
			if (pISymbol->get_type(&pIBaseType) == S_OK)
			{
				PrintType(pIBaseType);

				if (pISymbol->get_rank(&dwRank) == S_OK)
				{
					if (SUCCEEDED(pISymbol->findChildren
						(SymTagDimension, NULL, nsNone, &pIEnumSym)) && (pIEnumSym != NULL))
					{
						while (SUCCEEDED(pIEnumSym->Next(1, &pISubSym, &celt)) && (celt == 1))
						{
							CComPtr<IDiaSymbol> pIBound;
							wprintf(L"[");
							if (pISubSym->get_lowerBound(&pIBound) == S_OK)
							{
								PrintBound(pIBound);
								wprintf(L"..");
								pIBound = 0;
							}
							if (pISubSym->get_upperBound(&pIBound) == S_OK)
							{
								PrintBound(pIBound);
								pIBound = 0;
							}
							pISubSym = 0;
							wprintf(L"]");
						}
						pIEnumSym = 0;
					}
				}
				else if (SUCCEEDED(pISymbol->findChildren
					(SymTagCustomType, NULL, nsNone, &pIEnumSym)) && (pIEnumSym != NULL) &&
					(pIEnumSym->get_Count(&lCount) == S_OK) && (lCount > 0)) 
				{
					while (SUCCEEDED(pIEnumSym->Next(1, &pISubSym, &celt)) && (celt == 1))
					{
						wprintf(L"[");
						PrintType(pISubSym);
						wprintf(L"]");
						pISubSym = 0;
					}
					pIEnumSym = 0;
				}
				else
				{
					DWORD dwCountElems;
					ULONGLONG ulLenArray;
					ULONGLONG ulLenElem;

					if (pISymbol->get_count(&dwCountElems) == S_OK)
						wprintf(L"[0x%X]", dwCountElems);
					else if ((pISymbol->get_length(&ulLenArray) == S_OK) &&
							(pIBaseType->get_length(&ulLenElem) == S_OK))
					{
						if (ulLenElem == 0)
							wprintf(L"[0x%lX]", ulLenArray);
						else
							wprintf(L"[0x%lX]", ulLenArray/ulLenElem);
					}
				}
				pIBaseType = 0;
			}
			else
			{
				wprintf(L"ERROR - SymTagArrayType get_type\n");
				return;
			}
		break;

		case SymTagBaseType:
			if (pISymbol->get_baseType(&dwInfo) != S_OK)
			{
				wprintf(L"SymTagBaseType get_baseType\n");
				return;
			}
			switch (dwInfo)
			{
				case btUInt : wprintf(L"unsigned ");
				case btInt :
					switch (ulLen)
					{
						case 1:
							if (dwInfo == btInt) wprintf(L"signed ");
							wprintf(L"char");		break;
						case 2: wprintf(L"short");	 break;
						case 4: wprintf(L"int");	 break;
						case 8: wprintf(L"__int64"); break;
					}
					dwInfo = 0xFFFFFFFF;
				break;
				case btFloat :
					switch (ulLen) 
					{
						case 4: wprintf(L"float"); break;
						case 8: wprintf(L"double"); break;
					}
					dwInfo = 0xFFFFFFFF;
				break;
			}
			if (dwInfo == 0xFFFFFFFF)
				break;
			wprintf(L"%s", GSZ_BASE_TYPES[dwInfo]);
		break;

		case SymTagTypedef:
			PrintName(pISymbol);
		break;

		case SymTagCustomType:
		{
			DWORD idOEM, idOEMSym;
			DWORD cbData = 0;
			DWORD count;

			if (pISymbol->get_oemId(&idOEM) == S_OK)
				wprintf(L"OEMId = %X, ", idOEM);
			if (pISymbol->get_oemSymbolId(&idOEMSym) == S_OK)
				wprintf(L"SymbolId = %X, ", idOEMSym);
			if (pISymbol->get_types(0, &count, NULL) == S_OK)
			{
				IDiaSymbol** ppIDiaSyms = (IDiaSymbol**)_alloca(sizeof(IDiaSymbol*) * count);
				if (pISymbol->get_types(count, &count, ppIDiaSyms) == S_OK)
				{
					for (ULONG i = 0; i < count; i++)
					{
						PrintType(ppIDiaSyms[i]);
						ppIDiaSyms[i]->Release();
					}
				}
			}

			// print custom data
			if ((pISymbol->get_dataBytes(cbData, &cbData, NULL) == S_OK) && (cbData != 0))
			{
				wprintf(L", Data: ");

				PBYTE pbData = new BYTE[cbData];
				pISymbol->get_dataBytes(cbData, &cbData, pbData);
				for (ULONG i = 0; i < cbData; i++)
					wprintf(L"0x%02X ", pbData[i]);
				delete [] pbData;
			}
		}
		break;

		case SymTagData: // This really is member data, just print its location
			PrintLocation(pISymbol);
		break;
	}
}

////////////////////////////////////////////////////////////
// Print bound information
//
void PrintBound(IDiaSymbol* pISymbol)
{
	DWORD dwTag = 0, dwKind;
	if (pISymbol->get_symTag(&dwTag) != S_OK) 
	{
		wprintf(L"ERROR - PrintBound() get_symTag");
		return;
	}

	if (pISymbol->get_locationType(&dwKind) != S_OK) 
	{
		wprintf(L"ERROR - PrintBound() get_locationType");
		return;
	}

	if (dwTag == SymTagData && dwKind == LocIsConstant)
	{
		CComVariant vt;
		if (pISymbol->get_value(&vt) == S_OK) 
			PrintVariant(vt);
	}
	else
		PrintName(pISymbol);
}

////////////////////////////////////////////////////////////
//
void PrintData(IDiaSymbol* pISymbol, DWORD dwIndent)
{
	PrintLocation(pISymbol);

	DWORD dwDataKind;
	if (pISymbol->get_dataKind(&dwDataKind) != S_OK)
	{
		wprintf(L"ERROR - PrintData() get_dataKind");
		return;
	}
	wprintf(L", %s", SafeDRef(GSZ_DATA_KINDS, dwDataKind));
	PrintSymbolType(pISymbol);

	wprintf(L", ");
	PrintName(pISymbol);
}

////////////////////////////////////////////////////////////
// Print a VARIANT
//
void PrintVariant(VARIANT vt)
{
	switch (vt.vt)
	{
		case VT_UI1: case VT_I1:
			wprintf(L"0x%X", vt.bVal);
		break;
		case VT_I2: case VT_UI2: case VT_BOOL:
			wprintf(L"0x%X", vt.iVal);
		break;
		case VT_I4: case VT_UI4: 
		case VT_INT: case VT_UINT: case VT_ERROR:
			wprintf(L"0x%X", vt.lVal);
		break;
		case VT_R4:
			wprintf(L"%g", vt.fltVal);
		break;
		case VT_R8:
			wprintf(L"%g", vt.dblVal);
		break;
		case VT_BSTR:
			wprintf(L"\"%s\"", vt.bstrVal);
		break;
		default:
			wprintf(L"??");
		break;
	}
}

////////////////////////////////////////////////////////////
// Print a string corresponding to a UDT kind
//
void PrintUdtKind(IDiaSymbol* pISymbol)
{
	DWORD dwKind = 0;

	if (pISymbol->get_udtKind(&dwKind) == S_OK)
		wprintf(L"%s ", GSZ_UDT_KINDS[dwKind]);
}

////////////////////////////////////////////////////////////
// Print type informations is details
//
void PrintTypeInDetail(IDiaSymbol* pISymbol, DWORD dwIndent)
{
	CComPtr<IDiaEnumSymbols> pIEnumChild;
	CComPtr<IDiaSymbol> pIType;
	CComPtr<IDiaSymbol> pIChild;
	DWORD dwSymTag;
	DWORD dwSymTagType;
	ULONG celt = 0;
	BOOL bFlag;

	if (dwIndent > MAX_TYPE_IN_DETAIL)
		return;

	if (pISymbol->get_symTag(&dwSymTag) != S_OK)
	{
		wprintf(L"ERROR - PrintTypeInDetail() get_symTag\n");
		return;
	}

	PrintSymTag(dwSymTag);
	for (DWORD i = 0;i < dwIndent; i++)
		putwchar(L' ');

	switch (dwSymTag)
	{
		case SymTagData:
			PrintData(pISymbol, dwIndent);
			if (pISymbol->get_type(&pIType) == S_OK)
			{
				if (pIType->get_symTag(&dwSymTagType) == S_OK) 
				{
					if (dwSymTagType == SymTagUDT)
					{
						putwchar(L'\n');
						PrintTypeInDetail(pIType, dwIndent + 2);
					}
				}
				pIType = 0;
			}
		break;
		case SymTagTypedef:
		case SymTagVTable:
			PrintName(pISymbol);
			PrintSymbolType(pISymbol);
		break;
		case SymTagEnum:
		case SymTagUDT:
			PrintUDT(pISymbol);
			putwchar(L'\n');
			if (SUCCEEDED(pISymbol->findChildren
				(SymTagNull, NULL, nsNone, &pIEnumChild)))
			{
				while (SUCCEEDED(pIEnumChild->Next(1, &pIChild, &celt)) && (celt == 1))
				{
					PrintTypeInDetail(pIChild, dwIndent + 2);
					pIChild = 0;
				}
				pIEnumChild = 0;
			}
		return;
		case SymTagFunction:
			PrintFunctionType(pISymbol);
		return;
		case SymTagPointerType:
			PrintName(pISymbol);
			wprintf(L" has type ");
			PrintType(pISymbol);
		break;
		case SymTagArrayType:
		case SymTagBaseType:
		case SymTagFunctionArgType:
		case SymTagUsingNamespace:
		case SymTagCustom:
		case SymTagFriend:
			PrintName(pISymbol);
			PrintSymbolType(pISymbol);
		break;
		case SymTagVTableShape:
		case SymTagBaseClass:
			PrintName(pISymbol);
			if ((pISymbol->get_virtualBaseClass(&bFlag) == S_OK) && bFlag)
			{
				CComPtr<IDiaSymbol> pVBTableType;
				LONG ptrOffset;
				DWORD dispIndex;
				if ((pISymbol->get_virtualBaseDispIndex(&dispIndex) == S_OK) &&
					(pISymbol->get_virtualBasePointerOffset(&ptrOffset) == S_OK))
				{
					wprintf(L" virtual, offset = 0x%X, pointer offset = %ld, "
						L"virtual base pointer type = ", dispIndex, ptrOffset);
					if (pISymbol->get_virtualBaseTableType(&pVBTableType) == S_OK)
					{
						PrintType(pVBTableType);
						pVBTableType = 0;
					}
					else
						wprintf(L"(unknown)");
				}
			}
			else
			{
				LONG offset;
				if (pISymbol->get_offset(&offset) == S_OK)
					wprintf(L", offset = 0x%X", offset);
			}

			putwchar(L'\n');
			if (SUCCEEDED(pISymbol->findChildren(SymTagNull, NULL, nsNone, &pIEnumChild)))
			{
				while (SUCCEEDED(pIEnumChild->Next(1, &pIChild, &celt)) && (celt == 1))
				{
					PrintTypeInDetail(pIChild, dwIndent + 2);
					pIChild = 0;
				}
				pIEnumChild = 0;
			}
		break;
		case SymTagFunctionType:
			if (pISymbol->get_type(&pIType) == S_OK)
				PrintType(pIType);
		break;
		case SymTagThunk:
			// Happens for functions which only have S_PROCREF
			PrintThunk(pISymbol);
		break;
		default:
			wprintf(L"ERROR - PrintTypeInDetail() invalid SymTag\n");
		break;
	}

	putwchar(L'\n');
}

////////////////////////////////////////////////////////////
// Print a function type
//
void PrintFunctionType(IDiaSymbol* pISymbol)
{
	DWORD dwAccess = 0;
	if (pISymbol->get_access(&dwAccess) == S_OK)
		wprintf(L"%s ", SafeDRef(GSZ_ACCESS_SPECS, dwAccess));

	BOOL bIsStatic = FALSE;
	if ((pISymbol->get_isStatic(&bIsStatic) == S_OK) && bIsStatic)
		wprintf(L"static ");

	CComPtr<IDiaSymbol> pIFuncType;
	if (pISymbol->get_type(&pIFuncType) == S_OK) 
	{
		CComPtr<IDiaSymbol> pReturnType;
		if (pIFuncType->get_type(&pReturnType) == S_OK) 
		{
			PrintType(pReturnType);
			putwchar(L' ');

			CComBSTR bszName;
			if (pISymbol->get_name(&bszName) == S_OK)
				wprintf(L"%s", bszName);

			CComPtr<IDiaSymbol> pIObj;
			if (pIFuncType->get_objectPointerType(&pIObj) == S_OK)
			{
				wprintf(L", => ");
				PrintSymbol(pIObj, 0);
			}
			LONG llThis;
			if (pIFuncType->get_thisAdjust(&llThis) == S_OK)
			{
				wprintf(L",    %d", llThis);
			}

			CComPtr<IDiaEnumSymbols> pIEnumChild;
			if (SUCCEEDED(pIFuncType->findChildren
				(SymTagNull, NULL, nsNone, &pIEnumChild))) 
			{
				CComPtr<IDiaSymbol> pIChild;
				ULONG celt = 0;
				ULONG nParam = 0;

				wprintf(L"(");

				while (SUCCEEDED(pIEnumChild->Next(1, &pIChild, &celt)) && (celt == 1))
				{
					CComPtr<IDiaSymbol> pIType;
					if (pIChild->get_type(&pIType) == S_OK)
					{
						if (nParam++)
							wprintf(L", ");
						PrintType(pIType);
						pIType = 0;
					}
					pIChild = 0;
				}
				pIEnumChild = 0;
				wprintf(L")\n");
			}
			pReturnType = 0;
		}
		pIFuncType = 0;
	}
}

////////////////////////////////////////////////////////////
//
void PrintSourceFile(IDiaSourceFile *pSource)
{
	CComBSTR bstrSourceName;
	if (pSource->get_fileName(&bstrSourceName) == S_OK)
		wprintf(L"\t%s", bstrSourceName);
	else 
	{
		wprintf(L"ERROR - PrintSourceFile() get_fileName");
		return;
	}

	BYTE checksum[256];
	DWORD cbChecksum = sizeof(checksum);
	if (pSource->get_checksum(cbChecksum, &cbChecksum, checksum) == S_OK)
	{
		wprintf(L" (");

		DWORD checksumType;
		if (pSource->get_checksumType(&checksumType) == S_OK)
		{
			switch (checksumType)
			{
				case CHKSUM_TYPE_NONE	: wprintf(L"None")	; break;
				case CHKSUM_TYPE_MD5	: wprintf(L"MD5")	;break;
				case CHKSUM_TYPE_SHA1	: wprintf(L"SHA1")	;break;
				default					: wprintf(L"0x%X", checksumType); break;
			}
			if (cbChecksum != 0)
				wprintf(L": ");
		}

		for (DWORD ib = 0; ib < cbChecksum; ib++)
			wprintf(L"%02X", checksum[ib]);
		wprintf(L")");
	}
}

////////////////////////////////////////////////////////////
//
void PrintLines(IDiaSession* pISession, IDiaSymbol* pIFunc)
{
	DWORD dwSymTag;
	if ((pIFunc->get_symTag(&dwSymTag) != S_OK) || (dwSymTag != SymTagFunction))
	{
		wprintf(L"ERROR - PrintLines() dwSymTag != SymTagFunction");
		return;
	}

	CComBSTR bszName;
	if (pIFunc->get_name(&bszName) == S_OK)
		wprintf(L"\n** %s\n\n", bszName);

	ULONGLONG ulLength;
	if (pIFunc->get_length(&ulLength) != S_OK)
	{
		wprintf(L"ERROR - PrintLines() get_length");
		return;
	}

	DWORD dwRVA;
	IDiaEnumLineNumbers *pLines;
	if (pIFunc->get_relativeVirtualAddress(&dwRVA) == S_OK)
	{
		if (SUCCEEDED(pISession->findLinesByRVA
			(dwRVA, static_cast<DWORD>(ulLength), &pLines)))
		{
			PrintLines(pLines);
			pLines->Release();
		}
	}
	else
	{
		DWORD dwSect, dwOffset;
		if ((pIFunc->get_addressSection(&dwSect) == S_OK) &&
			(pIFunc->get_addressOffset(&dwOffset) == S_OK))
		{
			if (SUCCEEDED(pISession->findLinesByAddr
				(dwSect, dwOffset, static_cast<DWORD>(ulLength), &pLines)))
			{
				PrintLines(pLines);
				pLines->Release();
			}
		}
	}
}

////////////////////////////////////////////////////////////
//
void PrintLines(IDiaEnumLineNumbers *pLines)
{
	IDiaLineNumber *pLine;
	DWORD celt;
	DWORD dwRVA;
	DWORD dwSeg;
	DWORD dwOffset;
	DWORD dwLinenum;
	DWORD dwSrcId;
	DWORD dwLength;

	DWORD dwSrcIdLast = (DWORD)(-1);
	while (SUCCEEDED(pLines->Next(1, &pLine, &celt)) && (celt == 1))
	{
		if ((pLine->get_relativeVirtualAddress(&dwRVA) == S_OK) &&
			(pLine->get_addressSection(&dwSeg) == S_OK) &&
			(pLine->get_addressOffset(&dwOffset) == S_OK) &&
			(pLine->get_lineNumber(&dwLinenum) == S_OK) &&
			(pLine->get_sourceFileId(&dwSrcId) == S_OK) &&
			(pLine->get_length(&dwLength) == S_OK))
		{
			wprintf(L"\tline %u at [%08X][%04X:%08X], len = 0x%X", 
				dwLinenum, dwRVA, dwSeg, dwOffset, dwLength);

			if (dwSrcId != dwSrcIdLast)
			{
				IDiaSourceFile *pSource;
				if (pLine->get_sourceFile(&pSource) == S_OK)
				{
					PrintSourceFile(pSource);
					dwSrcIdLast = dwSrcId;
					pSource->Release();
				}
			}
			pLine->Release();
			putwchar(L'\n');
		}
	}
}

////////////////////////////////////////////////////////////
// Print the section contribution data: name, Sec::Off, length
void PrintSecContribs(IDiaSession* pISession, IDiaSectionContrib* pISegment)
{
	DWORD dwRVA, dwSect, dwOffset, dwLen;
	CComPtr<IDiaSymbol> pICompiland;
	CComBSTR bszName;

	if ((pISegment->get_relativeVirtualAddress(&dwRVA) == S_OK) &&
		(pISegment->get_addressSection(&dwSect) == S_OK) &&
		(pISegment->get_addressOffset(&dwOffset) == S_OK) &&
		(pISegment->get_length(&dwLen) == S_OK) &&
		(pISegment->get_compiland(&pICompiland) == S_OK) &&
		(pICompiland->get_name(&bszName) == S_OK))
	{
		wprintf(L"  %08X  %04X:%08X  %08X  %s\n", dwRVA, dwSect, dwOffset, dwLen, bszName);

		CComPtr<IDiaSymbol> pISymbol;
		if (pISession->findSymbolByRVA(dwRVA, SymTagNull, &pISymbol) == S_OK)
		{
			DWORD tag;
			if ((pISymbol->get_symTag(&tag) == S_OK) && (pISymbol->get_name(&bszName) == S_OK))
				wprintf(L"      => %s, %s\n", GSZ_SYM_TAGS[tag], bszName);
			pISymbol = 0;
		}
		pICompiland = 0;
	}
}

////////////////////////////////////////////////////////////
// Print a debug stream data
//
void PrintStreamData(IDiaEnumDebugStreamData* pIStream)
{
	CComBSTR bszName;
	if (pIStream->get_name(&bszName) != S_OK)
		wprintf(L"ERROR - PrintStreamData() get_name\n");
	else
		wprintf(L"Stream: %s", bszName);

	LONG dwElem;
	if (pIStream->get_Count(&dwElem) != S_OK)
		wprintf(L"ERROR - PrintStreamData() get_Count\n");
	else
		wprintf(L"(%u)\n", dwElem);

	DWORD cbTotal = 0;
	BYTE data[1024];
	DWORD cbData;
	ULONG celt = 0;
	while (SUCCEEDED(pIStream->Next(1, sizeof(data), &cbData, (BYTE*)&data, &celt)) && (celt == 1))
	{
		for (DWORD i = 0; i < cbData; i++)
		{
			wprintf(L"%02X ", data[i]);
			if (i && (i % 8 == 7) && (i+1 < cbData))
				wprintf(L"- ");
		}
		wprintf(L"| ");

		for (DWORD i = 0; i < cbData; i++)
			wprintf(L"%c", iswprint(data[i]) ? data[i] : '.');
		putwchar(L'\n');
		cbTotal += cbData;
	}

	wprintf(L"Summary :\n\tNo of Elems = %u\n", dwElem);
	if (dwElem != 0)
		wprintf(L"\tSizeof(Elem) = %u\n", cbTotal / dwElem);
	putwchar(L'\n');
}

////////////////////////////////////////////////////////////
// Print the FPO info for a given symbol;
//
void PrintFrameData(IDiaFrameData *pFrameData)
{
	DWORD dwSect;
	DWORD dwOffset;
	DWORD cbBlock;
	DWORD cbLocals;     // Number of bytes reserved for the function locals
	DWORD cbParams;     // Number of bytes reserved for the function arguments
	DWORD cbMaxStack;
	DWORD cbProlog;
	DWORD cbSavedRegs;
	BOOL bSEH;
	BOOL bEH;
	BOOL bStart;

	if ((pFrameData->get_addressSection(&dwSect) == S_OK) &&
		(pFrameData->get_addressOffset(&dwOffset) == S_OK) &&
		(pFrameData->get_lengthBlock(&cbBlock) == S_OK) &&
		(pFrameData->get_lengthLocals(&cbLocals) == S_OK) &&
		(pFrameData->get_lengthParams(&cbParams) == S_OK) &&
		(pFrameData->get_maxStack(&cbMaxStack) == S_OK) &&
		(pFrameData->get_lengthProlog(&cbProlog) == S_OK) &&
		(pFrameData->get_lengthSavedRegisters(&cbSavedRegs) == S_OK) &&
		(pFrameData->get_systemExceptionHandling(&bSEH) == S_OK) &&
		(pFrameData->get_cplusplusExceptionHandling(&bEH) == S_OK) &&
		(pFrameData->get_functionStart(&bStart) == S_OK))
	{
		wprintf(L"%04X:%08X   %8X %8X %8X %8X %8X %8X %c   %c   %c",
				dwSect, dwOffset, cbBlock, cbLocals, cbParams, cbMaxStack, cbProlog, cbSavedRegs,
				bSEH ? L'Y' : L'N',
				bEH ? L'Y' : L'N',
				bStart ? L'Y' : L'N');

		CComBSTR bsZProgram;
		if (pFrameData->get_program(&bsZProgram) == S_OK)
			wprintf(L" %s", bsZProgram);
		putwchar(L'\n');
	}
}

////////////////////////////////////////////////////////////
// Print all the valid properties associated to a symbol
//
void PrintPropertyStorage(IDiaPropertyStorage *pIPropStorage)
{
	CComPtr<IEnumSTATPROPSTG> pIEnumProps;
	if (SUCCEEDED(pIPropStorage->Enum(&pIEnumProps)))
	{
		STATPROPSTG prop;
		DWORD celt = 1;
		while (SUCCEEDED(pIEnumProps->Next(celt, &prop, &celt)) && (celt == 1)) 
		{
			PROPSPEC pspec = { PRSPEC_PROPID, prop.propid };
			PROPVARIANT vt = { VT_EMPTY };
			if (SUCCEEDED(pIPropStorage->ReadMultiple(1, &pspec, &vt))) 
			{
				switch (vt.vt)
				{
					case VT_BOOL:
						wprintf(L"%32s:\t %s\n", prop.lpwstrName, vt.bVal ? L"true" : L"false");
					break;
					case VT_I2:
						wprintf(L"%32s:\t %d\n", prop.lpwstrName, vt.iVal);
					break;
					case VT_UI2:
						wprintf(L"%32s:\t %u\n", prop.lpwstrName, vt.uiVal);
					break;
					case VT_I4:
						wprintf(L"%32s:\t %d\n", prop.lpwstrName, vt.intVal);
					break;
					case VT_UI4:
						wprintf(L"%32s:\t 0x%0X\n", prop.lpwstrName, vt.uintVal);
					break;
					case VT_UI8:
						wprintf(L"%32s:\t 0x%X\n", prop.lpwstrName, vt.uhVal.QuadPart);
					break;
					case VT_BSTR:
						wprintf(L"%32s:\t %s\n", prop.lpwstrName, vt.bstrVal);
					break;
					case VT_UNKNOWN:
						wprintf(L"%32s:\t %p\n", prop.lpwstrName, vt.punkVal);
					break;
					case VT_SAFEARRAY:
					break;
				}
				VariantClear((VARIANTARG *) &vt);
			}
			SysFreeString(prop.lpwstrName);
		}
		pIEnumProps = 0;
	}
}

DWORD GetPlatormId(IDiaSymbol* pISymbol)
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
				case IMAGE_FILE_MACHINE_IA64 : dwCpuId = CV_CFL_IA64 ; break;
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