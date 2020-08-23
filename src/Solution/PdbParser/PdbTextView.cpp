#include "stdafx.h"
#ifndef SHARED_HANDLERS
#	include "PdbParser.h"
#endif
#include "PdbDoc.h"
#include "PdbTextView.h"

#ifdef _DEBUG
#	define new DEBUG_NEW
#endif

// CPdbTextView
IMPLEMENT_DYNCREATE(CPdbTextView, CEditView)
BEGIN_MESSAGE_MAP(CPdbTextView, CEditView)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CPdbTextView 생성/소멸
CPdbTextView::CPdbTextView()
{
	// TODO: 여기에 생성 코드를 추가합니다.
}

CPdbTextView::~CPdbTextView()
{
}

bool CPdbTextView::UpdateCode(PDIA_NODE pdn)
{
	CComPtr<IDiaSymbol> pISymbol;
	if (pdn->IDiaPtr->QueryInterface(&pISymbol) != S_OK)
		return false;
	DIA_SYMTAGS tag = (DIA_SYMTAGS)pdn->Category;

	CString szVal;
	szVal = PEPdb::ToStr_Symbol(pISymbol, 0);
	szVal.Append(GSZ_CRLF);
	if (tag == SymTagUDT)
	{
		szVal.Append(GetCode_UDT(pISymbol, 0));
	}
	else if (tag == SymTagEnum)
	{
		szVal.Append(GetCode_Enum(pISymbol));
	}
	else if (tag == SymTagFunction)
	{
		szVal.Append(GetCode_Function(pISymbol));
	}
	else if (tag == SymTagData)
	{
		szVal = PEPdb::ToStr_Data(pISymbol);
	}
	else if (tag == SymTagCompiland)
	{
		ULONG uCelt = 0;
		CComPtr<IDiaEnumSymbols> pIEnumUps;
		HRESULT hr = pISymbol->findChildren(SymTagCompilandDetails, NULL, nsNone, &pIEnumUps);
		if (SUCCEEDED(hr))
		{
			CComPtr<IDiaSymbol> pIDetail;
			while (SUCCEEDED(pIEnumUps->Next(1, &pIDetail, &uCelt)) && (uCelt == 1))
			{
				szVal.Append(PEPdb::ToStr_CompilandDetails(pIDetail));
				pIDetail = 0;
			}
		}
	}
	GetEditCtrl().SetWindowText(szVal);
	return true;
}

BOOL CPdbTextView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CEditView::PreCreateWindow(cs))
		return FALSE;
	cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);	// 자동 줄바꿈을 사용합니다.
	cs.style |= ES_READONLY;
	return TRUE;
}

void CPdbTextView::OnInitialUpdate()
{
	CEditView::OnInitialUpdate();

	GetEditCtrl().SetFont(((CPdbApp*)AfxGetApp())->GetFont());
	GetEditCtrl().SetWindowText(L"");
}

// CPdbTextView 진단
#ifdef _DEBUG
void CPdbTextView::AssertValid() const
{
	CEditView::AssertValid();
}

void CPdbTextView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CPdbDoc* CPdbTextView::GetDocument() const // 디버그되지 않은 버전은 인라인으로 지정됩니다.
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CPdbDoc)));
	return (CPdbDoc*)m_pDocument;
}
#endif //_DEBUG

// CPdbTextView 메시지 처리기
void CPdbTextView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CPdbTextView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	//theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}






CString CPdbTextView::GetCode_Enum(IDiaSymbol* pISymbol)
{
	CString szCode = L"enum ";

	CComBSTR bszName;
	pISymbol->get_name(&bszName);
	szCode.Append(bszName);

	CComPtr<IDiaSymbol> pIBase;
	if (pISymbol->get_type(&pIBase) == S_OK)
	{
		szCode.AppendFormat(L" : %s", PEPdb::ToStr_BaseType(pIBase));
		pIBase = 0;
	}
	szCode.Append(GSZ_CRLF L"{" GSZ_CRLF);

	CComPtr<IDiaEnumSymbols> pIEnumMbr;
	if (SUCCEEDED(pISymbol->findChildren(SymTagData, NULL, nsNone, &pIEnumMbr)))
	{
		CComPtr<IDiaSymbol> pIMbr;
		ULONG uCelt = 0;
		while (SUCCEEDED(pIEnumMbr->Next(1, &pIMbr, &uCelt)) && (uCelt == 1))
		{
			pIMbr->get_name(&bszName);
			CComVariant vt;
			pIMbr->get_value(&vt);
			szCode.AppendFormat(L"\t%s =%s," GSZ_CRLF, bszName, PEPdb::ToStr_Variant(&vt));
			pIMbr = 0;
		}
		pIEnumMbr = 0;
	}
	szCode.Append(L"};");
	return szCode;
}

