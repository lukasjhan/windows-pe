#pragma once

class CMemDump : public CDialogEx
{
	DECLARE_DYNAMIC(CMemDump)

	CBrush	m_brBkg;
	CFont	m_fnt;
	CSize	m_siMargin;

	bool	m_b32bit;
	PVOID	m_pBegin;
	PVOID	m_pMem;
	DWORD	m_dwSize;

public:
	CMemDump(bool bIs32bit, PVOID pBegin, PVOID pMem, DWORD dwSize, CWnd* pParent = NULL);
	virtual ~CMemDump();

	enum { IDD = IDD_MEM_DUMP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

};
