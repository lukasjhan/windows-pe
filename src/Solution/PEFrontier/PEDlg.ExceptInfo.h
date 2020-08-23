#pragma once

class CExceptInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CExceptInfo)

	LPEXCEPTION_DEBUG_INFO m_pei;
	CListCtrl* LV() { return (CListCtrl*)GetDlgItem(IDC_LV_DETAIL); }

public:
	CExceptInfo(LPEXCEPTION_DEBUG_INFO pei, CWnd* pParent = NULL);
	virtual ~CExceptInfo();

	virtual BOOL OnInitDialog();
	virtual void OnCancel();

	enum { IDD = IDD_EXCEPT_INFO };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	afx_msg void OnBnClickedBtnHalt();
	afx_msg void OnBnClickedBtnContinue();
	afx_msg void OnBnClickedBtnIgnore();
	DECLARE_MESSAGE_MAP()

public:

};
