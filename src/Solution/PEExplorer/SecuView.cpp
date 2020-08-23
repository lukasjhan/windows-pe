#include "stdafx.h"
#include <wincrypt.h>
#include <wintrust.h>
#include "PEExplorer.h"
#include "SecuView.h"
#include "afxdialogex.h"

#pragma comment(lib, "crypt32.lib")


// CSecuView 대화 상자입니다.
IMPLEMENT_DYNAMIC(CSecuView, CDialogEx)
CSecuView::CSecuView(PBYTE pData, int nSize, CWnd* pParent /*=NULL*/)
	: CDialogEx(CSecuView::IDD, pParent)
{
	m_pData = pData;
	m_nSize = nSize;
}

CSecuView::~CSecuView()
{
}

void CSecuView::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSecuView, CDialogEx)
END_MESSAGE_MAP()

// CSecuView 메시지 처리기입니다.
BOOL CSecuView::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	CListCtrl* pLv = (CListCtrl*)GetDlgItem(IDC_LV_ACCELTBL);
	DWORD dwExStyle = LVS_EX_FLATSB | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT;
	pLv->SetExtendedStyle(pLv->GetExtendedStyle() | dwExStyle);
	pLv->InsertColumn(0, L"필드", LVCFMT_LEFT, 140);
	pLv->InsertColumn(1, L"값", LVCFMT_LEFT, 300);
	ParseSecuData(pLv);

	return TRUE;
}

void CSecuView::ParseSecuData(CListCtrl* pLv)
{
	CERT_BLOB cb;
	cb.cbData = m_nSize;
	cb.pbData = m_pData;

	DWORD dwEnc, dwConType, dwFmtType;
	HCERTSTORE hStore = NULL;
	HCRYPTMSG hMsg = NULL;
	PCMSG_SIGNER_INFO psi = NULL, psi2 = NULL;
	PCCERT_CONTEXT pctx = NULL;

	try
	{
		BOOL bIsOK = CryptQueryObject(CERT_QUERY_OBJECT_BLOB, &cb,
			CERT_QUERY_CONTENT_FLAG_ALL, CERT_QUERY_FORMAT_FLAG_BINARY,
			0, &dwEnc, &dwConType, &dwFmtType, &hStore, &hMsg, NULL);
		if (!bIsOK)
			throw GetLastError();

		DWORD dwSigSize = 0;
		bIsOK = CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, NULL, &dwSigSize);
		if (!bIsOK)
			throw GetLastError();

		// Allocate memory for signer information.
		psi = (PCMSG_SIGNER_INFO)new BYTE[dwSigSize];
		// Get Signer Information.
		bIsOK = CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, psi, &dwSigSize);
		if (!bIsOK)
			throw GetLastError();

		// Get program name and publisher information from 
		// signer info structure.
		int nItem = 0;
		SPROG_PUBLISHERINFO pub;
		bIsOK = GetProgAndPublisherInfo(psi, &pub);
		if (bIsOK)
		{
			pLv->InsertItem(nItem, L"Program Name");
			pLv->SetItemText(nItem++, 1, pub.szProgName);
			pLv->InsertItem(nItem, L"Publisher Link");
			pLv->SetItemText(nItem++, 1, pub.szPubLink);
			pLv->InsertItem(nItem, L"MoreInfo Link");
			pLv->SetItemText(nItem++, 1, pub.szMoreLink);
		}

		CERT_INFO ci;
		ci.Issuer = psi->Issuer;
		ci.SerialNumber = psi->SerialNumber;
		pctx = CertFindCertificateInStore(hStore,
			ENCODING, 0, CERT_FIND_SUBJECT_CERT, &ci, NULL);
		if (!pctx)
			throw GetLastError();

		// Print Signer certificate information.
		pLv->InsertItem(nItem, L"Signer Certificate");
		pLv->SetItemText(nItem++, 1, L"**********************************");
		nItem = AppendCertificateInfo(pctx, pLv, nItem);
		CertFreeCertificateContext(pctx);

		// Get the timestamp certificate signerinfo structure.
		if (GetTimeStampSignerInfo(psi, &psi2))
		{
			// Search for Timestamp certificate in the temporary
			// certificate store.
			ci.Issuer = psi2->Issuer;
			ci.SerialNumber = psi2->SerialNumber;

			pctx = CertFindCertificateInStore(hStore,
				ENCODING, 0, CERT_FIND_SUBJECT_CERT, &ci, NULL);
			if (!pctx)
				throw GetLastError();

			// Print timestamp certificate information.
			pLv->InsertItem(nItem, L"TimeStamp Certificate");
			pLv->SetItemText(nItem++, 1, L"**********************************");
			nItem = AppendCertificateInfo(pctx, pLv, nItem);

			// Find Date of timestamp.
			SYSTEMTIME st;
			if (GetDateOfTimeStamp(psi2, &st))
			{
				CString sz;
				sz.Format(L"%04d/%02d/%02d %02d:%02d:%2d", 
					st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
				pLv->InsertItem(nItem, L"  Date of TimeStamp");
				pLv->SetItemText(nItem++, 1, sz);
			}
		}

	}
	catch (DWORD hr)
	{
		AfxMessageBox(L"Error occurred with code=0x%08X", hr);
	}
	if (pctx != NULL) CertFreeCertificateContext(pctx);
	if (psi != NULL) delete[]((PBYTE)psi);
	if (hStore != NULL) CertCloseStore(hStore, 0);
	if (hMsg != NULL) CryptMsgClose(hMsg);
}

