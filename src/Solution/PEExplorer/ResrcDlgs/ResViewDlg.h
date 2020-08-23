#pragma once


class CResViewDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CResViewDlg)

	PBYTE	m_pImgStart;
	bool	m_bIsExt;

	static LRESULT CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	void ParseDialogTemplate(CListCtrl*);

public:
	CResViewDlg(PBYTE, CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CResViewDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_RES_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedBtnShowParse();
	afx_msg void OnBnClickedBtnShowDialog();
	DECLARE_MESSAGE_MAP()

};
