#pragma once

enum PE_TYPE : BYTE
{
	UInt8 = 0, UInt16, UInt32, UInt64,
	Int8, Int16, Int32, Int64,
	Struct, PEFormat
};

struct PE_ENUM
{
	struct Item
	{
		CString Name;
		__int64	Value;

		Item() { Value = 0LL; }
		Item(PCTSTR name, __int64 val)
		{
			Name = name, Value = val;
		}
	};
	typedef std::map<__int64, Item*> ITEM_MAP;

	CString		Name;
	PE_TYPE		Type;
	bool		Flags;
	ITEM_MAP	Map;

	PE_ENUM()
	{
		Flags = false;
	}
	PE_ENUM(PCTSTR name, PE_TYPE dt, bool isFlags)
	{
		Name = name, Type = dt, Flags = isFlags;
	}
	~PE_ENUM()
	{
		for (ITEM_MAP::iterator it = Map.begin(); it != Map.end(); it++)
			delete it->second;
	}

	CString Find(__int64 val)
	{
		ITEM_MAP::iterator it = Map.find(val);
		if (it != Map.end())
			return it->second->Name;
		return _T("");
	}
};
typedef PE_ENUM* PPE_ENUM;
typedef std::map<CString, PPE_ENUM> ENUM_MAP;

struct PE_STRUCT;
typedef PE_STRUCT* PPE_STRUCT;
struct PE_MEMBER
{
	static PCTSTR C_Names[PE_TYPE::Struct];
	static short  C_Sizes[PE_TYPE::Struct];
	static PE_TYPE GetDefType(PCTSTR pszType);
	
	CString		Name;
	PE_TYPE		Type;
	BYTE		Res		: 1;
	BYTE		HasInfo : 1;
	BYTE		IsTime	: 1;
	BYTE		IsGuid	: 1;
	BYTE		IsEnum	: 1;
	BYTE		IsBits	: 1;
	BYTE		IsRva	: 1;
	BYTE		IsArr	: 1;

	union
	{
		PPE_ENUM	Enum;
		PVOID		Rva;
		PTSTR		Info;
		DWORD_PTR	Dummy;
	} Ref;

	struct 
	{
		BYTE Mask;
		BYTE Shift;
	} Bits;
	int ArrLen  : 29;
	int IsStr   :  1;
	int ChgUp   :  1;
	int ShowSub :  1;
	PE_STRUCT* Struct;


	PE_MEMBER()
	{
		Type = PE_TYPE::UInt32;
		IsTime = IsEnum = IsRva = IsArr = HasInfo = IsBits = IsGuid = 0;
		Ref.Dummy = 0, Struct = NULL;
		Bits.Mask = Bits.Shift = 0;
		ArrLen = 0, IsStr = 0, ShowSub = 0, ChgUp = 0;
	}
	PE_MEMBER(PCTSTR pszName)
	{
		Name = pszName;
		Type = PE_TYPE::UInt32;
		IsTime = IsEnum = IsRva = IsArr = HasInfo = IsBits = IsGuid = 0;
		Ref.Dummy = 0, Struct = NULL;
		Bits.Mask = Bits.Shift = 0;
		ArrLen = 0, IsStr = 0, ShowSub = 0, ChgUp = 0;
	}
	PE_MEMBER(PCTSTR pszName, PE_TYPE dt)
	{
		Name = pszName;
		Type = dt;
		IsTime = IsEnum = IsRva = IsArr = HasInfo = IsBits = IsGuid = 0;
		Ref.Dummy = 0, Struct = NULL;
		Bits.Mask = Bits.Shift = 0;
		ArrLen = 0, IsStr = 0, ShowSub = 0, ChgUp = 0;
	}
	~PE_MEMBER()
	{
		if (HasInfo)
			delete[] Ref.Info;
	}