CString CPdbTextView::GetCode_UDT(IDiaSymbol* pISymbol, int nDepth)
{
	CString szCode;
	for (int i = 0; i < nDepth; i++)
		szCode.Append(L"    ");

	ULONG	uCelt = 0;
	UdtKind eKind;
	pISymbol->get_udtKind((PDWORD)&eKind);
	CComBSTR bszName;
	pISymbol->get_name(&bszName);

	bool bNoname = (wcsstr(bszName, L"unnamed-") != 0);
	if (bNoname)
		szCode = PEPdb::GSZ_UDT_KINDS[eKind];
	else
	{
		szCode.AppendFormat(L"%s %s", PEPdb::GSZ_UDT_KINDS[eKind], bszName);
		CString szBase;
		CComPtr<IDiaEnumSymbols> pIEnumUps;
		HRESULT hr = pISymbol->findChildren(SymTagBaseClass, NULL, nsNone, &pIEnumUps);
		if (SUCCEEDED(hr))
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
			szCode.AppendFormat(L" : public %s", szBase);
	}
	szCode.Append(GSZ_CRLF);
	for (int i = 0; i < nDepth; i++)
		szCode.Append(L"    ");
	szCode.Append(L"{" GSZ_CRLF);

	CString szMbr;
	{
		CComPtr<IDiaEnumSymbols> pIEnumMbr;
		if (SUCCEEDED(pISymbol->findChildren(SymTagData, NULL, nsNone, &pIEnumMbr)))
		{
			CComPtr<IDiaSymbol> pIMbr;
			ULONG uCelt = 0;
			while (SUCCEEDED(pIEnumMbr->Next(1, &pIMbr, &uCelt)) && (uCelt == 1))
			{
				for (int i = 0; i < nDepth + 1; i++)
					szMbr.Append(L"    ");
				if (eKind == UdtClass)
				{
					DWORD dwAccess = 0;
					pIMbr->get_access(&dwAccess);
					if (dwAccess > 0)
						szMbr.AppendFormat(L"%s: ", PEPdb::GSZ_ACCESS_SPECS[dwAccess]);
				}

				CComPtr<IDiaSymbol> pIType;
				if (pIMbr->get_type(&pIType) == S_OK)
				{
					CComBSTR bszTypeName;
					bool bNoname2 = false;
					if (pIType->get_name(&bszTypeName) == S_OK)
						bNoname2 = (wcsstr(bszTypeName, L"unnamed-") != 0);

					BOOL bIsSet;
					if (pIType->get_nested(&bIsSet) == S_OK && bIsSet || bNoname2)
						szMbr.Append(GetCode_UDT(pIType, nDepth + 1));
					else
						szMbr.Append(PEPdb::ToStr_Type(pIType));
					pIType = 0;
				}
				pIMbr->get_name(&bszName);
				DWORD dwLoc;
				if (pIMbr->get_locationType(&dwLoc) == S_OK && dwLoc == LocIsBitField)
				{
					ULONGLONG length = 0;
					pIMbr->get_length(&length);
					szMbr.AppendFormat(L" %s : %d;\xd\xa", bszName, length);
				}
				else
					szMbr.AppendFormat(L" %s;\xd\xa", bszName);
				pIMbr = 0;
			}
			pIEnumMbr = 0;
		}
	}
	if (!szMbr.IsEmpty())
	{
		szCode.Append(szMbr);
		for (int i = 0; i < nDepth; i++)
			szCode.Append(L"    ");
		szCode.Append(GSZ_CRLF);
	}
	CString szFnc;
	{
		CComPtr<IDiaEnumSymbols> pIEnumMbr;
		if (SUCCEEDED(pISymbol->findChildren(SymTagFunction, NULL, nsNone, &pIEnumMbr)))
		{
			CComPtr<IDiaSymbol> pIMbr;
			ULONG uCelt = 0;
			while (SUCCEEDED(pIEnumMbr->Next(1, &pIMbr, &uCelt)) && (uCelt == 1))
			{
				for (int i = 0; i < nDepth + 1; i++)
					szFnc.Append(L"    ");
				if (eKind == UdtClass)
				{
					DWORD dwAccess = 0;
					pIMbr->get_access(&dwAccess);
					if (dwAccess > 0)
						szFnc.AppendFormat(L"%s: ", PEPdb::GSZ_ACCESS_SPECS[dwAccess]);
				}

				pIMbr->get_name(&bszName);
				CComPtr<IDiaSymbol> pIType;
				if (pIMbr->get_type(&pIType) == S_OK)
				{
					CString szFunc = PEPdb::ToStr_Type(pIType);
					szFunc.Replace(L"(FUNC)", bszName);
					szFnc.AppendFormat(L" %s;\xd\xa", szFunc);
					pIType = 0;
				}
				pIMbr = 0;
			}
			pIEnumMbr = 0;
		}
	}
	if (!szFnc.IsEmpty())
		szCode.Append(szFnc);
	for (int i = 0; i < nDepth; i++)
		szCode.Append(L"    ");
	if (bNoname)
		szCode.Append(L"}");
	else
		szCode.Append(L"};");
	return szCode;
}

