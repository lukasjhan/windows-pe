#pragma once
#include "afxeditbrowsectrl.h"


// CEnvConfig ��ȭ �����Դϴ�.

class CEnvConfig : public CDialogEx
{
	DECLARE_DYNAMIC(CEnvConfig)

public:
	CEnvConfig(PCTSTR pszSymPath, CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CEnvConfig();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_ENV_CONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

public:
	//CMFCEditBrowseCtrl m_btnPdbPath;
	CString m_szPdbSymPath;
	PCTSTR GetSymbolPath()	{ return m_szPdbSymPath; }

};
