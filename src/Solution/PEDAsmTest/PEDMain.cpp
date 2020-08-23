#include "stdafx.h"
#include <Windows.h>
#include "..\PEDAsm\PEDAsm.h"


PCTSTR G_PSZ_OPTS[] = { L"-p", L"-r", L"-d" };
enum PRM_OPTS { PO_PE, PO_RAW, PO_DIR };

void PrintUsage()
{
	printf("PE Disassembler by YHD -> Usage:\n");
	printf("PEDAsm -p PEFilePath.\n");
	printf("       -r [-32 or -64] RwaFilePath.\n");
	printf("       -d [-32 or -64] Hex String: ex) \"0B 2D 3A\"\n");
}

PCTSTR ParseParams(int argc, _TCHAR* argv[], PRM_OPTS& po, bool& bIs32)
{
	if (argc < 2)
		return NULL;

	PCTSTR pszPrm = argv[1];
	if (argc > 2)
	{
		int i = 0;
		for (; i < 3; i++)
		{
			if (_tcsicmp(argv[1], G_PSZ_OPTS[i]) == 0)
			{
				po = PRM_OPTS(i);
				break;
			}
		}
		if (i == 3)
			return NULL;

		pszPrm = argv[2];
		if (po == PRM_OPTS::PO_RAW || po == PRM_OPTS::PO_DIR)
		{
			if (argc > 3)
			{
				if (argv[2][0] != L'-')
					return NULL;
				if (_tcsicmp(&argv[2][1], L"32") == 0)
					bIs32 = true;
				else if (_tcsicmp(&argv[2][1], L"64") != 0)
					return NULL;
				pszPrm = argv[3];
			}
		}
	}
	return pszPrm;
}

void _tmain(int argc, _TCHAR* argv[])
{
	PRM_OPTS po = PRM_OPTS::PO_PE;
	bool bIs32 = false;
	PCTSTR pszPrm = ParseParams(argc, argv, po, bIs32);
	if (pszPrm == NULL)
	{
		PrintUsage();
		return;
	}

	CString	szOut;
	HANDLE	hImgFile = INVALID_HANDLE_VALUE;
	HANDLE	hImgMap  = NULL;
	PBYTE	pImgView = NULL;
	try
	{
		DWORD dwOffset = 0;
		int nBuffSize = 0;

		if (po <= PRM_OPTS::PO_RAW)
		{
			hImgFile = CreateFile
			(
				pszPrm, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL
			);
			if (hImgFile == INVALID_HANDLE_VALUE)
				throw MAKE_HRESULT(1, FACILITY_WIN32, GetLastError());

			if (po == PRM_OPTS::PO_PE)
			{
				hImgMap = CreateFileMapping
				(
					hImgFile, NULL, PAGE_READONLY, 0, 0, NULL
				);
				if (hImgMap == NULL)
					throw MAKE_HRESULT(1, FACILITY_WIN32, GetLastError());
				pImgView = (PBYTE)MapViewOfFile(hImgMap, FILE_MAP_READ, 0, 0, 0);
				if (pImgView == NULL)
					throw MAKE_HRESULT(1, FACILITY_WIN32, GetLastError());

				PIMAGE_DOS_HEADER pdh = PIMAGE_DOS_HEADER(pImgView);
				if (pdh->e_magic != IMAGE_DOS_SIGNATURE)
					throw "PE 포맷을 가진 파일이 아닙니다.";

				dwOffset = pdh->e_lfanew;
				PIMAGE_NT_HEADERS pnh = PIMAGE_NT_HEADERS(pImgView + dwOffset);
				if (pnh->Signature != IMAGE_NT_SIGNATURE)
					throw "PE NT_HEADERS 포맷이 아닙니다.";
				bIs32 = (pnh->FileHeader.Machine == IMAGE_FILE_MACHINE_I386);

				PIMAGE_SECTION_HEADER pshs = IMAGE_FIRST_SECTION(pnh);
				PIMAGE_SECTION_HEADER ptxt = NULL;
				for (DWORD i = 0; i < pnh->FileHeader.NumberOfSections; i++)
				{
					char szName[9] = { 0, };
					memcpy(szName, pshs[i].Name, IMAGE_SIZEOF_SHORT_NAME);
					if (strcmp(szName, ".text") == 0)
					{
						ptxt = &pshs[i];
						break;
					}
				}
				dwOffset = ptxt->PointerToRawData;
				nBuffSize = ptxt->Misc.VirtualSize;
			}
			else
			{
				nBuffSize = GetFileSize(hImgFile, NULL);
				pImgView = new BYTE[nBuffSize];
				DWORD dwReadBytes = 0;
				ReadFile(hImgFile, pImgView, nBuffSize, &dwReadBytes, NULL);
			}
		}
		else
		{
			BYTE code = 0;
			BYTE arCode[512];
			bool bInit = false;
			int nLen = (int)_tcslen(pszPrm);
			for (int i = 0; i < nLen; i++)
			{
				TCHAR bt = pszPrm[i];
				if (bt == 0x20)
					continue;
				if (bt >= 0x30 && bt <= 0x39)
					bt -= 0x30;
				else if (bt >= L'a' && bt <= L'f')
					bt -= (L'a' - 0x0a);
				else if(bt >= L'A' && bt <= L'F')
					bt -= (L'A' - 0x0A);
				else
					continue;

				if (!bInit)
				{
					code = (BYTE)(bt << 4);
					bInit = true;
				}
				else
				{
					code |= (BYTE)bt;
					arCode[nBuffSize++] = code;
					code = 0;
					bInit = false;
				}
			}
			if (bInit)
			{
				code = (code >> 4);
				arCode[nBuffSize++] = code;
			}
			pImgView = new BYTE[nBuffSize];
			memcpy(pImgView, arCode, nBuffSize);
		}

		DECODED_INS dis[10];
		while (nBuffSize > 0)
		{
			int ndiCnt = sizeof(dis) / sizeof(DECODED_INS);
			int nReadBytes = PEDisAsm::DisAssemble(dis, ndiCnt, pImgView, dwOffset, nBuffSize, bIs32);
			nBuffSize -= nReadBytes;
			dwOffset  += nReadBytes;

			for (int i = 0; i < ndiCnt; i++)
			{
				DECODED dec;
				PEDisAsm::DecodedFormat(&dis[i], &dec, bIs32);
				CStringA fmt;
				for (int j = 0; j < dec.Count; j++)
					fmt.AppendFormat(" %02X", *(pImgView + dec.Offset + j));
				printf("%08X%-45s %s %s\n", dec.Offset, fmt, dec.Mnemonic,
					(dec.Operands != NULL) ? dec.Operands : "");
			}
		}
	}
	catch (HRESULT e)
	{
		printf("PE 입출력에 실패했습니다. Code = 0x%08X\n", e);
	}
	catch (PCTSTR e)
	{
		printf("%s\n", e);
	}

	if (pImgView != NULL)
	{
		if (po == PRM_OPTS::PO_PE)
			UnmapViewOfFile(pImgView);
		else
			delete[] pImgView;
	}
	if (hImgMap != NULL)
		CloseHandle(hImgMap);
	if (hImgFile != INVALID_HANDLE_VALUE)
		CloseHandle(hImgFile);
}
