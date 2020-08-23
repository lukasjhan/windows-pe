#include "stdafx.h"

typedef enum SymTagEnum SYM_TAGS;
// Tags returned by Dia
PCWSTR SZ_DIA_TAGS[] =
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
	L"UserDefinedType",			// SymTagUDT
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

void PrintSubSyms(IDiaSymbol* pISym, int nDepth)
{
	CComBSTR bszName = L"NO_NAME";
	pISym->get_name(&bszName);
	SYM_TAGS eSymTag = SYM_TAGS::SymTagNull;
	pISym->get_symTag((PDWORD)&eSymTag);
	CString szName = bszName;
	szName.Replace(L"<", L"&lt;");
	szName.Replace(L">", L"&gt;");

	//char szTabs[16];
	//for (int i = 0; i < nDepth; i++)
	//	szTabs[i] = '\t';
	//szTabs[nDepth] = 0;
	//printf("%sTags : %S\n", szTabs, SZ_DIA_TAGS[eSymTag]);
	//printf("%sName : %S\n", szTabs, bszName);
	for (int i = 0; i < nDepth; i++)
		printf("\t");
	printf("<Symbol Tag=\"%S\" Name=\"%S\"", SZ_DIA_TAGS[eSymTag], szName);

	int nSubCnt = 0;
	CComPtr<IDiaEnumSymbols> pIEnumSyms;
	HRESULT hr = pISym->findChildren(SymTagNull, NULL, nsNone, &pIEnumSyms);
	if (hr == S_OK)
	{
		nDepth++;

		CComPtr<IDiaSymbol> pISubSym;
		ULONG uCelt = 0, uModIdx = 1;
		while (SUCCEEDED(pIEnumSyms->Next(1, &pISubSym, &uCelt)) && (uCelt == 1))
		{
			if (nSubCnt == 0)
			{
				printf(">\n");
			}
			PrintSubSyms(pISubSym, nDepth);
			pISubSym = 0;
			nSubCnt++;
		}
		pIEnumSyms = 0;
	}
	if (nSubCnt > 0)
	{
		for (int i = 0; i < nDepth-1; i++)
			printf("\t");
		printf("</Symbol>\n");
	}
	else
		printf("/>\n");
}

void _tmain()
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
		return;

	CComPtr<IDiaDataSource>	pISrc;
	CComPtr<IDiaSession>	pISess;
	CComPtr<IDiaSymbol>		pIGS;
	try
	{
		hr = CoCreateInstance(__uuidof(DiaSource), NULL, CLSCTX_INPROC_SERVER,
			__uuidof(IDiaDataSource), (PVOID*)&pISrc);
		if (FAILED(hr))
			throw hr;

		hr = pISrc->loadDataFromPdb(L"Z:\\0.DevMune\\1.Books\\01.PE.For.64\\02.srcs\\Sample\\0.bin\\x64\\Debug\\PdbTest.pdb");
		if (FAILED(hr))
			throw hr;

		hr = pISrc->openSession(&pISess);
		if (FAILED(hr))
			throw hr;

		hr = pISess->get_globalScope(&pIGS);
		if (hr != S_OK)
			throw hr;
		//CComBSTR bszName = L"NO_NAME";
		//pIGS->get_name(&bszName);
		//SYM_TAGS eSymTag = SYM_TAGS::SymTagNull;
		//pIGS->get_symTag((PDWORD)&eSymTag);
		//printf("Name : %S\nTag  : %S\n\n", bszName, SZ_DIA_TAGS[eSymTag]);

		PrintSubSyms(pIGS, 0);
	}
	catch (HRESULT e)
	{
		printf("Error occurred => 0x%08X\n", e);
	}
	pIGS = 0;
	pISess = 0;
	pISrc = 0;
	CoUninitialize();
}

