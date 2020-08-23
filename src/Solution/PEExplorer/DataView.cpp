#include "stdafx.h"
#include "PEExplorer.h"
#include "PEAnals.h"
#include "DataView.h"
#include "afxdialogex.h"

#define BSZ_STR			L"`string'"
#define BSZ_PAT			L"??_C@_"
#define BSZ_PAT_LEN		6
#define BSZ_IMP			L"__imp_"
#define BSZ_IMP_LEN		6


// CDataView 대화 상자입니다.
IMPLEMENT_DYNAMIC(CDataView, CDialogEx)

CDataView::CDataView(PPE_NODE pNode, PEAnals* pPE, CWnd* pParent /*=NULL*/)
	: CDialogEx(CDataView::IDD, pParent)
{
	m_pPN = pNode;
	m_pPE = pPE;
}

CDataView::~CDataView()
{
}

void CDataView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LV_DATA, m_lv);
}

CString CDataView::GetPrimitiveVal(IDiaSymbol* pIPrimtv, int nSize, DWORD dwOffset)
{
	BasicType dbt = BasicType::btNoType;
	if (pIPrimtv->get_baseType((PDWORD)&dbt) != S_OK)
		return L"??";

	PE_TYPE dt = PE_TYPE::PEVar;
	switch (dbt)
	{
		case btUInt: case btInt:
			switch (nSize)
			{
				case 1:	dt = PE_TYPE::UInt8; break;
				case 2: dt = PE_TYPE::UInt16; break;
				case 4: dt = PE_TYPE::UInt32; break;
				case 8: dt = PE_TYPE::UInt64; break;
			}
			if (dt == PE_TYPE::PEVar)
				break;
			if(dbt == btInt)
				dt = (PE_TYPE)(dt + PE_TYPE::Int8);
		break;
		case btFloat:
			switch (nSize)
			{
				case 4: dt = PE_TYPE::UInt32; break;
				case 8: dt = PE_TYPE::UInt64; break;
			}
		break;
		case btBool:	dt = PE_TYPE::UInt8; break;
		case btHresult: dt = PE_TYPE::Int32; break;
	}
	if (dt == PE_TYPE::PEVar)
		return L"??";

	UINT64 uv = PE_SCHEMA::GetValue(m_pPE->GetImgView() + dwOffset, dt);
	return PE_SCHEMA::Val2Str(uv, dt);
}

int CDataView::BuildPublicSymbol(IDiaSymbol* pISymbol, PCWSTR pszName, 
								 DWORD dwOffset, CString& szType, CString& szVal)
{
	ULONGLONG ulSize = 0;
	if (pISymbol->get_length(&ulSize) != S_OK)
		throw E_INVALIDARG;

	CComBSTR bszUnname;
	if (pISymbol->get_undecoratedName(&bszUnname) != S_OK)
		throw E_INVALIDARG;

	PE_TYPE pt = PE_TYPE::PEVar;
	if (_wcsicmp(bszUnname, BSZ_STR) == 0 && wcsncmp(pszName, BSZ_PAT, BSZ_PAT_LEN) == 0)
	{
		int nLen = 0;
		if (pszName[BSZ_PAT_LEN] == L'0')
		{
			pt = PE_TYPE::UInt8;
			nLen = (int)ulSize;
			szType = L"const char*";
		}
		else
		{
			pt = PE_TYPE::UInt16;
			nLen = (int)(ulSize >> 1);
			szType = L"const wchar_t*";
		}
		szVal = PE_SCHEMA::GetString(m_pPE->GetImgView() + dwOffset, pt, nLen);
	}
	else if (wcsncmp(bszUnname, BSZ_IMP, BSZ_IMP_LEN) == 0)
	{
		szType = L"Import Thunk";
		pt = (m_pPE->Is32Bit()) ? PE_TYPE::UInt32 : PE_TYPE::UInt64;
		UINT64 uv = PE_SCHEMA::GetValue(m_pPE->GetImgView() + dwOffset, pt);
		szVal = PE_SCHEMA::Val2Str(uv, pt);
	}
	return (int)ulSize;
}

