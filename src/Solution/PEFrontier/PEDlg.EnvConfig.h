#pragma once
#include "afxeditbrowsectrl.h"


// CEnvConfig 대화 상자입니다.

class CEnvConfig : public CDialogEx
{
	DECLARE_DYNAMIC(CEnvConfig)

public:
	CEnvConfig(PCTSTR pszSymPath, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CEnvConfig();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ENV_CONFIG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();
	virtual void OnOK();

	DECLARE_MESSAGE_MAP()

public:
	//CMFCEditBrowseCtrl m_btnPdbPath;
	CString m_szPdbSymPath;
	PCTSTR GetSymbolPath()	{ return m_szPdbSymPath; }

};
