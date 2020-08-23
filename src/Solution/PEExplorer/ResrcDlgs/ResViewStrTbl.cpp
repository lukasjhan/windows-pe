#include "..\stdafx.h"
#include "..\PEExplorer.h"
#include "ResViewStrTbl.h"
#include "afxdialogex.h"


// CResViewStrTbl 대화 상자입니다.
IMPLEMENT_DYNAMIC(CResViewStrTbl, CDialogEx)
CResViewStrTbl::CResViewStrTbl(LPBYTE pImgStart, UINT uResID, 
							 DWORD dwItemSize, CWnd* pParent /*=NULL*/)
	: CDialogEx(CResViewStrTbl::IDD, pParent)
{
	m_pImgStart	 = pImgStart;
	m_uResID	 = uResID;
	m_dwItemSize = dwItemSize;
}

CResViewStrTbl::~CResViewStrTbl()
{
}

void CResViewStrTbl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CResViewStrTbl, CDialogEx)
END_MESSAGE_MAP()

// CResViewStrTbl 메시지 처리기입니다.

BOOL CResViewStrTbl::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetDlgItemInt(IDC_EDT_RESID, m_uResID);

	CListCtrl* pLV = (CListCtrl*)GetDlgItem(IDC_LV_STRTBL);
	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	pLV->SetExtendedStyle(pLV->GetExtendedStyle() | dwExStyle);
	pLV->InsertColumn(0, L"ID", LVCFMT_CENTER, 100);
	pLV->InsertColumn(1, L"문자열", LVCFMT_LEFT, 300);

	ParseStringTable(pLV);

	return TRUE;
}

void CResViewStrTbl::ParseStringTable(CListCtrl* pLV)
{
	PWORD pwResStrTbl = (PWORD)m_pImgStart;
	INT i = 0;
	for(; i < 16; i++)
	{
		if(pwResStrTbl[i] != 0)
			break;
	}

	DWORD dwSize = i * 2;
	UINT uBaseID = (m_uResID - 1) * 16 + i;
	UINT uIter = 0;
	PWSTR pwszIter = (PWSTR)&pwResStrTbl[i];

	INT nItem = 0;
	while(dwSize < m_dwItemSize)
	{
		WORD wStrLen = (WORD)pwszIter[0];
		pwszIter++;
		dwSize += sizeof(WORD);
		if(wStrLen == 0)
		{
			uIter++;
			continue;
		}

		CString sz;
		PWSTR pszVal = sz.GetBufferSetLength(wStrLen);
		memcpy(pszVal, pwszIter, wStrLen * sizeof(WCHAR));
		pwszIter += wStrLen;
		dwSize += (wStrLen * sizeof(WCHAR));

		CString szOut;
		szOut.Format(L"%d (0x%X)", uBaseID + uIter, uBaseID + uIter);
		pLV->InsertItem(nItem, szOut);
		pLV->SetItemText(nItem, 1, sz);

		uIter++; nItem++;
	}
}
