#pragma once

#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)

typedef struct
{
	CString	szProgName;
	CString	szPubLink;
	CString	szMoreLink;
} SPROG_PUBLISHERINFO, *PSPROG_PUBLISHERINFO;


class CSecuView : public CDialogEx
{
	DECLARE_DYNAMIC(CSecuView)

	PBYTE	m_pData;
	int		m_nSize;

	void ParseSecuData(CListCtrl*);
	BOOL GetProgAndPublisherInfo(PCMSG_SIGNER_INFO psi, PSPROG_PUBLISHERINFO ppub);
	BOOL GetDateOfTimeStamp(PCMSG_SIGNER_INFO pcsi, PSYSTEMTIME pst);
	BOOL GetTimeStampSignerInfo(PCMSG_SIGNER_INFO pcsi, PCMSG_SIGNER_INFO* ppcsi);
	int AppendCertificateInfo(PCCERT_CONTEXT pctx, CListCtrl* pLv, int nItem);

public:
	CSecuView(PBYTE, int, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CSecuView();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_SECU_VIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

public:

};
