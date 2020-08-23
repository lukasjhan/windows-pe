#include "..\stdafx.h"
#include "..\PEExplorer.h"
#include "ResViewAccelTbl.h"
#include "afxdialogex.h"


// CResViewAccelTbl 대화 상자입니다.
IMPLEMENT_DYNAMIC(CResViewAccelTbl, CDialogEx)
CResViewAccelTbl::CResViewAccelTbl(WORD wResType, PBYTE pImgStart, CWnd* pParent /*=NULL*/)
	: CDialogEx(CResViewAccelTbl::IDD, pParent)
{
	m_wResType	= wResType;
	m_pImgStart	= pImgStart;
}

CResViewAccelTbl::~CResViewAccelTbl()
{
}

void CResViewAccelTbl::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CResViewAccelTbl, CDialogEx)
END_MESSAGE_MAP()

// CResViewAccelTbl 메시지 처리기입니다.
BOOL CResViewAccelTbl::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CListCtrl* pLv = (CListCtrl*)GetDlgItem(IDC_LV_ACCELTBL);
	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	pLv->SetExtendedStyle(pLv->GetExtendedStyle() | dwExStyle);

	if (m_wResType == (WORD)RT_ACCELERATOR)
	{
		pLv->InsertColumn(0, L"ID", LVCFMT_LEFT, 120);
		pLv->InsertColumn(1, L"키코드", LVCFMT_CENTER, 50);
		pLv->InsertColumn(2, L"보조키", LVCFMT_LEFT, 60);
		pLv->InsertColumn(3, L"가상키", LVCFMT_CENTER, 50);
		pLv->InsertColumn(4, L"기타", LVCFMT_LEFT, 150);
		ParseAccelatorTable(pLv);
	}
	else if (m_wResType == (WORD)RT_GROUP_CURSOR ||
			 m_wResType == (WORD)RT_GROUP_ICON)
	{
		CString szTitle;
		if (m_wResType == (WORD)RT_GROUP_ICON)
			szTitle = L"그룹 아이콘 보기";
		else
			szTitle = L"그룹 커서 보기";
		SetWindowText(szTitle);

		pLv->InsertColumn(0, L"ID", LVCFMT_LEFT, 50);
		pLv->InsertColumn(1, L"Width", LVCFMT_LEFT, 70);
		pLv->InsertColumn(2, L"Height", LVCFMT_LEFT, 70);
		pLv->InsertColumn(3, L"BitCount", LVCFMT_LEFT, 70);
		pLv->InsertColumn(4, L"Size", LVCFMT_LEFT, 100);
		ParseGroupResInfo(pLv);
	}
	else
		return FALSE;
	return TRUE;
}


#define FEND	0x80

struct ACCELTABLEENTRY
{
	WORD fFlags;
	WORD wAnsi;
	WORD wId;
	WORD padding;
};
typedef ACCELTABLEENTRY* PACCELTABLEENTRY;

