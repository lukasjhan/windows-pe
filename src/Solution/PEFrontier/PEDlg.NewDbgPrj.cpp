#include "stdafx.h"
#include "PEApp.h"
#include "PEDlg.NewDbgPrj.h"
#include "afxdialogex.h"


// CNewDbgPrj ��ȭ �����Դϴ�.
IMPLEMENT_DYNAMIC(CNewDbgPrj, CDialogEx)

CNewDbgPrj::CNewDbgPrj(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNewDbgPrj::IDD, pParent)
	, m_szPrjPath(_T(""))
	, m_szPrjName(_T(""))
	, m_szExePath(_T(""))
	, m_bIsProc(FALSE)
	, m_szExeArgs(_T(""))
{

}

CNewDbgPrj::~CNewDbgPrj()
{
}

void CNewDbgPrj::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MDB_PRJ_PATH, m_szPrjPath);
	DDX_Text(pDX, IDC_EDT_PRJ_NAME, m_szPrjName);
	DDX_Text(pDX, IDC_MDB_EXE_PATH, m_szExePath);
	DDX_Control(pDX, IDC_MDB_EXE_PATH, m_mbtnExe);
	DDX_Radio(pDX, IDC_RDO_PEFILE, m_bIsProc);
	DDX_Text(pDX, IDC_EDT_ARGS, m_szExeArgs);
}

BEGIN_MESSAGE_MAP(CNewDbgPrj, CDialogEx)
	ON_BN_CLICKED(IDC_RDO_PEFILE, &CNewDbgPrj::OnBnClickedRdoPefile)
	ON_BN_CLICKED(IDC_RDO_PROCESS, &CNewDbgPrj::OnBnClickedRdoPefile)
END_MESSAGE_MAP()

BOOL CNewDbgPrj::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: 
	((CMFCEditBrowseCtrl*)GetDlgItem(IDC_MDB_EXE_PATH))->
		EnableFileBrowseButton(L"exe", L"PE ���� ���� (*.exe)|*.exe||");
	return TRUE;
}

void CNewDbgPrj::OnBnClickedRdoPefile()
{
	// TODO: 
	UpdateData();
	CMFCEditBrowseCtrl* pbw = ((CMFCEditBrowseCtrl*)GetDlgItem(IDC_MDB_EXE_PATH));
	if (m_bIsProc)
		pbw->EnableBrowseButton();
	else
		pbw->EnableFileBrowseButton(L"exe", L"PE ���� ���� (*.exe)|*.exe||");
}

void CNewDbgPrj::OnOK()
{
	UpdateData();

	if (m_szExePath.IsEmpty() || m_szPrjName.IsEmpty() || m_szPrjPath.IsEmpty())
	{
		AfxMessageBox(L"�׸��� ��� �Է��� �ּ���...");
		return;
	}
	CDialogEx::OnOK();
}
