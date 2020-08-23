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

// CNewDbgPrj 대화 상자
class CNewDbgPrj : public CDialogEx
{
	DECLARE_DYNAMIC(CNewDbgPrj)

public:
	CNewDbgPrj(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CNewDbgPrj();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_NEW_DBGPRJ };
	CMFCEditBWEx m_mbtnExe;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

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
