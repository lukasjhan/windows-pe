#include "stdafx.h"
#include "PdbDoc.h"
#include "PdbListView.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif


void CPdbListView::ParseSymbolList(IDiaSymbol* pISymbol, int nItemIdx)
{
	CComPtr<IDiaSymbol> pIType;
	if (pISymbol->get_type(&pIType) == S_OK)
	{
		CString szType = PEPdb::ToStr_Type(pIType);
		GetListCtrl().SetItem(nItemIdx, 2, TVIF_TEXT, szType, 0, 0, 0, 0);
		pIType = 0;
	}
}

void CPdbListView::ParseUdtList(IDiaSymbol* pISymbol, int nItemIdx)
{
	UdtKind eKind;
	pISymbol->get_udtKind((PDWORD)&eKind);
	GetListCtrl().SetItem(nItemIdx, 2, TVIF_TEXT, PEPdb::GSZ_UDT_KINDS[eKind], 0, 0, 0, 0);

	ULONGLONG ulLen = 0;
	pISymbol->get_length(&ulLen);
	CString szVal; szVal.Format(L"%d", ulLen);
	GetListCtrl().SetItem(nItemIdx, 3, TVIF_TEXT, szVal, 0, 0, 0, 0);

	CComBSTR bszName;
	ULONG	uCelt = 0;
	CString szBase;
	CComPtr<IDiaEnumSymbols> pIEnumUps;
	if (SUCCEEDED(pISymbol->findChildren(SymTagBaseClass, NULL, nsNone, &pIEnumUps)))
	{
		CComPtr<IDiaSymbol> pIUp;
		while (SUCCEEDED(pIEnumUps->Next(1, &pIUp, &uCelt)) && (uCelt == 1))
		{
			if (!szBase.IsEmpty())
				szBase.Append(L", ");
			if (pIUp->get_name(&bszName) == S_OK)
				szBase.Append(bszName);
			pIUp = 0;
		}
		pIEnumUps = 0;
	}
	if (SUCCEEDED(pISymbol->findChildren(SymTagBaseInterface, NULL, nsNone, &pIEnumUps)))
	{
		CComPtr<IDiaSymbol> pIUp;
		while (SUCCEEDED(pIEnumUps->Next(1, &pIUp, &uCelt)) && (uCelt == 1))
		{
			if (!szBase.IsEmpty())
				szBase.Append(L", ");
			if (pIUp->get_name(&bszName) == S_OK)
				szBase.Append(bszName);
			pIUp = 0;
		}
		pIEnumUps = 0;
	}
	if (!szBase.IsEmpty())
		GetListCtrl().SetItem(nItemIdx, 4, TVIF_TEXT, szBase, 0, 0, 0, 0);

	CString szFlags;
	BOOL bIsSet = FALSE;
	if (pISymbol->get_constructor(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|HasConstructor");
	if (pISymbol->get_constType(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|Const");
	if (pISymbol->get_nested(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|Nested");
	if (pISymbol->get_packed(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|Packed");
	if (pISymbol->get_scoped(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|NoGlobal");
	if (pISymbol->get_unalignedType(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|Unaligned");
	if (pISymbol->get_volatileType(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|Volatile");
	if (pISymbol->get_hasAssignmentOperator(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|HasAssignmentOp");
	if (pISymbol->get_hasCastOperator(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|HasCastOp");
	if (pISymbol->get_hasNestedTypes(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|HasNested");
	if (pISymbol->get_overloadedOperator(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|HasOverloadeOp");
	if (!szFlags.IsEmpty())
		szFlags = szFlags.Right(szFlags.GetLength() - 1);
	GetListCtrl().SetItem(nItemIdx, 5, TVIF_TEXT, szFlags, 0, 0, 0, 0);
}

void CPdbListView::ParseTypedDefList(IDiaSymbol* pISymbol, int nItemIdx)
{
	CString sz;
	CComPtr<IDiaSymbol> pIType;
	if (pISymbol->get_type(&pIType) == S_OK)
	{
		sz = PEPdb::ToStr_Type(pIType);
		GetListCtrl().SetItem(nItemIdx, 2, TVIF_TEXT, sz, 0, 0, 0, 0);
		pIType = 0;
	}
	ULONGLONG ulLen = 0;
	if (pISymbol->get_length(&ulLen) == S_OK)
	{
		sz.Format(L"%d", ulLen);
		GetListCtrl().SetItem(nItemIdx, 3, TVIF_TEXT, sz, 0, 0, 0, 0);
	}
	BOOL bFlgs = FALSE;
	if (pISymbol->get_nested(&bFlgs) == S_OK && bFlgs)
	{
		GetListCtrl().SetItem(nItemIdx, 4, TVIF_TEXT, L"¡Ü", 0, 0, 0, 0);
	}
}

void CPdbListView::ParseEnumList(IDiaSymbol* pISymbol, int nItemIdx)
{
	CComPtr<IDiaSymbol> pIType;
	if (pISymbol->get_type(&pIType) == S_OK)
	{
		CString szType = PEPdb::ToStr_Type(pIType);
		GetListCtrl().SetItem(nItemIdx, 2, TVIF_TEXT, szType, 0, 0, 0, 0);
		pIType = 0;
	}
	BOOL bFlgs = FALSE;
	pISymbol->get_nested(&bFlgs);
	if (bFlgs)
		GetListCtrl().SetItem(nItemIdx, 3, TVIF_TEXT, L"Nested", 0, 0, 0, 0);
}

void CPdbListView::ParseFunctionList(IDiaSymbol* pISymbol, int nItemIdx)
{
	CString szVal;
	DWORD dwRVA, dwSect, dwOffset;
	if ((pISymbol->get_relativeVirtualAddress(&dwRVA) == S_OK) &&
		(pISymbol->get_addressSection(&dwSect) == S_OK) &&
		(pISymbol->get_addressOffset(&dwOffset) == S_OK))
	{
		szVal.Format(L"0x%08X", dwRVA);
		GetListCtrl().SetItem(nItemIdx, 2, TVIF_TEXT, szVal, 0, 0, 0, 0);
		szVal.Format(L"%04X:%08X", dwSect, dwOffset);
		GetListCtrl().SetItem(nItemIdx, 3, TVIF_TEXT, szVal, 0, 0, 0, 0);
	}

	ULONGLONG ulSize = 0;
	pISymbol->get_length(&ulSize);
	szVal.Format(L"%d", ulSize);
	GetListCtrl().SetItem(nItemIdx, 4, TVIF_TEXT, szVal, 0, 0, 0, 0);
	//DWORD dwAccess = 0;
	//if (pISymbol->get_access(&dwAccess) == S_OK)
	//{
	//	CV_access_e cve = (CV_access_e)dwAccess;
	//	GetListCtrl().SetItem(nItemIdx, 4, TVIF_TEXT, CPdbDoc::GSZ_ACCESS_SPECS[cve], 0, 0, 0, 0);
	//}

	CString szFlags;
	BOOL bIsSet = FALSE;
	if (pISymbol->get_constType(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|const");
	if (pISymbol->get_isStatic(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|Static");
	if (pISymbol->get_pure(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|pure");
	if (pISymbol->get_virtual(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|virtual");
	if (pISymbol->get_volatileType(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|volatile");
	if (pISymbol->get_customCallingConvention(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|customCallingConv");
	if (pISymbol->get_farReturn(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|farReturn");
	if (pISymbol->get_hasAlloca(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|Alloca");
	if (pISymbol->get_hasEH(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|EH");
	if (pISymbol->get_hasEHa(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|EHa");
	if (pISymbol->get_hasInlAsm(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|InlAsm");
	if (pISymbol->get_hasLongJump(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|LongJump");
	if (pISymbol->get_hasSecurityChecks(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|SecurityChecks");
	if (pISymbol->get_hasSEH(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|SEH");
	if (pISymbol->get_hasSetJump(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|SetJump");
	if (pISymbol->get_interruptReturn(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|interruptReturn");
	if (pISymbol->get_intro(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|intro");
	if (pISymbol->get_inlSpec(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|inline");
	if (pISymbol->get_isNaked(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|naked");
	if (pISymbol->get_noInline(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|noInline");
	if (pISymbol->get_notReached(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|notReached");
	if (pISymbol->get_noReturn(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|noReturn");
	if (pISymbol->get_noStackOrdering(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|noStackOrdering");
	if (pISymbol->get_optimizedCodeDebugInfo(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|optimized");
	if (pISymbol->get_unalignedType(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|unaligned");
	if (!szFlags.IsEmpty())
		szFlags = szFlags.Right(szFlags.GetLength() - 1);
	GetListCtrl().SetItem(nItemIdx, 5, TVIF_TEXT, szFlags, 0, 0, 0, 0);

	CComPtr<IDiaSymbol> pIType;
	if (pISymbol->get_type(&pIType) == S_OK)
	{
		CComBSTR bszName;
		//if (pISymbol->get_name(&bszName) == S_OK)
		pISymbol->get_name(&bszName);
		CString szType = PEPdb::ToStr_FunctionType(pIType);
		GetListCtrl().SetItem(nItemIdx, 6, TVIF_TEXT, szType, 0, 0, 0, 0);
		pIType = 0;
	}
}

void CPdbListView::ParseDataList(IDiaSymbol* pISymbol, int nItemIdx)
{
	CString szVal;
	DWORD dwDataKind = 0;
	ULONGLONG ulLen;
	if (pISymbol->get_dataKind(&dwDataKind) == S_OK)
	{
		//if (dwDataKind != DataKind::DataIsFileStatic && dwDataKind != DataKind::DataIsGlobal)
		//	return;
		GetListCtrl().SetItem(nItemIdx, 2, TVIF_TEXT, PEPdb::GSZ_DATA_KINDS[dwDataKind], 0, 0, 0, 0);
		CComPtr<IDiaSymbol> pIType;
		if (pISymbol->get_type(&pIType) == S_OK)
		{
			GetListCtrl().SetItem(nItemIdx, 3, TVIF_TEXT, PEPdb::ToStr_Type(pIType), 0, 0, 0, 0);
			if (pIType->get_length(&ulLen) == S_OK)
			{
				szVal.Format(L"%d (0x%x)", ulLen, ulLen);
				GetListCtrl().SetItem(nItemIdx, 4, TVIF_TEXT, szVal, 0, 0, 0, 0);
			}
			pIType = 0;
		}
	}


	DWORD dwLocType;
	if (pISymbol->get_locationType(&dwLocType) != S_OK)
	{
		GetListCtrl().SetItem(nItemIdx, 5, TVIF_TEXT, L"Optmized Symbol", 0, 0, 0, 0);
		return;
	}
	GetListCtrl().SetItem(nItemIdx, 5, TVIF_TEXT, PEPdb::GSZ_LOC_TYPES[dwLocType], 0, 0, 0, 0);

	DWORD dwRVA, dwSect, dwOff, dwReg, dwBitPos, dwSlot;
	LONG  lOffset;
	CString	szLoc;
	switch (dwLocType)
	{
		case LocIsStatic:
		case LocIsTLS:
		case LocInMetaData:
		case LocIsIlRel:
			if ((pISymbol->get_relativeVirtualAddress(&dwRVA) == S_OK) &&
				(pISymbol->get_addressSection(&dwSect) == S_OK) &&
				(pISymbol->get_addressOffset(&dwOff) == S_OK))
			{
				szVal.Format(L"0x%08X", dwRVA);
				GetListCtrl().SetItem(nItemIdx, 6, TVIF_TEXT, szVal, 0, 0, 0, 0);
				szVal.Format(L"%04X:%08X", dwSect, dwOff);
				GetListCtrl().SetItem(nItemIdx, 7, TVIF_TEXT, szVal, 0, 0, 0, 0);
			}
			break;
		case LocIsRegRel:
			if ((pISymbol->get_registerId(&dwReg) == S_OK) &&
				(pISymbol->get_offset(&lOffset) == S_OK))
			{
				GetListCtrl().SetItem(nItemIdx, 6, TVIF_TEXT, 
					PEPdb::ToStr_C7REG((USHORT)dwReg, CPdbDoc::MACHINE_TYPE), 0, 0, 0, 0);
				szVal.Format(L"+%X", dwSect, dwOff);
				GetListCtrl().SetItem(nItemIdx, 7, TVIF_TEXT, szVal, 0, 0, 0, 0);
			}
			break;
		case LocIsThisRel:
			GetListCtrl().SetItem(nItemIdx, 6, TVIF_TEXT, L"this", 0, 0, 0, 0);
			if (pISymbol->get_offset(&lOffset) == S_OK)
			{
				szVal.Format(L"+%X", dwSect, dwOff);
				GetListCtrl().SetItem(nItemIdx, 7, TVIF_TEXT, szVal, 0, 0, 0, 0);
			}
			break;
		case LocIsBitField:
			if ((pISymbol->get_offset(&lOffset) == S_OK) &&
				(pISymbol->get_bitPosition(&dwBitPos) == S_OK) &&
				(pISymbol->get_length(&ulLen) == S_OK))
				szLoc.AppendFormat(L"%s, this(bf)+0x%X:0x%X len(0x%X)",
				PEPdb::GSZ_LOC_TYPES[dwLocType], lOffset, dwBitPos, ulLen);
			break;
		case LocIsEnregistered:
			if (pISymbol->get_registerId(&dwReg) == S_OK)
				GetListCtrl().SetItem(nItemIdx, 6, TVIF_TEXT, 
				PEPdb::ToStr_C7REG((USHORT)dwReg, CPdbDoc::MACHINE_TYPE), 0, 0, 0, 0);
			break;
		case LocIsSlot:
			GetListCtrl().SetItem(nItemIdx, 6, TVIF_TEXT, L"#No.", 0, 0, 0, 0);
			if (pISymbol->get_slot(&dwSlot) == S_OK)
			{
				szVal.Format(L"%08%X", dwSlot);
				GetListCtrl().SetItem(nItemIdx, 7, TVIF_TEXT, szVal, 0, 0, 0, 0);
			}
			break;
		case LocIsConstant:
		{
			GetListCtrl().SetItem(nItemIdx, 6, TVIF_TEXT, L"value", 0, 0, 0, 0);
			CComVariant vt;
			if (pISymbol->get_value(&vt) == S_OK)
				GetListCtrl().SetItem(nItemIdx, 7, TVIF_TEXT, PEPdb::ToStr_Variant(&vt), 0, 0, 0, 0);
		}
		break;
	}
}

void CPdbListView::ParsePublicSymbolList(IDiaSymbol* pISymbol, int nItemIdx)
{
	CString sz;
	ULONGLONG ulLen;
	if (pISymbol->get_length(&ulLen) == S_OK)
	{
		sz.Format(L"%d(0x%x)", ulLen, ulLen);
		GetListCtrl().SetItem(nItemIdx, 3, TVIF_TEXT, sz, 0, 0, 0, 0);
	}

	LocationType loc = LocationType::LocIsNull;
	pISymbol->get_locationType((PDWORD)&loc);
	GetListCtrl().SetItem(nItemIdx, 4, TVIF_TEXT, PEPdb::GSZ_LOC_TYPES[loc], 0, 0, 0, 0);

	sz.Empty();
	BOOL bCheck = FALSE;
	if(pISymbol->get_code(&bCheck) == S_OK && bCheck)
		sz = "Code";
	if (pISymbol->get_function(&bCheck) == S_OK && bCheck)
	{
		if (!sz.IsEmpty())
			sz.Append(L"|");
		sz.Append(L"Function");
	}
	CComBSTR bszUudeco;
	if (pISymbol->get_undecoratedName(&bszUudeco) == S_OK)
	{
		//pIBase->findChildren(SymTagPublicSymbol, L"??*", nsRegularExpression, &pIEnumSyms);
		if (wcscmp(bszUudeco, L"`string'") == 0)
		{
			if (!sz.IsEmpty())
				sz.Append(L"|");
			sz.Append(L"String");
		}
	}
	GetListCtrl().SetItem(nItemIdx, 2, TVIF_TEXT, sz, 0, 0, 0, 0);

	DWORD dwRVA, dwSect, dwOff;
	if (pISymbol->get_relativeVirtualAddress(&dwRVA) == S_OK)
	{
		sz.Format(L"0x%08X", dwRVA);
		GetListCtrl().SetItem(nItemIdx, 5, TVIF_TEXT, sz, 0, 0, 0, 0);
	}
	if (pISymbol->get_addressSection(&dwSect) == S_OK &&
		pISymbol->get_addressOffset(&dwOff) == S_OK)
	{
		sz.Format(L"%d:%08X", dwSect, dwOff);
		GetListCtrl().SetItem(nItemIdx, 6, TVIF_TEXT, sz, 0, 0, 0, 0);
	}
}

void CPdbListView::ParseThunkList(IDiaSymbol* pISymbol, int nItemIdx)
{
	CString szVal;
	DWORD dwRVA, dwSect, dwOffset;
	if ((pISymbol->get_relativeVirtualAddress(&dwRVA) == S_OK) &&
		(pISymbol->get_addressSection(&dwSect) == S_OK) &&
		(pISymbol->get_addressOffset(&dwOffset) == S_OK))
	{
		szVal.Format(L"0x%08X", dwRVA);
		GetListCtrl().SetItem(nItemIdx, 2, TVIF_TEXT, szVal, 0, 0, 0, 0);
		szVal.Format(L"%04X:%08X", dwSect, dwOffset);
		GetListCtrl().SetItem(nItemIdx, 3, TVIF_TEXT, szVal, 0, 0, 0, 0);
	}
	ULONGLONG ulSize = 0;
	pISymbol->get_length(&ulSize);
	szVal.Format(L"%d", ulSize);
	GetListCtrl().SetItem(nItemIdx, 4, TVIF_TEXT, szVal, 0, 0, 0, 0);

	if ((pISymbol->get_targetSection(&dwSect) == S_OK) &&
		(pISymbol->get_targetOffset(&dwOffset) == S_OK) &&
		(pISymbol->get_targetRelativeVirtualAddress(&dwRVA) == S_OK))
	{
		szVal.Format(L"[%08X][%04X:%08X] ", dwRVA, dwSect, dwOffset);
		GetListCtrl().SetItem(nItemIdx, 5, TVIF_TEXT, szVal, 0, 0, 0, 0);
	}
	else
		GetListCtrl().SetItem(nItemIdx, 5, TVIF_TEXT, PEPdb::ToStr_Name(pISymbol), 0, 0, 0, 0);
}

void CPdbListView::ParseCompliandList(IDiaSymbol* pISymbol, int nItemIdx)
{
	CComBSTR bszName;
	if (pISymbol->get_name(&bszName) == S_OK)
	{
		PWSTR pPos = _tcsrchr(bszName, L'\\');
		if (pPos != NULL)
		{
			WCHAR szPath[MAX_PATH];
			wcsncpy_s(szPath, bszName, pPos - bszName);
			szPath[pPos - bszName] = 0;
			GetListCtrl().SetItem(nItemIdx, 2, TVIF_TEXT, szPath, 0, 0, 0, 0);
		}
	}

	ULONG uCelt = 0;
	CComPtr<IDiaEnumSymbols> pIEnumUps;
	HRESULT hr = pISymbol->findChildren(SymTagCompilandDetails, NULL, nsNone, &pIEnumUps);
	if (SUCCEEDED(hr))
	{
		CComPtr<IDiaSymbol> pIDetail;
		while (SUCCEEDED(pIEnumUps->Next(1, &pIDetail, &uCelt)) && (uCelt == 1))
		{
			DWORD dwLanguage;
			if (pIDetail->get_language(&dwLanguage) == S_OK)
				GetListCtrl().SetItem(nItemIdx, 3, TVIF_TEXT, PEPdb::GSZ_LANG_SPECS[dwLanguage], 0, 0, 0, 0);

			CComBSTR bszCpName;
			if (pIDetail->get_compilerName(&bszCpName) == S_OK)
				GetListCtrl().SetItem(nItemIdx, 4, TVIF_TEXT, bszCpName, 0, 0, 0, 0);
			pIDetail = 0;
		}
		pIEnumUps = 0;
	}
	//CString szEnv;
	//ULONG uCelt = 0;
	//CComPtr<IDiaEnumSymbols> pIEnumUps;
	//HRESULT hr = pISymbol->findChildren(SymTagCompilandEnv, NULL, nsNone, &pIEnumUps);
	//if (SUCCEEDED(hr))
	//{
	//	CComPtr<IDiaSymbol> pIEnv;
	//	while (SUCCEEDED(pIEnumUps->Next(1, &pIEnv, &uCelt)) && (uCelt == 1))
	//	{
	//		szEnv.Append(CPdbDoc::ToStr_CompilandEnv(pIEnv));
	//		pIEnv = 0;
	//		break;
	//	}
	//	pIEnumUps = 0;
	//	GetListCtrl().SetItem(nItemIdx, 3, TVIF_TEXT, szEnv, 0, 0, 0, 0);
	//}
}




void CPdbListView::ParsePublics(IDiaSession* pISess, IDiaSymbol* pISim)
{

}

void CPdbListView::ParseSymbols(IDiaSession* pISess, IDiaSymbol* pISim)
{

}

void CPdbListView::ParseGlobals(IDiaSession* pISess, IDiaSymbol* pISim)
{

}

void CPdbListView::ParselLines(IDiaSession* pISess, IDiaSymbol* pISim)
{

}

void CPdbListView::ParseSecContribs(IDiaSession* pISess, IDiaSymbol* pISim)
{

}

void CPdbListView::ParseDebugStreams(IDiaSession* pISess, IDiaSymbol* pISim)
{

}
