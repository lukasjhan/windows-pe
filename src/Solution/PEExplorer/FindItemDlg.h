#pragma once


// CFindItemDlg ��ȭ �����Դϴ�.

class CFindItemDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFindItemDlg)


public:
	CFindItemDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CFindItemDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_FIND_ITEM };
	CString m_szVal;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
