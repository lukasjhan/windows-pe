#include "StdAfx.h"
#include "PESchema.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
// TypePrimtv class Definition
//
PCTSTR PE_MEMBER::C_Names[PE_TYPE::Struct] =
{
	_T("BYTE"), _T("WORD"), _T("DWORD"), _T("ULONGLONG"),
	_T("SBYTE"), _T("SHORT"), _T("INT"), _T("LONG64")
};
short PE_MEMBER::C_Sizes[PE_TYPE::Struct] =
{
	1, 2, 4, 8, 1, 2, 4, 8
};


PE_TYPE PE_MEMBER::GetDefType(PCTSTR pszType)
{
	short i = 0;
	for (; i < PE_TYPE::Struct; i++)
	{
		if (_tcsicmp(pszType, C_Names[i]) == 0)
			break;
	}
	return (PE_TYPE)i;
}
////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////
// 
CString PE_SCHEMA::Bin2Str(PBYTE pBin, PE_TYPE dt, int nLen)
{
	CString szBin;
	for (int i = 0; i < nLen; i++)
	{
		if (dt == PE_TYPE::UInt8 || dt == PE_TYPE::Int8)
			szBin.AppendFormat(_T("%02X "), pBin[i]);
		else if (dt == PE_TYPE::UInt16 || dt == PE_TYPE::Int16)
			szBin.AppendFormat(_T("%04X "), ((PWORD)pBin)[i]);
		else if (dt == PE_TYPE::UInt32 || dt == PE_TYPE::Int32)
			szBin.AppendFormat(_T("%08X "), ((PINT)pBin)[i]);
		else
			szBin.AppendFormat(_T("%016I64X "), ((PLONGLONG)pBin)[i]);
	}
	return szBin.Mid(0, szBin.GetLength() - 1);
}

UINT64 PE_SCHEMA::GetValue(PBYTE pBin, PE_TYPE dt)
{
	UINT64 uv = 0;
	switch (dt)
	{
		case PE_TYPE::UInt32:	uv = *(PUINT32(pBin)); break;
		case PE_TYPE::UInt16:	uv = *(PUINT16(pBin)); break;
		case PE_TYPE::UInt64:	uv = *(PUINT64(pBin)); break;
		case PE_TYPE::UInt8:	uv = *(PUINT8(pBin)); break;
		case PE_TYPE::Int32:	uv = *(PINT32(pBin)); break;
		case PE_TYPE::Int16:	uv = *(PINT16(pBin)); break;
		case PE_TYPE::Int64:	uv = *(PINT64(pBin)); break;
		case PE_TYPE::Int8:		uv = *(PINT8(pBin)); break;
	}
	return uv;
}

CString PE_SCHEMA::Val2Str(PBYTE pBin, PE_TYPE dt)
{
	CString szVal;
	switch (dt)
	{
		case PE_TYPE::UInt32:
			szVal.Format(_T("0x%08X" ), *(PUINT32(pBin))); break;
		case PE_TYPE::UInt16:
			szVal.Format(_T("0x%04X" ), *(PUINT16(pBin))); break;
		case PE_TYPE::UInt64:
			szVal.Format(_T("0x%016I64X"), *(PUINT64(pBin))); break;
		case PE_TYPE::UInt8:
			szVal.Format(_T("0x%02X" ), *(PUINT8(pBin))); break;
		case PE_TYPE::Int32:
			szVal.Format(_T("0x%08X" ), *(PINT32(pBin))); break;
		case PE_TYPE::Int16:
			szVal.Format(_T("0x%04X" ), *(PINT16(pBin))); break;
		case PE_TYPE::Int64:
			szVal.Format(_T("0x%016I64X"), *(PINT64(pBin))); break;
		case PE_TYPE::Int8:
			szVal.Format(_T("0x%02X" ), *(PINT8(pBin))); break;
	}
	return szVal;
}

