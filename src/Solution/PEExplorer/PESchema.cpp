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

DWORD PE_MEMBER::GetSize()
{
	DWORD dwSize = 0;
	if (Type == PE_TYPE::PEFormat)
		throw E_INVALIDARG;

	if (Type == PE_TYPE::Struct)
		dwSize = Ref.Struct->GetSize();
	else
		dwSize = C_Sizes[Type];
	if (IsArr)
	{
		if (More.ArrLen < 0)
			throw E_INVALIDARG;
		dwSize *= More.ArrLen;
	}
	return dwSize;
}

DWORD PE_STRUCT::GetSize()
{
	DWORD dwSize = 0;
	PE_TYPE dtBitFld = PE_TYPE::Struct;
	short nBFldPos = 0;

	for (MBR_LIST::iterator it = Members.begin(); it != Members.end(); it++)
	{
		PPE_MEMBER pm = *it;
		if (pm->IsBits)
		{
			nBFldPos += pm->More.BitFld.Mask;
			if (dtBitFld == PE_TYPE::Struct)
			{
				dtBitFld = pm->Type;
			}
			else
			{
				if (nBFldPos == PE_MEMBER::C_Sizes[dtBitFld] * 8)
				{
					dtBitFld = PE_TYPE::Struct;
					nBFldPos = 0;
					dwSize += pm->GetSize();
				}
			}
		}
		else
			dwSize += pm->GetSize();
	}
	return dwSize;
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
		UINT64 e = (UINT64)1 << i;
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
		throw _T("Tag \"name\" not found.");
	CString szName = vt.bstrVal;

	vt = pINode->getAttribute("type");
	if (vt.vt != VT_BSTR)
		throw _T("Tag \"type\" not found.");
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

		UINT64 value = 0;
		vt = pIMem->getAttribute("value");
		if (wcsncmp(vt.bstrVal, L"0x", 2) == 0)
		{
			PTSTR pszEndPos = NULL;
			value = _tcstoul(vt.bstrVal, &pszEndPos, 16);
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
			pmbr->Ref.Struct = it->second;
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
			pmbr->More.ArrLen = -1;
		else
			pmbr->More.ArrLen = val;

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
			pmbr->Ref.Struct = it->second;
		}
	}
	return pmbr;
}

PPE_STRUCT PE_SCHEMA::ParseStructs(XmlElement pINode)
{
	variant_t vt = pINode->getAttribute(L"name");
	if (vt.vt != VT_BSTR)
		throw _T("Tag \"name\" not found.");
	CString szName = vt.bstrVal;

	PE_TYPE dtBitFld = PE_TYPE::Struct;
	short nBFldPos = 0;

	PPE_STRUCT ps = new PE_STRUCT(szName);
	XmlNodeList pIMemList = pINode->selectNodes(L"Member");
	while (XmlElement pIMem = pIMemList->nextNode())
	{
		vt = pIMem->getAttribute(L"name");
		if (vt.vt != VT_BSTR)
			throw _T("Struct.Member Tag \"name\" not found.");
		PPE_MEMBER pmbr = new PE_MEMBER(vt.bstrVal);

		vt = pIMem->getAttribute(L"type");
		if (vt.vt != VT_BSTR)
			throw _T("Struct.Member Tag \"type\" not found.");
		CString szType = vt.bstrVal;
		int nPos = szType.Find(_T('['));
		if (nPos == 0)
			throw _T("Invalid Array Length format.");

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
					throw _T("Invalid Bitfield Length format.");
				if (nPos2 > 0)
				{
					PTSTR pszEndPos = NULL;
					int val = _tcstol(szType.Mid(nPos2 + 1), &pszEndPos, 10);
					if (val == LONG_MAX || val == LONG_MIN)
						throw L"length value overflow or minus.";
					if (val == 0)
						throw L"Bitfield length is zero.";

					pmbr->IsBits = 1;
					pmbr->More.BitFld.Mask = (BYTE)val;
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
						pmbr->More.BitFld.Shift = (BYTE)nBFldPos;
						nBFldPos += pmbr->More.BitFld.Mask;
					}
					else
					{
						if(dtBitFld != dt)
							throw _T("Enum tag \"type\" is struct.");
						pmbr->More.BitFld.Shift = (BYTE)nBFldPos;
						nBFldPos += pmbr->More.BitFld.Mask;
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
				if (vt.vt == VT_BSTR && _tcsicmp(vt.bstrVal, L"true") == 0)
					pmbr->IsRva = 1;
				vt = pIMem->getAttribute(L"va");
				if (vt.vt == VT_BSTR && _tcsicmp(vt.bstrVal, L"true") == 0)
					pmbr->IsVa = 1;
				vt = pIMem->getAttribute(L"size");
				if (vt.vt == VT_BSTR && _tcsicmp(vt.bstrVal, L"true") == 0)
					pmbr->IsSize = 1;
			}
			else
			{
				pmbr->Type = PE_TYPE::Struct;
				pmbr->Ref.Struct = it->second;
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
				throw _T("Invalid Array Length format.");
			pmbr->IsArr = 1;
			CString szLen = szType.Mid(nPos + 1, nPos2 - nPos - 1).Trim();
			szType = szType.Left(nPos).Trim();

			PTSTR pszEndPos = NULL;
			int val = _tcstol(szLen, &pszEndPos, 10);
			if (val == LONG_MAX || val == LONG_MIN)
				throw L"length value overflow or minus.";
			pmbr->More.ArrLen = val;

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
				}

				vt = pIMem->getAttribute(L"rva");
				if (vt.vt == VT_BSTR && _tcsicmp(vt.bstrVal, L"true") == 0)
					pmbr->IsRva = 1;
				vt = pIMem->getAttribute(L"va");
				if (vt.vt == VT_BSTR && _tcsicmp(vt.bstrVal, L"true") == 0)
					pmbr->IsVa = 1;

				vt = pIMem->getAttribute(L"guid");
				if (vt.vt == VT_BSTR && _tcsicmp(vt.bstrVal, L"true") == 0)
				{
					if (dt != PE_TYPE::UInt8 && pmbr->More.ArrLen != 16)
						throw _T("GUID type format invalid.");
					pmbr->IsGuid = 1;
				}
			}
			else
			{
				pmbr->Type = PE_TYPE::Struct;
				pmbr->Ref.Struct = it->second;
			}
		}
		ps->Members.push_back(pmbr);
	}
	return ps;
}

PPE_STRUCT PE_SCHEMA::FindType(PCTSTR name)
{
	TYPE_MAP::iterator it = m_typeMap.find(name);
	if (it == m_typeMap.end())
		throw  _T("Tag \"name\" not found.");
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
		throw _T("Tag \"name\" not found.");
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
