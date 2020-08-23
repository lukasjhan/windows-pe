#include "stdafx.h"
#include "PEExplorer.h"
#include "FindItemDlg.h"
#include "afxdialogex.h"


// CFindItemDlg ��ȭ �����Դϴ�.

IMPLEMENT_DYNAMIC(CFindItemDlg, CDialogEx)

CFindItemDlg::CFindItemDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFindItemDlg::IDD, pParent)
	, m_szVal(_T(""))
{

}

CFindItemDlg::~CFindItemDlg()
{
}

void CFindItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDT_FIND, m_szVal);
}


BEGIN_MESSAGE_MAP(CFindItemDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CFindItemDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CFindItemDlg �޽��� ó�����Դϴ�.
void CFindItemDlg::OnBnClickedOk()
{
	UpdateData();
	if (m_szVal.Trim().IsEmpty())
		return;

	CDialogEx::OnOK();
}
