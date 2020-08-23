#include "..\stdafx.h"
#include "..\PEExplorer.h"
#include "ResViewVerInfo.h"
#include "afxdialogex.h"


PCWSTR SZ_FileFlags[] =
{
	L"DEBUG",			//0x00000001L
	L"PRERELEASE",		//0x00000002L
	L"PATCHED",			//0x00000004L
	L"PRIVATEBUILD",	//0x00000008L
	L"INFOINFERRED",	//0x00000010L
	L"SPECIALBUILD",	//0x00000020L
};

/* ----- VS_VERSION.dwFileOS ----- */
PCWSTR SZ_FileMjrOS[] =
{
	L"UNKNOWN",	//0x00000000L
	L"DOS",		//0x00010000L
	L"OS216",	//0x00020000L
	L"OS232",	//0x00030000L
	L"NT",		//0x00040000L
	L"WINCE",	//0x00050000L
};
PCWSTR SZ_FileMinOS[] =
{
	L"BASE",		//0x00000000L
	L"WINDOWS16",	//0x00000001L
	L"PM16",		//0x00000002L
	L"PM32",		//0x00000003L
	L"WINDOWS32",	//0x00000004L
};
/* ------------------------------ */

PCWSTR SZ_FileTypes[] =
{
	L"UNKNOWN",		//0x00000000L
	L"APP",			//0x00000001L
	L"DLL",			//0x00000002L
	L"DRV",			//0x00000003L
	L"FONT",		//0x00000004L
	L"VXD",			//0x00000005L
	L"STATIC_LIB",	//0x00000007L
};

/* ----- VS_VERSION.dwFileSubtype for VFT_WINDOWS_DRV ----- */
PCWSTR SZ_FileTypeDRVs[] =
{
	L"UNKNOWN",		//0x00000000L
	L"PRINTER",		//0x00000001L
	L"KEYBOARD",	//0x00000002L
	L"LANGUAGE",	//0x00000003L
	L"DISPLAY",		//0x00000004L
	L"MOUSE",		//0x00000005L
	L"NETWORK",		//0x00000006L
	L"SYSTEM",		//0x00000007L
	L"INSTALLABLE",	//0x00000008L
	L"SOUND",		//0x00000009L
	L"COMM",		//0x0000000AL
	L"INPUTMETHOD",	//0x0000000BL
	L"PRINTER",		//0x0000000CL
};

/* ----- VS_VERSION.dwFileSubtype for VFT_WINDOWS_FONT ----- */
PCWSTR SZ_FileTypeFNTs[] =
{
	L"UNKNOWN",		//0x00000000L
	L"RASTER",		//0x00000001L
	L"VECTOR",		//0x00000002L
	L"TRUETYPE",	//0x00000003L
};


// CResViewVerInfo 대화 상자입니다.
IMPLEMENT_DYNAMIC(CResViewVerInfo, CDialogEx)
CResViewVerInfo::CResViewVerInfo(LPBYTE pImgStart, CWnd* pParent /*=NULL*/)
	: CDialogEx(CResViewVerInfo::IDD, pParent)
{
	m_pImgStart	 = pImgStart;
}

CResViewVerInfo::~CResViewVerInfo()
{
}

void CResViewVerInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CResViewVerInfo, CDialogEx)
END_MESSAGE_MAP()

// CResViewVerInfo 메시지 처리기입니다.
BOOL CResViewVerInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CListCtrl* pLv = (CListCtrl*)GetDlgItem(IDC_LV_ACCELTBL);
	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	pLv->SetExtendedStyle(pLv->GetExtendedStyle() | dwExStyle);

	pLv->InsertColumn(0, L"속성", LVCFMT_LEFT, 150);
	pLv->InsertColumn(1, L"값", LVCFMT_LEFT, 300);

	int nItem = 0;
	PVAR_VER_TYPE prvi = (PVAR_VER_TYPE)m_pImgStart;
	BuildVersionInfo(m_pImgStart, prvi->wLength, pLv, 0, nItem);

	return TRUE;
}

