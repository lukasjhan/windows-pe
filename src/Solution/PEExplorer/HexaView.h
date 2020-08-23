#pragma once

// CHexaView 대화 상자입니다.
class CHexaView : public CDialogEx
{
	DECLARE_DYNAMIC(CHexaView)

	CBrush	 m_brBkg;
	CSize	 m_siMargin;

	PPE_NODE m_pPN;
	PEAnals* m_pPE;

public:
	CHexaView(PPE_NODE pNode, PEAnals* pPE, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CHexaView();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_MEMORY_DUMP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

public:

};