int CDataView::BuildDataSymbol(IDiaSymbol* pISymbol, DWORD dwOffset, 
							   CString& szType, CString& szVal)
{
	CComPtr<IDiaSymbol> pIType;
	if (pISymbol->get_type(&pIType) != S_OK)
		throw E_INVALIDARG;
	szType = PEPdb::ToStr_Type(pIType);

	ULONGLONG ulSize = 0;
	if (pIType->get_length(&ulSize) != S_OK)
		throw E_INVALIDARG;

	DIA_SYMTAG eTag = DIA_SYMTAG::SymTagNull;
	if (pIType->get_symTag((PDWORD)&eTag) != S_OK)
		throw E_INVALIDARG;

	bool bIsStr = false;
	if (eTag == SymTagBaseType)
		szVal = GetPrimitiveVal(pIType, (int)ulSize, dwOffset);
	else if (eTag == SymTagPointerType)
	{
		PE_TYPE dt = (m_pPE->Is32Bit()) ? PE_TYPE::UInt32 : PE_TYPE::UInt64;
		UINT64 uv = PE_SCHEMA::GetValue(m_pPE->GetImgView() + dwOffset, dt);
		szVal = PE_SCHEMA::Val2Str(uv, dt);
	}
	else if (eTag == SymTagArrayType)
	{
		DWORD dwElmCnt = 0;
		if (pIType->get_count(&dwElmCnt) == S_OK && dwElmCnt > 0)
		{
			CComPtr<IDiaSymbol> pIBase;
			if (pIType->get_type(&pIBase) == S_OK &&
				pIBase->get_symTag((PDWORD)&eTag) == S_OK)
			{
				int nLen = (int)dwElmCnt;
				if (nLen > 5) nLen = 5;
				if (eTag == DIA_SYMTAG::SymTagBaseType)
				{
					int nElemSize = (int)(ulSize / dwElmCnt);
					if (nElemSize == 1 || nElemSize == 2)
					{
						BasicType dbt = BasicType::btNoType;
						pIBase->get_baseType((PDWORD)&dbt);
						if (dbt == btChar || dbt == btWChar)
						{
							PE_TYPE dt = (dbt == btChar) ? PE_TYPE::UInt8 : PE_TYPE::UInt16;
							szVal = PE_SCHEMA::GetString(m_pPE->GetImgView() + dwOffset, dt, (int)dwElmCnt);
							bIsStr = true;
						}
					}
					if (!bIsStr)
					{
						for (int i = 0; i < nLen; i++)
						{
							if (i > 0) szVal.Append(L" ");
							szVal.Append(GetPrimitiveVal(pIBase, nElemSize, dwOffset));
							dwOffset += nElemSize;
						}
						if (nLen != (int)dwElmCnt)
							szVal.Append(L"...");
					}
				}
				else if (eTag == DIA_SYMTAG::SymTagPointerType)
				{
					PE_TYPE dt = (m_pPE->Is32Bit()) ? PE_TYPE::UInt32 : PE_TYPE::UInt64;
					for (int i = 0; i < nLen; i++)
					{
						UINT64 uv = PE_SCHEMA::GetValue(m_pPE->GetImgView() + dwOffset, dt);
						szVal = PE_SCHEMA::Val2Str(uv, dt);
						dwOffset += PE_MEMBER::C_Sizes[dt];
					}
					if (nLen != (int)dwElmCnt)
						szVal.Append(L"...");
				}
				pIBase = 0;
			}
		}
	}
	if (szVal.IsEmpty() && !bIsStr)
	{
		int nLen = (int)ulSize;
		if (nLen > 5)
			nLen = 5;
		szVal = PE_SCHEMA::Bin2Str(m_pPE->GetImgView() + dwOffset, PE_TYPE::UInt8, nLen);
		if (nLen != (int)ulSize)
			szVal.Append(L"...");
	}

	return (int)ulSize;
}