void CResViewVerInfo::BuildFixedVerInfo(PVS_FIXEDFILEINFO pvfi, CListCtrl* pLv, int nDepth, int& nItem)
{
	PCWSTR pszFVFlds[] =
	{
		L"Signature",
		L"StrucVersion",
		L"FileVersion",
		L"ProductVersion",
		L"FileFlagsMask",
		L"FileFlags",
		L"FileOS",
		L"FileType",
		L"FileSubtype",
		L"FileDate"
	};

	CString szTab;
	for (int i = 0; i < nDepth; i++)
		szTab.Append(L"  ");
	pLv->InsertItem(nItem, szTab + L"VS_FIXEDFILEINFO");
	CString szVal; szVal.Format(L"0x%08X", sizeof(VS_FIXEDFILEINFO));
	pLv->SetItemText(nItem, 1, szVal);
	nItem++;
	szTab.Append(L"  ");

	PBYTE pIter = (PBYTE)pvfi;
	for (int i = 0; i < sizeof(pszFVFlds) / sizeof(PCWSTR); i++)
	{
		int nIdx = pLv->InsertItem(i + nItem, szTab + pszFVFlds[i]);
		nItem++;

		CString sz;
		DWORD dwVal = *((PDWORD)pIter); pIter += sizeof(DWORD);
		if (i == 3 || i == 4 || i == 9)
		{
			DWORD dwLS = *((PDWORD)pIter); pIter += sizeof(DWORD);
			sz.Format(L"%d.%d.%d.%d(MS:0x%08X, LS:0x%08X)", HIWORD(dwVal),
				LOWORD(dwVal), HIWORD(dwLS), LOWORD(dwLS), dwVal, dwLS);
		}
		else
		{
			if (i == 5)
			{
				DWORD dwFlags = pvfi->dwFileFlags & pvfi->dwFileFlagsMask;
				for (int i = 0; i < 6; i++)
				{
					if ((dwFlags & (1 << 1)) != 0)
					{
						if (sz.IsEmpty())
							sz.Append(L"|");
						sz.Append(SZ_FileFlags[i]);
					}
				}
			}
			else if (i == 6)
				sz.Format(L"%s_%s", SZ_FileMjrOS[HIWORD(pvfi->dwFileOS)],
				SZ_FileMinOS[LOWORD(pvfi->dwFileOS)]);
			else if (i == 7)
				sz = SZ_FileTypes[pvfi->dwFileType];
			else if (i == 8)
			{
				if (pvfi->dwFileOS == VFT_DRV)
					sz = SZ_FileTypeDRVs[pvfi->dwFileSubtype];
				else if (pvfi->dwFileOS == VFT_FONT)
					sz = SZ_FileTypeFNTs[pvfi->dwFileSubtype];
			}
			if (sz.IsEmpty())
				sz.Format(L"0x%08X", dwVal);
			else
				sz.AppendFormat(L" (0x%08X)", dwVal);
		}
		pLv->SetItemText(nIdx, 1, sz);
	}
}

void CResViewVerInfo::BuildVersionInfo(PBYTE pIter, int nBlockSize, CListCtrl* pLv, int nDepth, int& nItem)
{
	CString szTab;
	for (int i = 0; i < nDepth; i++)
		szTab.Append(L"  ");

	while (nBlockSize > 0)
	{
		PVAR_VER_TYPE pvi = (PVAR_VER_TYPE)pIter;
		pIter += sizeof(VAR_VER_TYPE);
		int nSize = sizeof(VAR_VER_TYPE);

		CString szKey = (PCWSTR)pIter;
		int nLen = (szKey.GetLength() + 1) * sizeof(WCHAR);
		pIter += nLen;
		if (((DWORD_PTR)pIter) & 3)
		{
			pIter += sizeof(WORD);
			nLen += sizeof(WORD);
		}
		nSize += nLen;

		int nIdx = pLv->InsertItem(nItem, szTab + szKey); nItem++;
		if (pvi->wValLen == 0)
		{
			CString szVal; szVal.Format(L"0x%X", pvi->wLength);
			pLv->SetItemText(nIdx, 1, szVal);

			BuildVersionInfo(pIter, pvi->wLength - nSize, pLv, nDepth + 1, nItem);
			nLen = pvi->wLength - nSize;
		}
		else
		{
			if (pvi->wType == 0)
			{
				if (*(PDWORD)pIter == VSFI_SIGNATURE)
				{
					BuildFixedVerInfo((PVS_FIXEDFILEINFO)pIter, pLv, nDepth + 1, nItem);
				}
				else
				{
					DWORD dwVal = *((PDWORD)pIter);
					CString szVal; szVal.Format(L"0x%08X", dwVal);
					pLv->SetItemText(nIdx, 1, szVal);
				}
				nLen = pvi->wValLen;
			}
			else
			{
				pLv->SetItemText(nIdx, 1, (PCWSTR)pIter);
				nLen = pvi->wValLen * sizeof(WCHAR);
			}
		}
		pIter += nLen;
		if (((DWORD_PTR)pIter) & 3)
		{
			pIter += sizeof(WORD);
			nLen += sizeof(WORD);
		}
		nSize += nLen;
		nBlockSize -= nSize;
	}
}