CString CPdbTextView::GetCode_Function(IDiaSymbol* pISymbol)
{
	CString szCode;
	CComBSTR bszName;
	//if (pISymbol->get_name(&bszName) != S_OK)
	if (pISymbol->get_undecoratedName(&bszName) == S_OK)
		szCode = bszName;
	else
		szCode = PEPdb::ToStr_Function(pISymbol);

	ULONGLONG ulSize = 0;
	pISymbol->get_length(&ulSize);
	szCode.AppendFormat(L"\xd\xaLength : %d\xd\xa", ulSize);

	DWORD dwRva, dwSect, dwOffset;
	//ULONGLONG dwVa;
	//pISymbol->get_virtualAddress(&dwVa);
	pISymbol->get_relativeVirtualAddress(&dwRva);
	//pISymbol->get_virtualBaseOffset(&dwAdOffset);
	pISymbol->get_addressOffset(&dwOffset);
	pISymbol->get_addressSection(&dwSect);
	szCode.AppendFormat(L"Location : [%08X][%04X:%08X]\xd\xa", dwRva, dwSect, dwOffset);

	DWORD dwAccess = 0;
	if (pISymbol->get_access(&dwAccess) == S_OK)
	{
		CV_access_e cve = (CV_access_e)dwAccess;
		//szCode.AppendFormat(L"Location : [%08X][%04X:%08X] ", dwRva, dwSect, dwOffset);
	}

	CString szFlags = 0;
	BOOL bIsSet = FALSE;
	if (pISymbol->get_constType(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|const");
	if (pISymbol->get_customCallingConvention(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|customCallingConvention");
	if (pISymbol->get_farReturn(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|farReturn");
	if (pISymbol->get_hasAlloca(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|hasAlloca");
	if (pISymbol->get_hasEH(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|hasEH");
	if (pISymbol->get_hasEHa(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|hasEHa");
	if (pISymbol->get_hasInlAsm(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|hasInlAsm");
	if (pISymbol->get_hasLongJump(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|hasLongJump");
	if (pISymbol->get_hasSecurityChecks(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|hasSecurityChecks");
	if (pISymbol->get_hasSEH(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|hasSEH");
	if (pISymbol->get_hasSetJump(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|hasSetJump");
	if (pISymbol->get_interruptReturn(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|interruptReturn");
	if (pISymbol->get_intro(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|intro");
	if (pISymbol->get_inlSpec(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|inline");
	if (pISymbol->get_isNaked(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|naked");
	if (pISymbol->get_isStatic(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|static");
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
	if (pISymbol->get_pure(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|pure");
	if (pISymbol->get_unalignedType(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|unaligned");
	if (pISymbol->get_virtual(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|virtual");
	if (pISymbol->get_volatileType(&bIsSet) == S_OK && bIsSet)
		szFlags.Append(L"|volatile");
	if (!szFlags.IsEmpty())
		szCode.AppendFormat(L"Attribute : %s\xd\xa", szFlags);

	return szCode;
}
