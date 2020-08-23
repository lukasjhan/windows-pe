#pragma once

class CProcList : public CDialogEx
{
	DECLARE_DYNAMIC(CProcList)

	DWORD	m_dwProcID;
	CString	m_szProcName;

	CListCtrl* LV() { return (CListCtrl*)GetDlgItem(IDC_LV_PROCESS); }

public:
	CProcList(CWnd* pParent = NULL);
	virtual ~CProcList();

	virtual BOOL OnInitDialog();
	virtual void OnOK();

	enum { IDD = IDD_PROC_LIST };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg void OnBnClickedBtnRefresh();
	afx_msg void OnNMDblclkLvProcess(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

public:
	DWORD GetProcID() { return m_dwProcID; }
	PCWSTR GetProcName() { return m_szProcName; }

};
