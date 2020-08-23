#pragma once
#include "afxeditbrowsectrl.h"

class CMFCEditBWEx : public CMFCEditBrowseCtrl
{
	virtual void OnBrowse()
	{
		if (this->GetMode() != CMFCEditBrowseCtrl::BrowseMode_Default)
			CMFCEditBrowseCtrl::OnBrowse();
		else
		{
			AfxMessageBox(L"Normal...");
		}
	}
};

// CNewDbgPrj ��ȭ ����
class CNewDbgPrj : public CDialogEx
{
	DECLARE_DYNAMIC(CNewDbgPrj)

public:
	CNewDbgPrj(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CNewDbgPrj();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_NEW_DBGPRJ };
	CMFCEditBWEx m_mbtnExe;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	afx_msg void OnBnClickedRdoPefile();
	DECLARE_MESSAGE_MAP()

public:
	CString m_szPrjPath;
	CString m_szPrjName;
	CString m_szExePath;
	CString m_szExeArgs;
	BOOL	m_bIsProc;

	virtual void OnOK();
	virtual BOOL OnInitDialog();

};