BOOL CDataView::BuildDataList()
{
	CComPtr<IDiaEnumSymbolsByAddr> pIEnumByAddr;
	if (FAILED(m_pPE->DIA_PDB.SESSION->getSymbolsByAddr(&pIEnumByAddr)))
		return FALSE;

	CComPtr<IDiaSymbol> pISymbol;
	if (FAILED(pIEnumByAddr->symbolByAddr(m_pPN->Index + 1, 0, &pISymbol)))
	{
		pIEnumByAddr = 0;
		return FALSE;
	}

	DWORD dwRvaLast = 0;
	HRESULT hr = pISymbol->get_relativeVirtualAddress(&dwRvaLast);
	pISymbol = 0;
	if (hr != S_OK)
	{
		pIEnumByAddr = 0;
		return FALSE;
	}

	if (pIEnumByAddr->symbolByRVA(dwRvaLast, &pISymbol) != S_OK)
	{
		pIEnumByAddr = 0;
		return FALSE;
	}

	PIMAGE_SECTION_HEADER psh = &(m_pPE->GetSectHdrs()[m_pPN->Index]);
	CComBSTR bszName;
	ULONG uCelt = 0;
	int nItemIdx = 0;
	do
	{
		DWORD dwSectId = 0;
		if ((pISymbol->get_addressSection(&dwSectId) != S_OK) ||
			(dwSectId != m_pPN->Index + 1))
			break;
		if (pISymbol->get_relativeVirtualAddress(&dwRvaLast) != S_OK)
			break;

		DIA_SYMTAG eTag = DIA_SYMTAG::SymTagNull;
		if (pISymbol->get_symTag((PDWORD)&eTag) != S_OK)
			break;
		if (eTag != SymTagData && eTag != SymTagPublicSymbol)
		{
			pISymbol = 0;
			continue;
		}

		CComBSTR bszName;
		if (pISymbol->get_name(&bszName) != S_OK)
			bszName = L"<NO_NAME>";
		DWORD dwOffset = RVA_TO_OFFSET(psh, dwRvaLast);
		int nSize = 0;
		CString szType, szVal;
		try
		{
			if (eTag == SymTagPublicSymbol)
				nSize = BuildPublicSymbol(pISymbol, bszName, dwOffset, szType, szVal);
			else
				nSize = BuildDataSymbol(pISymbol, dwOffset, szType, szVal);
			pISymbol = 0;
		}
		catch (HRESULT)
		{
			pISymbol = 0;
			continue;
		}

		CString sz;
		sz.Format(L"0x%08X:0x%08X", dwOffset, dwRvaLast);
		int nCurIdx = m_lv.InsertItem(nItemIdx, sz);
		m_lv.SetItemText(nCurIdx, 1, bszName);
		m_lv.SetItemText(nCurIdx, 2, szType);
		sz.Format(L"0x%X(%d)", nSize, nSize);
		m_lv.SetItemText(nCurIdx, 3, sz);
		m_lv.SetItemText(nCurIdx, 4, szVal);
		nItemIdx++;
		pISymbol = 0;
	}
	while (SUCCEEDED(pIEnumByAddr->Next(1, &pISymbol, &uCelt)) && uCelt == 1);
	if (pISymbol)
		pISymbol = 0;
	pIEnumByAddr = 0;

	return TRUE;
}

BEGIN_MESSAGE_MAP(CDataView, CDialogEx)
	ON_WM_SIZE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// CDataView 메시지 처리기입니다.
BOOL CDataView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CRect rc;
	GetClientRect(rc);
	CRect rcView;
	m_lv.GetWindowRect(rcView);
	ScreenToClient(rcView);
	m_siMargin.cx = rc.Width() - rcView.Width();
	m_siMargin.cy = rc.Height() - rcView.Height();

	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	m_lv.SetExtendedStyle(m_lv.GetExtendedStyle() | dwExStyle);
	m_lv.InsertColumn(0, _T("오프셋:RVA"), LVCFMT_LEFT, 180);
	m_lv.InsertColumn(1, _T("이름"), LVCFMT_LEFT, 120);
	m_lv.InsertColumn(2, _T("타입"), LVCFMT_LEFT, 100);
	m_lv.InsertColumn(3, _T("크기"), LVCFMT_RIGHT, 100);
	m_lv.InsertColumn(4, _T("값"), LVCFMT_LEFT, 150);

	return BuildDataList();
}

void CDataView::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_lv.GetSafeHwnd())
	{
		m_lv.SetWindowPos
			(NULL, 0, 0, cx - m_siMargin.cx, cy - m_siMargin.cy, SWP_NOMOVE | SWP_NOZORDER);
	}
}

void CDataView::OnDestroy()
{
	CDialogEx::OnDestroy();
}
