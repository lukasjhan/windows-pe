#include "stdafx.h"
#include "PEApp.h"
#include "PEDlg.MemDump.h"
#include "afxdialogex.h"


// CMemDump 대화 상자입니다.
IMPLEMENT_DYNAMIC(CMemDump, CDialogEx)

CMemDump::CMemDump(bool bIs32bit, PVOID pBegin, 
	PVOID pMem, DWORD dwSize, CWnd* pParent/*=NULL*/)
	: CDialogEx(CMemDump::IDD, pParent)
{
	m_b32bit = bIs32bit;
	m_pBegin = pBegin;
	m_pMem	 = pMem;
	m_dwSize = dwSize;
}

CMemDump::~CMemDump()
{
}

void CMemDump::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMemDump, CDialogEx)
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CMemDump 메시지 처리기입니다.
BOOL CMemDump::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CRect rc;
	GetClientRect(rc);
	CRect rcEdit;
	GetDlgItem(IDC_EDT_HEXA)->GetWindowRect(rcEdit);
	ScreenToClient(rcEdit);

	m_siMargin.cx = rc.Width() - rcEdit.Width();
	m_siMargin.cy = rc.Height() - rcEdit.Height();

	m_brBkg.CreateSolidBrush(RGB(255, 255, 255));
	m_fnt.CreateFont(14, 0, 0, 0, 0, 0, 0, 0, DEFAULT_CHARSET,
		0, 0, 0, DEFAULT_PITCH | FF_DONTCARE, _T("Courier New")); //_T("Lucida Console"));
	GetDlgItem(IDC_EDT_HEXA)->SetFont(&m_fnt);
	GetDlgItem(IDC_EDIT_COLUMN)->SetFont(&m_fnt);

	CString sz;
	if (m_b32bit)sz = L"--------  ";
	else		 sz = L"-----------------  ";
	sz.Append(L"+0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F  0123456789ABCDEF");
	SetDlgItemText(IDC_EDIT_COLUMN, sz);

	ULONG_PTR dwLineNo = (ULONG_PTR)m_pBegin;
	PBYTE pBase = (PBYTE)m_pMem;
	int nRow = m_dwSize / 16;
	int nMod = m_dwSize % 16;
	if (nMod > 0)
		nRow++;

	BeginWaitCursor();
	CString szDump;
	for (int i = 0; i < nRow; i++)
	{
		sz.Format(L"%s  ", PEPlus::GetAddrForm(m_b32bit, (PBYTE)dwLineNo));
		szDump += sz;

		int nCol = 16;
		if (i == nRow - 1 && nMod > 0)
			nCol = nMod;
		int j = 0;
		for (; j < nCol; j++)
		{
			BYTE ch = pBase[(i << 4) + j];
			sz.Format(_T("%02X "), ch);
			if (j == 7)
				sz += _T(' ');
			szDump += sz;
		}
		if (nCol < 16)
		{
			for (; j < 16; j++)
			{
				sz = _T("   ");
			if (j == 7)
					sz += _T(' ');
				szDump += sz;
			}
		}

		szDump += _T(' ');
		for (int j = 0; j < nCol; j++)
		{
			BYTE ch = pBase[(i << 4) + j];
			if (ch < 0x20 || ch > 0x7E)
				szDump += '.';
			else
				szDump += (TCHAR)ch;
		}

		dwLineNo += 0x10;
		szDump += "\xd\xa";
	}
	SetDlgItemText(IDC_EDT_HEXA, szDump);
	((CEdit*)GetDlgItem(IDC_EDT_HEXA))->SetSel(0, 0);
	EndWaitCursor();
	return TRUE;
}

void CMemDump::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (GetDlgItem(IDC_EDT_HEXA)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDT_HEXA)->SetWindowPos
			(NULL, 0, 0, cx - m_siMargin.cx, cy - m_siMargin.cy, SWP_NOMOVE | SWP_NOZORDER);
	}

	if (GetDlgItem(IDC_EDT_HEXA)->GetSafeHwnd())
	{
		CRect rc;
		GetDlgItem(IDC_EDIT_COLUMN)->GetWindowRect(rc);
		GetDlgItem(IDC_EDIT_COLUMN)->SetWindowPos
			(NULL, 0, 0, cx - m_siMargin.cx, rc.Height(), SWP_NOMOVE | SWP_NOZORDER);
	}
}

HBRUSH CMemDump::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_MSGBOX ||
		nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_STATIC)
		return (HBRUSH)(m_brBkg.GetSafeHandle());
	return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
}