int CSecuView::AppendCertificateInfo(PCCERT_CONTEXT pctx, CListCtrl* pLv, int nItem)
{
	TCHAR szName[MAX_PATH];
	CString sz;

	try
	{
		// Print Serial Number.
		DWORD dwData = pctx->pCertInfo->SerialNumber.cbData;
		for (DWORD i = 0; i < dwData; i++)
		{
			sz.AppendFormat(L"%02X ", pctx->pCertInfo->
				SerialNumber.pbData[dwData - (i + 1)]);
		}
		pLv->InsertItem(nItem, L"  Serial Number");
		pLv->SetItemText(nItem++, 1, sz);

		// Get Issuer name.
		BOOL bIsOK = CertGetNameString(pctx,
			CERT_NAME_SIMPLE_DISPLAY_TYPE,
			CERT_NAME_ISSUER_FLAG, NULL, szName, MAX_PATH);
		if (!bIsOK)
			throw GetLastError();
		pLv->InsertItem(nItem, L"  Issuer Name");
		pLv->SetItemText(nItem++, 1, szName);

		// Get subject name.
		bIsOK = CertGetNameString(pctx,
			CERT_NAME_SIMPLE_DISPLAY_TYPE,
			0, NULL, szName, MAX_PATH);
		if (!bIsOK)
			throw GetLastError();
		pLv->InsertItem(nItem, L"  Subject Name");
		pLv->SetItemText(nItem++, 1, szName);
	}
	catch (DWORD hr)
	{
		AfxMessageBox(L"Error occurred with code=0x%08X", hr);
	}

	return nItem;
}

BOOL CSecuView::GetProgAndPublisherInfo(PCMSG_SIGNER_INFO psi, PSPROG_PUBLISHERINFO ppub)
{
	BOOL fReturn = FALSE;
	PSPC_SP_OPUS_INFO pspc = NULL;
	DWORD dwData = 0;

	// Loop through authenticated attributes and find
	// SPC_SP_OPUS_INFO_OBJID OID.
	DWORD i = 0;
	for (; i < psi->AuthAttrs.cAttr; i++)
	{
		if (lstrcmpA(SPC_SP_OPUS_INFO_OBJID, psi->AuthAttrs.rgAttr[i].pszObjId) == 0)
			break;
	}
	if (i == psi->AuthAttrs.cAttr)
		return fReturn;

	try
	{
		// Get Size of SPC_SP_OPUS_INFO structure.
		BOOL bIsOK = CryptDecodeObject(ENCODING, SPC_SP_OPUS_INFO_OBJID,
			psi->AuthAttrs.rgAttr[i].rgValue[0].pbData,
			psi->AuthAttrs.rgAttr[i].rgValue[0].cbData,
			0, NULL, &dwData);
		if (!bIsOK)
			throw GetLastError();

		// Allocate memory for SPC_SP_OPUS_INFO structure.
		pspc = (PSPC_SP_OPUS_INFO)new BYTE[dwData];
		// Decode and get SPC_SP_OPUS_INFO structure.
		bIsOK = CryptDecodeObject(ENCODING,
			SPC_SP_OPUS_INFO_OBJID,
			psi->AuthAttrs.rgAttr[i].rgValue[0].pbData,
			psi->AuthAttrs.rgAttr[i].rgValue[0].cbData,
			0, pspc, &dwData);
		if (!bIsOK)
			throw GetLastError();

		// Fill in Program Name if present.
		if (pspc->pwszProgramName)
			ppub->szProgName = pspc->pwszProgramName;

		// Fill in Publisher Information if present.
		if (pspc->pPublisherInfo)
		{
			switch (pspc->pPublisherInfo->dwLinkChoice)
			{
				case SPC_URL_LINK_CHOICE:
					ppub->szPubLink = pspc->pPublisherInfo->pwszUrl;
				break;
				case SPC_FILE_LINK_CHOICE:
					ppub->szPubLink = pspc->pPublisherInfo->pwszFile;
				break;
			}
		}

		// Fill in More ppub if present.
		if (pspc->pMoreInfo)
		{
			switch (pspc->pMoreInfo->dwLinkChoice)
			{
				case SPC_URL_LINK_CHOICE:
					ppub->szMoreLink = pspc->pMoreInfo->pwszUrl;
				break;
				case SPC_FILE_LINK_CHOICE:
					ppub->szMoreLink = pspc->pMoreInfo->pwszFile;
				break;
			}
		}
		fReturn = TRUE;
	}
	catch (DWORD hr)
	{
		AfxMessageBox(L"Error occurred with code=0x%08X", hr);
	}
	if (pspc != NULL)
		delete[]((PBYTE)pspc);
	return fReturn;
}


