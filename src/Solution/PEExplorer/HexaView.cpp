#include "stdafx.h"
#include "PEExplorer.h"
#include "PEAnals.h"
#include "HexaView.h"
#include "afxdialogex.h"


// CHexaView 대화 상자입니다.
IMPLEMENT_DYNAMIC(CHexaView, CDialogEx)

CHexaView::CHexaView(PPE_NODE pNode, PEAnals* pPE, CWnd* pParent /*=NULL*/)
	: CDialogEx(CHexaView::IDD, pParent)
{
	m_pPN = pNode;
	m_pPE	= pPE;
}

CHexaView::~CHexaView()
{
}

void CHexaView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHexaView, CDialogEx)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

BOOL CHexaView::OnInitDialog()
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

	CString szOutDump;
	SetDlgItemText(IDC_EDIT_COLUMN,
		_T("--------  +0 +1 +2 +3 +4 +5 +6 +7  +8 +9 +A +B +C +D +E +F  0123456789ABCDEF"));
	DWORD dwLineNo = m_pPN->Offset;
	PBYTE pBase = m_pPE->GetImgView() + dwLineNo;
	int nRow = m_pPN->Size / 16;
	int nMod = m_pPN->Size % 16;
	if (nMod > 0)
		nRow++;

	CString szTemp;
	for (int i = 0; i < nRow; i++)
	{
		szTemp.Format(L"%08X  ", dwLineNo);
		szOutDump += szTemp;

		int nCol = 16;
		if (i == nRow - 1 && nMod > 0)
			nCol = nMod;
		int j = 0;
		for (; j < nCol; j++)
		{
			BYTE ch = pBase[(i << 4) + j];
			szTemp.Format(_T("%02X "), ch);
			if (j == 7)
				szTemp += _T(' ');
			szOutDump += szTemp;
		}
		if (nCol < 16)
		{
			for (; j < 16; j++)
			{
				szTemp = _T("   ");
				if (j == 7)
					szTemp += _T(' ');
				szOutDump += szTemp;
			}
		}

		szOutDump += _T(' ');
		for (int j = 0; j < nCol; j++)
		{
			BYTE ch = pBase[(i << 4) + j];
			if (ch < 0x20 || ch > 0x7E)
				szOutDump += '.';
			else
				szOutDump += (TCHAR)ch;
		}

		dwLineNo += 0x10;
		szOutDump += "\xd\xa";
	}
	SetDlgItemText(IDC_EDT_HEXA, szOutDump);

	return TRUE;
}

// CHexaView 메시지 처리기입니다.
void CHexaView::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (GetDlgItem(IDC_EDT_HEXA)->GetSafeHwnd())
	{
		GetDlgItem(IDC_EDT_HEXA)->SetWindowPos
			(NULL, 0, 0, cx - m_siMargin.cx, cy - m_siMargin.cy, SWP_NOMOVE | SWP_NOZORDER);
	}

	if (GetDlgItem(IDC_EDIT_COLUMN)->GetSafeHwnd())
	{
		CRect rc;
		GetDlgItem(IDC_EDIT_COLUMN)->GetWindowRect(rc);
		GetDlgItem(IDC_EDIT_COLUMN)->SetWindowPos
			(NULL, 0, 0, cx - m_siMargin.cx, rc.Height(), SWP_NOMOVE | SWP_NOZORDER);
	}
}

void CHexaView::OnDestroy()
{
	CDialogEx::OnDestroy();
}

HBRUSH CHexaView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	if (nCtlColor == CTLCOLOR_MSGBOX || 
		nCtlColor == CTLCOLOR_EDIT || nCtlColor == CTLCOLOR_STATIC)
		return (HBRUSH)(m_brBkg.GetSafeHandle());
	return CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);
}
