#include "stdafx.h"
#include <windows.h>
#include <wincrypt.h>
#include <wintrust.h>

#pragma comment(lib, "crypt32.lib")

#define ENCODING (X509_ASN_ENCODING | PKCS_7_ASN_ENCODING)

void PrintCertificateInfo(PCCERT_CONTEXT pctx);
void PrintProgAndPublisherInfo(PCMSG_SIGNER_INFO psi);
BOOL GetDateOfTimeStamp(PCMSG_SIGNER_INFO pcsi, PSYSTEMTIME pst);
PCMSG_SIGNER_INFO GetTimeStampSignerInfo(PCMSG_SIGNER_INFO pcsi);

int _tmain(int argc, TCHAR *argv[])
{
	if (argc != 2)
	{
		printf("Usage: SignedFileInfo <filename>\n");
		return 0;
	}

	DWORD dwEnc, dwConType, dwFmtType;
	HCERTSTORE hStore = NULL;
	HCRYPTMSG hMsg = NULL;
	PCMSG_SIGNER_INFO psi = NULL, psi2 = NULL;
	PCCERT_CONTEXT pctx = NULL;

	try
	{
		BOOL bIsOK = CryptQueryObject(CERT_QUERY_OBJECT_FILE, argv[1],
			CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED, CERT_QUERY_FORMAT_FLAG_BINARY,
			0, &dwEnc, &dwConType, &dwFmtType, &hStore, &hMsg, NULL);
		if (!bIsOK)
			throw GetLastError();

		DWORD dwSize = 0;
		for (int j = 0; j < 2; j++)
		{
			if (dwSize > 0)
				psi = (PCMSG_SIGNER_INFO)new BYTE[dwSize];
			bIsOK = CryptMsgGetParam(hMsg, CMSG_SIGNER_INFO_PARAM, 0, psi, &dwSize);
			if (!bIsOK)
				throw GetLastError();
		}
		PrintProgAndPublisherInfo(psi);
		printf("\n");

		CERT_INFO ci;
		ci.Issuer = psi->Issuer;
		ci.SerialNumber = psi->SerialNumber;
		pctx = CertFindCertificateInStore(hStore,
			ENCODING, 0, CERT_FIND_SUBJECT_CERT, &ci, NULL);
		if (!pctx)
			throw GetLastError();
		printf("Signer Certificate:\n\n");
		PrintCertificateInfo(pctx);
		CertFreeCertificateContext(pctx); pctx = NULL;
		printf("\n");

		psi2 = GetTimeStampSignerInfo(psi);
		if (psi2)
		{
			ci.Issuer = psi2->Issuer;
			ci.SerialNumber = psi2->SerialNumber;
			pctx = CertFindCertificateInStore(hStore,
				ENCODING, 0, CERT_FIND_SUBJECT_CERT, &ci, NULL);
			if (!pctx)
				throw GetLastError();
			printf("TimeStamp Certificate:\n\n");
			PrintCertificateInfo(pctx);
			printf("\n");

			SYSTEMTIME st;
			if (GetDateOfTimeStamp(psi2, &st))
			{
				CString sz;
				sz.Format(L"%04d/%02d/%02d %02d:%02d:%2d",
					st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute, st.wSecond);
				printf("Date of TimeStamp : %S\n", sz);
			}
		}
	}
	catch (DWORD hr)
	{
		printf("Error occurred with code=0x%08X", hr);
	}

	// Clean up.
	if (pctx != NULL) CertFreeCertificateContext(pctx);
	if (psi2 != NULL) delete[]((PBYTE)psi2);
	if (psi != NULL) delete[]((PBYTE)psi);
	if (hStore != NULL) CertCloseStore(hStore, 0);
	if (hMsg != NULL) CryptMsgClose(hMsg);
	return 0;
}

