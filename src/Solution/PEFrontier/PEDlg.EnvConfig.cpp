#include "stdafx.h"
#include "PEApp.h"
#include "PEDlg.EnvConfig.h"
#include "afxdialogex.h"


// CEnvConfig 대화 상자입니다.

IMPLEMENT_DYNAMIC(CEnvConfig, CDialogEx)

CEnvConfig::CEnvConfig(PCTSTR pszSymPath, CWnd* pParent /*=NULL*/)
	: CDialogEx(CEnvConfig::IDD, pParent)
	, m_szPdbSymPath(pszSymPath)
{
}

CEnvConfig::~CEnvConfig()
{
}

void CEnvConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_MDB_PDB_PATH, m_btnPdbPath);
	DDX_Text(pDX, IDC_MDB_PDB_PATH, m_szPdbSymPath);
}


BEGIN_MESSAGE_MAP(CEnvConfig, CDialogEx)
END_MESSAGE_MAP()


// CEnvConfig 메시지 처리기입니다.
BOOL CEnvConfig::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  
	SetDlgItemText(IDC_MDB_PDB_PATH, m_szPdbSymPath);

	return TRUE;
}


void CEnvConfig::OnOK()
{
	// TODO: 
	UpdateData();
	CDialogEx::OnOK();
}