CString PE_SCHEMA::Val2Str(ULONGLONG val, PE_TYPE dt)
{
	CString szVal;
	switch (dt)
	{
		case PE_TYPE::UInt32:
			szVal.Format(_T("0x%08X" ), UINT32(val)); break;
		case PE_TYPE::UInt16:
			szVal.Format(_T("0x%04X" ), UINT16(val)); break;
		case PE_TYPE::UInt64:
			szVal.Format(_T("0x%016I64X"), UINT64(val)); break;
		case PE_TYPE::UInt8:
			szVal.Format(_T("0x%02X" ), UINT8(val)); break;
		case PE_TYPE::Int32:
			szVal.Format(_T("0x%08X" ), INT32(val)); break;
		case PE_TYPE::Int16:
			szVal.Format(_T("0x%04X" ), INT16(val)); break;
		case PE_TYPE::Int64:
			szVal.Format(_T("0x%016I64X"), INT64(val)); break;
		case PE_TYPE::Int8:
			szVal.Format(_T("0x%02X" ), INT8(val)); break;
	}
	return szVal;
}


CString PE_SCHEMA::GetString(PBYTE pBin, PE_TYPE dt, int nLen)
{
	CString sz;
	if (dt == PE_TYPE::UInt8 || dt == PE_TYPE::Int8)
	{
		USES_CONVERSION;
		if (nLen == 0)
			sz = A2T((PSTR)pBin);
		else if (nLen > 0)
		{
			PSTR psz = new char[nLen + 1];
			memcpy(psz, pBin, nLen);
			psz[nLen] = 0;
			sz = A2T(psz);
			delete psz;
		}
	}
	else
	{
		if (nLen == 0)
			sz = PWSTR(pBin);
		else if (nLen > 0)
		{
			PWSTR psz = new WCHAR[nLen + 1];
			memcpy(psz, pBin, nLen * sizeof(WCHAR));
			psz[nLen] = 0;
			sz = psz;
			delete psz;
		}
	}
	return sz;
}

CString PE_SCHEMA::GetEnumFlags(UINT64 uv, PPE_ENUM pe)
{
	if (uv == 0)
		return pe->Find(0);

	CString sz;
	int nLoopCnt = PE_MEMBER::C_Sizes[pe->Type] * 8;
	for (int i = 0; i < nLoopCnt; i++)
	{
		UINT64 e = (1 << i);
		if ((e & uv) == 0)
			continue;
		CString esz = pe->Find(e);
		if (esz.IsEmpty())
			continue;
		if (!sz.IsEmpty())
			sz.Append(L"|");
		sz.Append(esz);
	}
	return sz;
}