	void Copy(PE_MEMBER& pf)
	{
		Name	= pf.Name;
		Type	= pf.Type;
		IsTime	= pf.IsTime; IsEnum = pf.IsEnum;
		IsBits  = pf.IsBits; IsGuid = pf.IsGuid;
		IsRva	= pf.IsRva; IsArr = pf.IsArr; HasInfo = pf.HasInfo;

		if (pf.HasInfo)
		{
			Ref.Info = new TCHAR[wcslen(pf.Ref.Info) + 1];
			wcscpy_s(Ref.Info, wcslen(pf.Ref.Info) + 1, pf.Ref.Info);
		}
		else
			Ref		= pf.Ref;

		Bits	= pf.Bits;
		Struct	= pf.Struct;
		ArrLen	= pf.ArrLen, IsStr = pf.IsStr; 
		ShowSub	= pf.ShowSub, ChgUp = pf.ChgUp;
	}

	PE_MEMBER* Clone()
	{
		PE_MEMBER* pmbr = new PE_MEMBER();
		pmbr->Copy(*this);
		return pmbr;
	}

};
typedef PE_MEMBER* PPE_MEMBER;
typedef std::map<CString, PPE_MEMBER>	MBR_MAP;
typedef std::vector<PPE_MEMBER>			MBR_LIST;


struct PE_STRUCT
{
	CString	 Name;
	MBR_LIST Members;
	MBR_MAP  MbrMap;

	PE_STRUCT()
	{
	}
	PE_STRUCT(PCTSTR name)
	{
		Name = name;
	}
	~PE_STRUCT()
	{
		for (MBR_LIST::iterator it = Members.begin(); it != Members.end(); it++)
			delete (*it);
	}

	PPE_MEMBER Find(PCTSTR pszFld)
	{
		MBR_MAP::iterator it = MbrMap.find(pszFld);
		if (it != MbrMap.end())
			return it->second;
		return NULL;
	}
};
typedef std::map<CString, PPE_STRUCT> TYPE_MAP;

class PE_SCHEMA
{
	TYPE_MAP	m_typeMap;
	ENUM_MAP	m_enumMap;
	CString		m_szName;

	PPE_ENUM ParseEnums(XmlElement pINode);
	PPE_STRUCT ParseStructs(XmlElement pINode);
	PPE_MEMBER GetRVAType(PCTSTR pszType);

public:
	static CString Bin2Str(PBYTE pBin, PE_TYPE dt, int nLen);
	static CString Val2Str(PBYTE pBin, PE_TYPE dt);
	static CString Val2Str(ULONGLONG val, PE_TYPE dt);
	static UINT64 GetValue(PBYTE pBin, PE_TYPE dt);
	static CString GetString(PBYTE pBin, PE_TYPE dt, int nLen);
	static CString Int2TimeStr(DWORD dwVal);
	static CString Bin2GuidStr(PBYTE pBin);
	static CString GetEnumFlags(UINT64 uv, PPE_ENUM pe);

	static PIMAGE_FILE_HEADER GetFileHdr(PBYTE pImgBase);
	static PIMAGE_DATA_DIRECTORY GetDataDirs(PBYTE pImgBase);
	static PIMAGE_SECTION_HEADER GetSectHdrs(PBYTE pImgBase);
	static PIMAGE_SECTION_HEADER FindSectHdr(PBYTE pImgBase, PSTR pszName);
	static PIMAGE_SECTION_HEADER FindSectHdr(PBYTE pImgBase, DWORD dwOffset);
	static PIMAGE_OPTIONAL_HEADER32 GetOptHdr32(PBYTE pImgBase);
	static PIMAGE_OPTIONAL_HEADER64 GetOptHdr64(PBYTE pImgBase);
	static bool IsPlatform32bit(PBYTE pImgBase);
	static CString GetSectionName(PIMAGE_SECTION_HEADER psh);

public:
	PE_SCHEMA();
	~PE_SCHEMA();

	TYPE_MAP*	TypeMap()			{ return &m_typeMap; }
	ENUM_MAP*	EnumMap()			{ return &m_enumMap; }
	PCTSTR		Name()				{ return m_szName;	}
	void		Name(PCTSTR value)	{ m_szName = value;	}

public:
	void Load(PCTSTR pszSchemPath);
	PPE_STRUCT FindType(PCTSTR name);

};
