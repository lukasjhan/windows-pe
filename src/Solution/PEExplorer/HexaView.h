#pragma once

// CHexaView ��ȭ �����Դϴ�.
class CHexaView : public CDialogEx
{
	DECLARE_DYNAMIC(CHexaView)

	CBrush	 m_brBkg;
	CSize	 m_siMargin;

	PPE_NODE m_pPN;
	PEAnals* m_pPE;

public:
	CHexaView(PPE_NODE pNode, PEAnals* pPE, CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CHexaView();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_MEMORY_DUMP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	virtual BOOL OnInitDialog();

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

public:

};
