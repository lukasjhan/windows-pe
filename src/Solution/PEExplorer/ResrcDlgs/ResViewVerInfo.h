#pragma once

typedef VS_FIXEDFILEINFO*	PVS_FIXEDFILEINFO;
#define VSFI_SIGNATURE		0xFEEF04BD

typedef struct
{
	WORD	wLength;
	WORD	wValLen;
	WORD	wType;
} VAR_VER_TYPE, *PVAR_VER_TYPE;


class CResViewVerInfo : public CDialogEx
{
	DECLARE_DYNAMIC(CResViewVerInfo)

	PBYTE	m_pImgStart;

	void BuildVersionInfo(PBYTE pIter, int nSize, CListCtrl* pLv, int nDepth, int& nItem);
	void BuildFixedVerInfo(PVS_FIXEDFILEINFO pvfi, CListCtrl* pLv, int nDepth, int& nItem);

public:
	CResViewVerInfo(PBYTE, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CResViewVerInfo();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_RES_VER_INFO };

	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	DECLARE_MESSAGE_MAP()

};