void PrintProgAndPublisherInfo(PCMSG_SIGNER_INFO psi)
{
	DWORD i = 0;
	for (; i < psi->AuthAttrs.cAttr; i++)
	{
		if (lstrcmpA(SPC_SP_OPUS_INFO_OBJID, psi->AuthAttrs.rgAttr[i].pszObjId) == 0)
			break;
	}
	// Loop through authenticated attributes and find SPC_SP_OPUS_INFO_OBJID OID.
	if (i == psi->AuthAttrs.cAttr)
		return;

	BOOL bRetOK = TRUE;
	PSPC_SP_OPUS_INFO pspc = NULL;
	try
	{
		BOOL bIsOK = FALSE;
		DWORD dwSize = 0;
		for (int j = 0; j < 2; j++)
		{
			if (dwSize > 0)
				pspc = (PSPC_SP_OPUS_INFO)new BYTE[dwSize];
			bIsOK = CryptDecodeObject(ENCODING, SPC_SP_OPUS_INFO_OBJID,
				psi->AuthAttrs.rgAttr[i].rgValue[0].pbData,
				psi->AuthAttrs.rgAttr[i].rgValue[0].cbData,
				0, pspc, &dwSize);
			if (!bIsOK)
				throw GetLastError();
			// Get Size of SPC_SP_OPUS_INFO structure.
			// Allocate memory for SPC_SP_OPUS_INFO structure.
			// Decode and get SPC_SP_OPUS_INFO structure.
		}

		CString szProgName, szPubLink, szMoreLink;
		// Fill in Program Name if present.
		if (pspc->pwszProgramName)
			szProgName = pspc->pwszProgramName;
		// Fill in Publisher Information if present.
		if (pspc->pPublisherInfo)
		{
			switch (pspc->pPublisherInfo->dwLinkChoice)
			{
				case SPC_URL_LINK_CHOICE:
					szPubLink = pspc->pPublisherInfo->pwszUrl;
				break;
				case SPC_FILE_LINK_CHOICE:
					szPubLink = pspc->pPublisherInfo->pwszFile;
				break;
			}
		}
		// Fill in More ppub if present.
		if (pspc->pMoreInfo)
		{
			switch (pspc->pMoreInfo->dwLinkChoice)
			{
				case SPC_URL_LINK_CHOICE:
					szMoreLink = pspc->pMoreInfo->pwszUrl;
				break;
				case SPC_FILE_LINK_CHOICE:
					szMoreLink = pspc->pMoreInfo->pwszFile;
				break;
			}
		}
		printf("Program Name   : %S\n", szProgName);
		printf("Publisher Link : %S\n", szPubLink);
		printf("MoreInfo Link  : %S\n", szMoreLink);
	}
	catch (DWORD hr)
	{
		printf("Error occurred with code=0x%08X", hr);
	}
	if (pspc != NULL)
		delete[]((PBYTE)pspc);
}

void PrintCertificateInfo(PCCERT_CONTEXT pctx)
{
	TCHAR szName[MAX_PATH];
	try
	{
		// Print Serial Number.
		CString sz;
		DWORD dwData = pctx->pCertInfo->SerialNumber.cbData;
		for (DWORD i = 0; i < dwData; i++)
		{
			sz.AppendFormat(L"%02X ", pctx->pCertInfo->
				SerialNumber.pbData[dwData - (i + 1)]);
		}
		printf("  Serial Number: %S\n", sz);

		// Get Issuer name.
		BOOL bIsOK = CertGetNameString(pctx,
			CERT_NAME_SIMPLE_DISPLAY_TYPE,
			CERT_NAME_ISSUER_FLAG, NULL, szName, MAX_PATH);
		if (!bIsOK)
			throw GetLastError();
		printf("  Issuer Name  : %S\n", szName);

		// Get subject name.
		bIsOK = CertGetNameString(pctx,
			CERT_NAME_SIMPLE_DISPLAY_TYPE,
			0, NULL, szName, MAX_PATH);
		if (!bIsOK)
			throw GetLastError();
		printf("  Subject Name : %S\n", szName);
	}
	catch (DWORD hr)
	{
		printf("Error occurred with code=0x%08X", hr);
	}
}

BOOL GetDateOfTimeStamp(PCMSG_SIGNER_INFO pcsi, PSYSTEMTIME pst)
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

PCMSG_SIGNER_INFO GetTimeStampSignerInfo(PCMSG_SIGNER_INFO pcsi)
{
	// Loop through unathenticated attributes for
	// szOID_RSA_counterSign OID.
	DWORD i = 0;
	for (; i < pcsi->UnauthAttrs.cAttr; i++)
	{
		if (lstrcmpA(pcsi->UnauthAttrs.rgAttr[i].pszObjId, szOID_RSA_counterSign) == 0)
			break;
	}
	if (i == pcsi->UnauthAttrs.cAttr)
		return NULL;

	PCMSG_SIGNER_INFO pcsiRet = NULL;
	try
	{
		// Get size of CMSG_SIGNER_INFO structure.
		// Allocate memory for CMSG_SIGNER_INFO.
		// Decode and get CMSG_SIGNER_INFO structure
		// for timestamp certificate.
		DWORD dwSize = 0;
		BOOL bIsOK = CryptDecodeObject(ENCODING, PKCS7_SIGNER_INFO,
			pcsi->UnauthAttrs.rgAttr[i].rgValue[0].pbData,
			pcsi->UnauthAttrs.rgAttr[i].rgValue[0].cbData,
			0, NULL, &dwSize);
		if (!bIsOK)
			throw GetLastError();

		pcsiRet = (PCMSG_SIGNER_INFO)new BYTE[dwSize];
		bIsOK = CryptDecodeObject(ENCODING, PKCS7_SIGNER_INFO,
			pcsi->UnauthAttrs.rgAttr[i].rgValue[0].pbData,
			pcsi->UnauthAttrs.rgAttr[i].rgValue[0].cbData,
			0, pcsiRet, &dwSize);
		if (!bIsOK)
			throw GetLastError();
	}
	catch (DWORD hr)
	{
		printf("Error occurred with code=0x%08X", hr);
		if (pcsiRet) delete[]((PBYTE)pcsiRet);
		return NULL;
	}
	return pcsiRet;
}