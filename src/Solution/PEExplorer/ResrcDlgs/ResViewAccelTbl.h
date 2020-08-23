#pragma once


// CResViewAccelTbl 대화 상자입니다.
class CResViewAccelTbl : public CDialogEx
{
	DECLARE_DYNAMIC(CResViewAccelTbl)

	WORD	m_wResType;
	PBYTE	m_pImgStart;

	void ParseAccelatorTable(CListCtrl* pLv);
	void ParseGroupResInfo(CListCtrl* pLv);

public:
	CResViewAccelTbl(WORD wResType, PBYTE, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CResViewAccelTbl();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_RES_ACCEL_TBL };

	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	DECLARE_MESSAGE_MAP()

};
