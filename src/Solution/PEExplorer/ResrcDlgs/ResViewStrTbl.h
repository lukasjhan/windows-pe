#pragma once


// CResViewStrTbl ��ȭ �����Դϴ�.

class CResViewStrTbl : public CDialogEx
{
	DECLARE_DYNAMIC(CResViewStrTbl)

	LPBYTE	m_pImgStart;
	UINT	m_uResID;
	DWORD	m_dwItemSize;

	void ParseStringTable(CListCtrl* pLV);

public:
	CResViewStrTbl(PBYTE, UINT, DWORD, CWnd* pParent = NULL);
	virtual ~CResViewStrTbl();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_RES_STR_TBL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
};
