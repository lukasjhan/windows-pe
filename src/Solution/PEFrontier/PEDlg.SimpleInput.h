#pragma once

class CSimpleInput : public CDialogEx
{
	DECLARE_DYNAMIC(CSimpleInput)
	CString m_szTitle;

public:
	CSimpleInput(PCWSTR pszTitle, PCWSTR pszDesc, CWnd* pParent = NULL);
	virtual ~CSimpleInput();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_SIMPLE_INPUT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg void OnBnClickedOk();
	DECLARE_MESSAGE_MAP()

public:
	CString m_szInput;

	virtual BOOL OnInitDialog();
};
