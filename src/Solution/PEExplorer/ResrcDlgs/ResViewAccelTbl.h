#pragma once


// CResViewAccelTbl ��ȭ �����Դϴ�.
class CResViewAccelTbl : public CDialogEx
{
	DECLARE_DYNAMIC(CResViewAccelTbl)

	WORD	m_wResType;
	PBYTE	m_pImgStart;

	void ParseAccelatorTable(CListCtrl* pLv);
	void ParseGroupResInfo(CListCtrl* pLv);

public:
	CResViewAccelTbl(WORD wResType, PBYTE, CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CResViewAccelTbl();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_RES_ACCEL_TBL };

	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	DECLARE_MESSAGE_MAP()

};