BOOL CSecuView::GetDateOfTimeStamp(PCMSG_SIGNER_INFO pcsi, PSYSTEMTIME pst)
{
	// Loop through authenticated attributes and find szOID_RSA_signingTime OID.
	DWORD i = 0;
	for (; i < pcsi->AuthAttrs.cAttr; i++)
	{
		if (lstrcmpA(szOID_RSA_signingTime, pcsi->AuthAttrs.rgAttr[i].pszObjId) == 0)
			break;
	}
	if (i == pcsi->AuthAttrs.cAttr)
		return FALSE;

	// Decode and get FILETIME structure.
	FILETIME lft, ft;
	DWORD dwData = sizeof(ft);
	BOOL bIsOK = CryptDecodeObject(ENCODING, szOID_RSA_signingTime,
		pcsi->AuthAttrs.rgAttr[i].rgValue[0].pbData,
		pcsi->AuthAttrs.rgAttr[i].rgValue[0].cbData,
		0, (PVOID)&ft, &dwData);
	if (!bIsOK)
	{
		GetLastError();
		return FALSE;
	}

	// Convert to local time.
	FileTimeToLocalFileTime(&ft, &lft);
	FileTimeToSystemTime(&lft, pst);
	return TRUE;
}

BOOL CSecuView::GetTimeStampSignerInfo(PCMSG_SIGNER_INFO pcsi, PCMSG_SIGNER_INFO* ppcsi)
{
	BOOL fReturn = FALSE;
	BOOL bIsOK;
	DWORD dwSize;

	try
	{
		*ppcsi = NULL;

		// Loop through unathenticated attributes for
		// szOID_RSA_counterSign OID.
		DWORD i = 0;
		for (; i < pcsi->UnauthAttrs.cAttr; i++)
		{
			if (lstrcmpA(pcsi->UnauthAttrs.rgAttr[i].pszObjId, szOID_RSA_counterSign) == 0)
				break;
		}
		if (i == pcsi->UnauthAttrs.cAttr)
			return FALSE;

		// Get size of CMSG_SIGNER_INFO structure.
		bIsOK = CryptDecodeObject(ENCODING, PKCS7_SIGNER_INFO,
			pcsi->UnauthAttrs.rgAttr[i].rgValue[0].pbData,
			pcsi->UnauthAttrs.rgAttr[i].rgValue[0].cbData,
			0, NULL, &dwSize);
		if (!bIsOK)
			throw GetLastError();

		// Allocate memory for CMSG_SIGNER_INFO.
		*ppcsi = (PCMSG_SIGNER_INFO)LocalAlloc(LPTR, dwSize);
		if (!*ppcsi)
			throw GetLastError();

		// Decode and get CMSG_SIGNER_INFO structure
		// for timestamp certificate.
		bIsOK = CryptDecodeObject(ENCODING, PKCS7_SIGNER_INFO,
			pcsi->UnauthAttrs.rgAttr[i].rgValue[0].pbData,
			pcsi->UnauthAttrs.rgAttr[i].rgValue[0].cbData,
			0, *ppcsi, &dwSize);
		if (!bIsOK)
			throw GetLastError();

		fReturn = TRUE;
	}
	catch (DWORD hr)
	{
		AfxMessageBox(L"Error occurred with code=0x%08X", hr);
	}

	return fReturn;
}