void CResViewAccelTbl::ParseAccelatorTable(CListCtrl* pLv)
{
	PCWSTR pszKey[] = 
	{
		L"BS",	  L"Tab",  L"",		 L"Clr",	L"Enter",	L"",	
		L"Shift", L"Ctrl", L"Alt",	 L"Pause",	L"CAPS",	L"Kana",
		L"Han",	  L"",	   L"Junja", L"Final",	L"Hanja",	L"Kanji",
		L"",	  L"Esc",  L"Ime",	 L"NoIme",	L"ImeChg",	L"Space",
		L"PgUp",  L"PgDn", L"End",	 L"Home",	L"Left",	L"Up",
		L"Right", L"Down", L"Sel",	 L"Prn",	L"Exec",	L"PrnScrn", 
		L"Ins",	  L"Del",  L"Help"
	};
	bool	bIsNext = true;
	INT		nItem = 0;
	LPBYTE	pIter = m_pImgStart;

	do
	{
		PACCELTABLEENTRY pATE = (PACCELTABLEENTRY)pIter;
		pIter += sizeof(ACCELTABLEENTRY);

		CString szNum;
		szNum.Format(L"%d (0x%X)", pATE->wId, pATE->wId);
		pLv->InsertItem(nItem, szNum);

		if(pATE->wAnsi >= VK_BACK && pATE->wAnsi <= VK_HELP)
			szNum = pszKey[pATE->wAnsi - VK_BACK];
		else
		if(pATE->wAnsi >= VK_F1 && pATE->wAnsi <= VK_F24)
			szNum.Format(L"F%c", pATE->wAnsi - VK_F1 + 0x31);
		else
		if(pATE->wAnsi >= 0x30 && pATE->wAnsi <= 0x5A)
			szNum.Format(L"%c", pATE->wAnsi);
		else
			szNum = L"미지정";
		pLv->SetItemText(nItem, 1, szNum);

		szNum.Empty();
		if(pATE->fFlags & FSHIFT)
			szNum += L"Shift";
		if(pATE->fFlags & FCONTROL)
		{
			if(szNum.IsEmpty())
				szNum += L"Ctrl";
			else
				szNum += L" + Ctrl";
		}
		if(pATE->fFlags & FALT)
		{
			if(szNum.IsEmpty())
				szNum += L"Alt";
			else
				szNum += L" + Alt";
		}
		pLv->SetItemText(nItem, 2, szNum);

		if(pATE->fFlags & FVIRTKEY)
			szNum = L"v";
		else
			szNum = L"-";
		pLv->SetItemText(nItem, 3, szNum);

		if(pATE->fFlags & FNOINVERT)
			szNum = L"No Invert";
		else
			szNum = L"";
		if(pATE->fFlags & FEND)
		{
			if(szNum.IsEmpty())
				szNum += L"Last Entry";
			else
				szNum += L", Last Entry";

			bIsNext = false;
		}
		pLv->SetItemText(nItem, 4, szNum);
		nItem++;
	}
	while(bIsNext);
}



#pragma pack(1)
typedef struct
{
	WORD	wReserved;		// Reserved
	WORD	wType;			// resource type (1 for icons)
	WORD	wCount;			// how many images?
} ICONDIR, *LPICONDIR;

typedef struct
{
	BYTE	bWidth;				// Width of the image
	BYTE	bHeight;			// Height of the image
	BYTE	bColorCount;		// Number of colors in image (0 if >=8bpp)
	BYTE	bReserved;			// Reserved
	WORD	wPlanes;			// Color Planes
	WORD	wBitCount;			// Bits per pixel
	DWORD	dwBytesInRes;		// how many bytes in this resource?
	WORD	wID;				// the ID
} RES_ICONDIRENTRY, *LPRES_ICONDIRENTRY;

typedef struct
{
	WORD	wWidth;			// Width of the image
	WORD	wHeight;		// Height of the image(times 2)
	WORD	wPlanes;		// Color Planes
	WORD	wBitCount;		// Bits per pixel
	DWORD	dwBytesInRes;	// how many bytes in this resource?
	WORD	wID;			// the ID
} RES_CURSORDIRENTRY, *LPRES_CURSORDIRENTRY;
#pragma pack()

void CResViewAccelTbl::ParseGroupResInfo(CListCtrl* pLv)
{
	PBYTE pIter = m_pImgStart;
	LPICONDIR pid = (LPICONDIR)pIter;
	pIter += sizeof(ICONDIR);

	if (pid->wType != 1 && pid->wType != 2)
		return;

	for (WORD i = 0; i < pid->wCount; i++)
	{
		WORD wID, wW, wH, wB;
		DWORD dwSize;
		if (pid->wType == 1)
		{
			LPRES_ICONDIRENTRY pride = (LPRES_ICONDIRENTRY)pIter;
			pIter += sizeof(RES_ICONDIRENTRY);
			wW = pride->bWidth, wH = pride->bHeight, wB = pride->wBitCount;
			wID = pride->wID, dwSize = pride->dwBytesInRes;
		}
		else
		{
			LPRES_CURSORDIRENTRY pride = (LPRES_CURSORDIRENTRY)pIter;
			pIter += sizeof(RES_CURSORDIRENTRY);
			wW = pride->wWidth, wH = pride->wHeight, wB = pride->wBitCount;
			wID = pride->wID, dwSize = pride->dwBytesInRes;
		}

		CString sz; sz.Format(L"%d", wID);
		int nItem = pLv->InsertItem(i, sz);
		sz.Format(L"%d", wW);
		pLv->SetItemText(nItem, 1, sz);
		sz.Format(L"%d", wH);
		pLv->SetItemText(nItem, 2, sz);
		sz.Format(L"%d", wB);
		pLv->SetItemText(nItem, 3, sz);
		sz.Format(L"%d", dwSize);
		pLv->SetItemText(nItem, 4, sz);
	}
}
