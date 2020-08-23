#include "stdafx.h"
#include "PEApp.h"
#include "PEDlg.SimpleInput.h"
#include "afxdialogex.h"


IMPLEMENT_DYNAMIC(CSimpleInput, CDialogEx)
BEGIN_MESSAGE_MAP(CSimpleInput, CDialogEx)
	ON_BN_CLICKED(IDOK, &CSimpleInput::OnBnClickedOk)
END_MESSAGE_MAP()


CSimpleInput::CSimpleInput(PCWSTR pszTitle, PCWSTR pszDesc, CWnd* pParent /*=NULL*/)
	: CDialogEx(CSimpleInput::IDD, pParent)
	, m_szInput(pszDesc)
{
	m_szTitle = pszTitle;
}

CSimpleInput::~CSimpleInput()
{
}

void CSimpleInput::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDT_INPUT, m_szInput);
}

BOOL CSimpleInput::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	SetWindowText(m_szTitle);
	return TRUE;
}

void CSimpleInput::OnBnClickedOk()
{
	UpdateData();
	m_szInput.Trim();
	if (m_szInput.IsEmpty())
	{
		AfxMessageBox(L"항목이 비었습니다.");
		GetDlgItem(IDC_EDT_INPUT)->SetFocus();
		return;
	}
	CDialogEx::OnOK();
}
