#pragma once

class CDllInjectorDlg : public CDialogEx
{
public:
	CDllInjectorDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_DLLINJECTOR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	HMODULE FindModule(PCWSTR pszDllName, DWORD dwProcId);

// 구현입니다.
protected:
	HICON m_hIcon;

	virtual BOOL OnInitDialog();

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnClickedBtnProcList();
	afx_msg void OnBnClickedBtnInject();
	afx_msg void OnBnClickedBtnEject();
	DECLARE_MESSAGE_MAP()

public:
	int m_dwProcID;
	CString m_szDllPath;

};