CString PE_SCHEMA::Int2TimeStr(DWORD dwVal)
{
	SYSTEMTIME st; FILETIME ft; ULARGE_INTEGER li;
	::memset(&st, 0x00, sizeof(st));
	st.wYear = 1970, st.wMonth = 1, st.wDay = 1, st.wHour = 9;
	::SystemTimeToFileTime(&st, &ft);
	li.HighPart = ft.dwHighDateTime;
	li.LowPart = ft.dwLowDateTime;
	li.QuadPart += ((LONGLONG)dwVal * 10000000LL);
	ft.dwHighDateTime = li.HighPart;
	ft.dwLowDateTime = li.LowPart;
	::FileTimeToSystemTime(&ft, &st);
	
	CString sz;
	sz.Format(_T("%04d/%02d/%02d-%02d:%02d:%02d"),
		st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	return sz;
}

CString PE_SCHEMA::Bin2GuidStr(PBYTE pBin)
{
	GUID guid = *LPGUID(pBin);
	WCHAR wszGuid[65];
	StringFromGUID2(guid, wszGuid, 64);
	return CString(wszGuid);
}
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
// 
PIMAGE_OPTIONAL_HEADER32 PE_SCHEMA::GetOptHdr32(PBYTE pImgBase)
{
	int nHdrOffset = (int)((PDWORD)pImgBase)[15];
	PIMAGE_NT_HEADERS32 pnh = (PIMAGE_NT_HEADERS32)(pImgBase + nHdrOffset);
	return &pnh->OptionalHeader;
}

PIMAGE_OPTIONAL_HEADER64 PE_SCHEMA::GetOptHdr64(PBYTE pImgBase)
{
	int nHdrOffset = (int)((PDWORD)pImgBase)[15];
	PIMAGE_NT_HEADERS64 pnh = (PIMAGE_NT_HEADERS64)(pImgBase + nHdrOffset);
	return &pnh->OptionalHeader;
}

PIMAGE_SECTION_HEADER PE_SCHEMA::FindSectHdr(PBYTE pImgBase, PSTR pszName)
{
	int nHdrOffset = (int)((PDWORD)pImgBase)[15];
	PIMAGE_NT_HEADERS pnh = (PIMAGE_NT_HEADERS)(pImgBase + nHdrOffset);
	PIMAGE_SECTION_HEADER pshs = IMAGE_FIRST_SECTION(pnh);
	for (DWORD i = 0; i < pnh->FileHeader.NumberOfSections; i++)
	{
		char szName[9] = { 0, };
		memcpy(szName, pshs[i].Name, IMAGE_SIZEOF_SHORT_NAME);
		if (strcmp(szName, pszName) == 0)
			return &pshs[i];
	}
	return NULL;
}

PIMAGE_SECTION_HEADER PE_SCHEMA::FindSectHdr(PBYTE pImgBase, DWORD dwOffset)
{
	int nHdrOffset = (int)((PDWORD)pImgBase)[15];
	PIMAGE_NT_HEADERS pnh = (PIMAGE_NT_HEADERS)(pImgBase + nHdrOffset);
	PIMAGE_SECTION_HEADER pshs = IMAGE_FIRST_SECTION(pnh);
	for (DWORD i = 0; i < pnh->FileHeader.NumberOfSections; i++)
	{
		if (dwOffset >= pshs[i].VirtualAddress &&
			dwOffset < pshs[i].VirtualAddress + pshs[i].SizeOfRawData)
			return &pshs[i];
	}
	return NULL;
}


PIMAGE_FILE_HEADER PE_SCHEMA::GetFileHdr(PBYTE pImgBase)
{
	PIMAGE_FILE_HEADER pfh = PIMAGE_FILE_HEADER
		(pImgBase + (int)((PDWORD)pImgBase)[15] + sizeof(DWORD));
	return pfh;
}

PIMAGE_DATA_DIRECTORY PE_SCHEMA::GetDataDirs(PBYTE pImgBase)
{
	PIMAGE_FILE_HEADER pfh = PIMAGE_FILE_HEADER
		(pImgBase + (int)((PDWORD)pImgBase)[15] + sizeof(DWORD));
	return PIMAGE_DATA_DIRECTORY(PBYTE(pfh) +
		sizeof(IMAGE_FILE_HEADER) + pfh->SizeOfOptionalHeader -
		IMAGE_NUMBEROF_DIRECTORY_ENTRIES * sizeof(IMAGE_DATA_DIRECTORY));
}

PIMAGE_SECTION_HEADER PE_SCHEMA::GetSectHdrs(PBYTE pImgBase)
{
	PIMAGE_FILE_HEADER pfh = PIMAGE_FILE_HEADER
		(pImgBase + (int)((PDWORD)pImgBase)[15] + sizeof(DWORD));
	return PIMAGE_SECTION_HEADER(PBYTE(pfh) +
		sizeof(IMAGE_FILE_HEADER) + pfh->SizeOfOptionalHeader);
}

bool PE_SCHEMA::IsPlatform32bit(PBYTE pImgBase)
{
	PIMAGE_NT_HEADERS pnh = (PIMAGE_NT_HEADERS)(pImgBase + (int)((PDWORD)pImgBase)[15]);
	return (pnh->FileHeader.Machine == IMAGE_FILE_MACHINE_I386);
}

CString PE_SCHEMA::GetSectionName(PIMAGE_SECTION_HEADER psh)
{
	USES_CONVERSION;
	if (psh->Name[IMAGE_SIZEOF_SHORT_NAME - 1] != 0)
	{
		char szName[9];
		memcpy(szName, psh->Name, IMAGE_SIZEOF_SHORT_NAME);
		szName[IMAGE_SIZEOF_SHORT_NAME] = 0;
		return A2CT(PSTR(szName));
	}
	return A2CT(PSTR(psh->Name));
}
////////////////////////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////////////////////////
// 
PE_SCHEMA::PE_SCHEMA()
{
}

PE_SCHEMA::~PE_SCHEMA()
{
	for (TYPE_MAP::iterator it = m_typeMap.begin(); it != m_typeMap.end(); it++)
		delete it->second;
	for (ENUM_MAP::iterator it = m_enumMap.begin(); it != m_enumMap.end(); it++)
		delete it->second;
}

PPE_ENUM PE_SCHEMA::ParseEnums(XmlElement pINode)
{
	variant_t vt = pINode->getAttribute(L"name");
	if (vt.vt != VT_BSTR)
		throw _T("Parent \"name\" not found.");
	CString szName = vt.bstrVal;

	vt = pINode->getAttribute("type");
	if (vt.vt != VT_BSTR)
		throw _T("Parent \"type\" not found.");
	PE_TYPE dt = PE_MEMBER::GetDefType(vt.bstrVal);
	if (dt == PE_TYPE::Struct)
		throw _T("Enum tag \"type\" is struct.");

	bool bBitMask = false;
	vt = pINode->getAttribute(L"flags");
	if (vt.vt == VT_BSTR)
		bBitMask = (_wcsicmp(vt.bstrVal, L"true") == 0);

	PPE_ENUM pe = new PE_ENUM(szName, dt, bBitMask);
	XmlNodeList pIMemList = pINode->selectNodes(L"Item");
	while (XmlElement pIMem = pIMemList->nextNode())
	{
		vt = pIMem->getAttribute(L"name");
		CString name = vt.bstrVal;

		__int64 value = 0;
		vt = pIMem->getAttribute("value");
		if (wcsncmp(vt.bstrVal, L"0x", 2) == 0)
		{
			PTSTR pszEndPos = NULL;
			value = _tcstol(vt.bstrVal, &pszEndPos, 16);
		}
		else
			value = _ttoi64(vt.bstrVal);
		pe->Map.insert(std::make_pair(value, new PE_ENUM::Item(name, value)));
	}
	return pe;
}

PPE_MEMBER PE_SCHEMA::GetRVAType(PCTSTR pszType)
{
	CString szRva = pszType;
	CStringArray szArr;
	int nStart = 0;
	while (true)
	{
		int nPos = szRva.Find(_T(':'), nStart);
		int nLen = 0;
		if (nPos < 0)
			nLen = szRva.GetLength() - nPos;
		else
			nLen = nPos - nStart;
		if (nLen > 0)
			szArr.Add(szRva.Mid(nStart, nLen));
		if (nPos < 0)
			break;
		nStart = nPos + 1;
	}

	if (szArr.GetCount() < 2)
		throw _T("Invalid Array Length format.");

	PPE_MEMBER pmbr = new PE_MEMBER();
	pmbr->Name = szArr.GetAt(0);

	CString szType = szArr.GetAt(1);
	int nPos = szType.Find(_T('['));
	if (nPos == 0)
		throw _T("Invalid Array Length format.");

	//vt = pIMem->getAttribute(L"enum");
	//if (vt.vt == VT_BSTR)
	//{
	//	ENUM_MAP::iterator ie = m_enumMap.find(vt.bstrVal);
	//	if (ie == m_enumMap.end())
	//		throw _T("Enum tag \"type\" format invalid.");
	//	pmbr->IsEnum = 1;
	//	pmbr->Ref.Enum = ie->second;
	//}
	if (nPos < 0)
	{
		TYPE_MAP::iterator it = m_typeMap.find(szType);
		if (it == m_typeMap.end())
		{
			PE_TYPE dt = PE_MEMBER::GetDefType(szType);
			if (dt == PE_TYPE::Struct)
				throw _T("Enum tag \"type\" is struct.");
			pmbr->Type = dt;
		}
		else
		{
			pmbr->Type = PE_TYPE::Struct;
			pmbr->Struct = it->second;
		}
	}
	else
	{
		int nPos2 = szType.Find(_T(']'), nPos + 1);
		if (nPos2 <= 0)
			throw _T("Invalid Array Length format : %s");// , vt.bstrVal);
		pmbr->IsArr = 1;
		CString szLen = szType.Mid(nPos + 1, nPos2 - nPos - 1).Trim();
		szType = szType.Left(nPos).Trim();

		PTSTR pszEndPos = NULL;
		int val = _tcstol(szLen, &pszEndPos, 10);
		if (val == LONG_MAX || val == LONG_MIN)
			//throw L"length value overflow or minus.";
			pmbr->ArrLen = -1;
		else
			pmbr->ArrLen = val;

		TYPE_MAP::iterator it = m_typeMap.find(szType);
		if (it == m_typeMap.end())
		{
			PE_TYPE dt = PE_MEMBER::GetDefType(szType);
			if (dt == PE_TYPE::Struct)
				throw _T("Default Type tag \"type\" format invalid.");
			pmbr->Type = dt;
			if (szArr.GetCount() > 2)
			{
				if (szArr.GetAt(2).CompareNoCase(L"string") == 0)
					pmbr->IsStr = 1;
			}
		}
		else
		{
			pmbr->Type = PE_TYPE::Struct;
			pmbr->Struct = it->second;
			pmbr->ShowSub = 1;
		}
	}
	return pmbr;
}

PPE_STRUCT PE_SCHEMA::ParseStructs(XmlElement pINode)
{
	variant_t vt = pINode->getAttribute(L"name");
	if (vt.vt != VT_BSTR)
		throw _T("Parent \"name\" not found.");
	CString szName = vt.bstrVal;

	PE_TYPE dtBitFld = PE_TYPE::Struct;
	short nBFldPos = 0;

	PPE_STRUCT ps = new PE_STRUCT(szName);
	XmlNodeList pIMemList = pINode->selectNodes(L"Member");
	while (XmlElement pIMem = pIMemList->nextNode())
	{
		vt = pIMem->getAttribute(L"name");
		if (vt.vt != VT_BSTR)
			throw _T("Struct.Member Parent \"name\" not found.");
		PPE_MEMBER pmbr = new PE_MEMBER(vt.bstrVal);

		vt = pIMem->getAttribute(L"type");
		if (vt.vt != VT_BSTR)
			throw _T("Struct.Member Parent \"type\" not found.");
		CString szType = vt.bstrVal;
		int nPos = szType.Find(_T('['));
		if (nPos == 0)
			throw _T("Invalid Array Length format.");// , vt.bstrVal);

		vt = pIMem->getAttribute(L"enum");
		if (vt.vt == VT_BSTR)
		{
			ENUM_MAP::iterator ie = m_enumMap.find(vt.bstrVal);
			if (ie == m_enumMap.end())
				throw _T("Enum tag \"type\" format invalid.");
			pmbr->IsEnum = 1;
			pmbr->Ref.Enum = ie->second;
		}
		if (nPos < 0)
		{
			TYPE_MAP::iterator it = m_typeMap.find(szType);
			if (it == m_typeMap.end())
			{
				int nPos2 = szType.Find(_T(':'));
				if (nPos2 == 0)
					throw _T("Invalid Bitfield Length format.");// , vt.bstrVal);
				if (nPos2 > 0)
				{
					PTSTR pszEndPos = NULL;
					int val = _tcstol(szType.Mid(nPos2 + 1), &pszEndPos, 10);
					if (val == LONG_MAX || val == LONG_MIN)
						throw L"length value overflow or minus.";
					if (val == 0)
						throw L"Bitfield length is zero.";

					pmbr->IsBits = 1;
					pmbr->Bits.Mask = (BYTE)val;
					szType = szType.Left(nPos2);
				}

				PE_TYPE dt = PE_MEMBER::GetDefType(szType);
				if (dt == PE_TYPE::Struct)
					throw _T("Enum tag \"type\" is struct.");
				pmbr->Type = dt;
				if (nPos2 > 0)
				{
					if (dtBitFld == PE_TYPE::Struct)
					{
						dtBitFld = dt;
						pmbr->Bits.Shift = (BYTE)nBFldPos;
						nBFldPos += pmbr->Bits.Mask;
					}
					else
					{
						if(dtBitFld != dt)
							throw _T("Enum tag \"type\" is struct.");
						pmbr->Bits.Shift = (BYTE)nBFldPos;
						nBFldPos += pmbr->Bits.Mask;
						if (nBFldPos > PE_MEMBER::C_Sizes[dtBitFld] * 8)
							throw _T("Enum tag \"type\" is struct.");
						if (nBFldPos == PE_MEMBER::C_Sizes[dtBitFld] * 8)
						{
							dtBitFld = PE_TYPE::Struct;
							nBFldPos = 0;
						}
					}
				}
				else
				{
					if (dtBitFld < PE_TYPE::Struct)
					{
						dtBitFld = PE_TYPE::Struct;
						nBFldPos = 0;
					}
				}

				vt = pIMem->getAttribute(L"time");
				if (vt.vt == VT_BSTR && _tcsicmp(vt.bstrVal, L"true") == 0)
					pmbr->IsTime = 1;

				vt = pIMem->getAttribute(L"info");
				if (vt.vt == VT_BSTR)
				{
					pmbr->HasInfo = 1;
					pmbr->Ref.Info = new TCHAR[wcslen(vt.bstrVal) + 1];
					wcscpy_s(pmbr->Ref.Info, wcslen(vt.bstrVal) + 1, vt.bstrVal);
				}

				vt = pIMem->getAttribute(L"rva");
				if (vt.vt == VT_BSTR)
				{
					pmbr->IsRva = 1;
					//pmbr->Ref.Rva = GetRVAType(vt.bstrVal);
				}
			}
			else
			{
				pmbr->Type = PE_TYPE::Struct;
				pmbr->Struct = it->second;
			}
		}
		else
		{
			if (dtBitFld < PE_TYPE::Struct)
			{
				dtBitFld = PE_TYPE::Struct;
				nBFldPos = 0;
			}

			int nPos2 = szType.Find(_T(']'), nPos + 1);
			if (nPos2 <= 0)
				throw _T("Invalid Array Length format : %s");// , vt.bstrVal);
			pmbr->IsArr = 1;
			CString szLen = szType.Mid(nPos + 1, nPos2 - nPos - 1).Trim();
			szType = szType.Left(nPos).Trim();

			PTSTR pszEndPos = NULL;
			int val = _tcstol(szLen, &pszEndPos, 10);
			if (val == LONG_MAX || val == LONG_MIN)
				throw L"length value overflow or minus.";
			pmbr->ArrLen = val;

			TYPE_MAP::iterator it = m_typeMap.find(szType);
			if (it == m_typeMap.end())
			{
				PE_TYPE dt = PE_MEMBER::GetDefType(szType);
				if (dt == PE_TYPE::Struct)
					throw _T("Default Type tag \"type\" format invalid.");
				pmbr->Type = dt;

				vt = pIMem->getAttribute(L"string");
				if (vt.vt == VT_BSTR)
				{
					if (PE_MEMBER::C_Sizes[dt] > 2)
						throw _T("string type format invalid.");
					pmbr->IsStr = 1;
					if (_tcsicmp(vt.bstrVal, L"parent") == 0)
						pmbr->ChgUp = 1;
				}

				vt = pIMem->getAttribute(L"rva");
				if (vt.vt == VT_BSTR)
				{
					pmbr->IsRva = 1;
					//pmbr->Ref.Rva = GetRVAType(vt.bstrVal);
				}

				vt = pIMem->getAttribute(L"guid");
				if (vt.vt == VT_BSTR && _tcsicmp(vt.bstrVal, L"true") == 0)
				{
					if (dt != PE_TYPE::UInt8 && pmbr->ArrLen != 16)
						throw _T("GUID type format invalid.");
					pmbr->IsGuid = 1;
				}

				vt = pIMem->getAttribute(L"showSub");
				if (vt.vt == VT_BSTR && _tcsicmp(vt.bstrVal, L"true") == 0)
					pmbr->ShowSub = 1;
			}
			else
			{
				pmbr->Type = PE_TYPE::Struct;
				pmbr->Struct = it->second;
				pmbr->ShowSub = 1;
			}
		}
		ps->MbrMap.insert(std::make_pair(pmbr->Name, pmbr));
		ps->Members.push_back(pmbr);
	}
	return ps;
}



PPE_STRUCT PE_SCHEMA::FindType(PCTSTR name)
{
	TYPE_MAP::iterator it = m_typeMap.find(name);
	if (it == m_typeMap.end())
		throw  _T("Parent \"name\" not found.");
	return it->second;
}

void PE_SCHEMA::Load(PCTSTR pszXmlRes)
{
	XmlDocument pIDoc(__uuidof(MSXML2::DOMDocument));
	VARIANT_BOOL bIsOK = pIDoc->loadXML(pszXmlRes);
	if (bIsOK == VARIANT_FALSE)
	{
		XmlParseError pIErr = pIDoc->GetparseError();
		throw (HRESULT)pIErr->errorCode;
	}
	XmlElement pIRoot = pIDoc->documentElement;

	variant_t vt = pIRoot->getAttribute(L"name");
	if (vt.vt != VT_BSTR)
		throw _T("Parent \"name\" not found.");
	m_szName = vt.bstrVal;

	// Make enum map
	XmlNodeList pINodeList = pIDoc->documentElement->selectNodes("EnumList/Enum");
	while (XmlElement pINode = pINodeList->nextNode())
	{
		PPE_ENUM pe = ParseEnums(pINode);
		m_enumMap.insert(std::make_pair(pe->Name, pe));
	}
	pINodeList = pIDoc->documentElement->selectNodes("StructList/Struct");
	while (XmlElement pINode = pINodeList->nextNode())
	{
		PPE_STRUCT ps = ParseStructs(pINode);
		m_typeMap.insert(std::make_pair(ps->Name, ps));
	}
}
