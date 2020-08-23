#include "stdafx.h"
#include "PEApp.h"
#include "PEDlg.ExceptInfo.h"
#include "afxdialogex.h"


// CExceptInfo 대화 상자입니다.
IMPLEMENT_DYNAMIC(CExceptInfo, CDialogEx)

CExceptInfo::CExceptInfo(LPEXCEPTION_DEBUG_INFO pei, CWnd* pParent /*=NULL*/)
	: CDialogEx(CExceptInfo::IDD, pParent)
{
	m_pei = pei;
}

CExceptInfo::~CExceptInfo()
{
}

void CExceptInfo::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CExceptInfo, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_HALT, &CExceptInfo::OnBnClickedBtnHalt)
	ON_BN_CLICKED(IDC_BTN_CONTINUE, &CExceptInfo::OnBnClickedBtnContinue)
	ON_BN_CLICKED(IDC_BTN_IGNORE, &CExceptInfo::OnBnClickedBtnIgnore)
END_MESSAGE_MAP()


// CExceptInfo 메시지 처리기입니다.


BOOL CExceptInfo::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	String szMsg = PEPlus::GetErrMsg(m_pei->ExceptionRecord.ExceptionCode);
	SetDlgItemText(IDC_EDT_MSG, szMsg);

	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	LV()->SetExtendedStyle(LV()->GetExtendedStyle() | dwExStyle);

	LV()->InsertColumn(0, _T("필드"), LVCFMT_LEFT, 120);
	LV()->InsertColumn(1, _T("정보"), LVCFMT_LEFT, 200);

	CString sz;
	sz.Format(L"%d", m_pei->dwFirstChance);
	int nIdx = LV()->InsertItem(0, L"FirstChance");
	LV()->SetItemText(nIdx++, 1, sz);
	sz.Format(L"0x%08X", m_pei->ExceptionRecord.ExceptionCode);
	nIdx = LV()->InsertItem(nIdx, L"ExceptionCode");
	LV()->SetItemText(nIdx++, 1, sz);
	sz.Format(L"0x%08X", m_pei->ExceptionRecord.ExceptionFlags);
	nIdx = LV()->InsertItem(nIdx, L"ExceptionFlags");
	LV()->SetItemText(nIdx++, 1, sz);
	sz.Format(L"0x%s", PEPlus::GetAddrForm(false, 
		(PBYTE)m_pei->ExceptionRecord.ExceptionAddress));
	nIdx = LV()->InsertItem(nIdx, L"ExceptionAddress");
	LV()->SetItemText(nIdx++, 1, sz);
	sz.Format(L"0x%s", PEPlus::GetAddrForm(false,
		(PBYTE)m_pei->ExceptionRecord.ExceptionRecord));
	nIdx = LV()->InsertItem(nIdx, L"ExceptionRecord");
	LV()->SetItemText(nIdx++, 1, sz);
	sz.Format(L"%d", m_pei->ExceptionRecord.NumberParameters);
	nIdx = LV()->InsertItem(nIdx, L"NumberParameters");
	LV()->SetItemText(nIdx++, 1, sz);

	if (m_pei->ExceptionRecord.NumberParameters > 0)
	{
		sz.Empty();
		for (DWORD i = 0; i < m_pei->ExceptionRecord.NumberParameters; i++)
		{
			if (!sz.IsEmpty())
				sz.Append(L", ");
			sz.AppendFormat(L"%p", (PBYTE)m_pei->ExceptionRecord.ExceptionInformation[i]);
		}
		nIdx = LV()->InsertItem(nIdx, L"ExceptionInformations");
		LV()->SetItemText(nIdx++, 1, sz);
	}

	return TRUE;
}

void CExceptInfo::OnBnClickedBtnHalt()
{
	EndDialog(IDABORT);
}

void CExceptInfo::OnBnClickedBtnContinue()
{
	EndDialog(IDCONTINUE);
}

void CExceptInfo::OnBnClickedBtnIgnore()
{
	EndDialog(IDIGNORE);
}

void CExceptInfo::OnCancel()
{
	EndDialog(IDABORT);
}
