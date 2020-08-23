#pragma once


// CFindItemDlg 대화 상자입니다.

class CFindItemDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CFindItemDlg)


public:
	CFindItemDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CFindItemDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_FIND_ITEM };
	CString m_szVal;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};
