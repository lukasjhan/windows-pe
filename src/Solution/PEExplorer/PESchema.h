#pragma once

enum PE_TYPE : WORD
{
	UInt8 = 0, UInt16, UInt32, UInt64,
	Int8, Int16, Int32, Int64,
	Struct, PEFormat, PEVar = 0xFF
};

struct PE_ENUM
{
	struct Item
	{
		CString Name;
		UINT64	Value;

		Item() { Value = 0LL; }
		Item(PCTSTR name, UINT64 val)
		{
			Name = name, Value = val;
		}
	};
	typedef std::map<UINT64, Item*> ITEM_MAP;

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

typedef WORD PE_ATTR;
#define	PEA_IsCode	0x0800
#define	PEA_IsSize	0x0400
#define	PEA_IsCount	0x0200
#define	PEA_HasInfo	0x0100
#define	PEA_IsTime	0x0080
#define	PEA_IsGuid	0x0040
#define	PEA_IsEnum	0x0020
#define	PEA_IsBits	0x0010
#define	PEA_IsVa	0x0008
#define	PEA_IsRva	0x0004
#define	PEA_IsArr	0x0002
#define	PEA_IsStr	0x0001

struct PE_STRUCT;
typedef PE_STRUCT* PPE_STRUCT;
struct PE_MEMBER
{
	static PCTSTR C_Names[PE_TYPE::Struct];
	static short  C_Sizes[PE_TYPE::Struct];
	static PE_TYPE GetDefType(PCTSTR pszType);
	
	CString		Name;
	PE_TYPE		Type;
	union
	{
		struct
		{
			WORD ResBits : 4;
			WORD IsCode  : 1;
			WORD IsSize  : 1;
			WORD IsCount : 1;
			WORD HasInfo : 1;
			WORD IsTime  : 1;
			WORD IsGuid  : 1;
			WORD IsEnum  : 1;
			WORD IsBits  : 1;
			WORD IsVa	 : 1;
			WORD IsRva	 : 1;
			WORD IsArr   : 1;
			WORD IsStr   : 1;
		};
		WORD Attr;
	};

	union
	{
		struct 
		{
			BYTE Mask, Shift;
		}		BitFld;
		int		ArrLen;
		LONG	None;
	} More;
	union
	{
		PPE_ENUM	Enum;
		PE_MEMBER*	Rva;
		PWSTR		Info;
		PPE_STRUCT	Struct;
		DWORD_PTR	Dummy;
	} Ref;


	PE_MEMBER()
	{
		Type = PE_TYPE::UInt32;
		Attr = 0;
		Ref.Dummy = 0;
		More.None = 0;
	}
	PE_MEMBER(PCTSTR pszName)
	{
		Name = pszName;
		Type = PE_TYPE::UInt32;
		Attr = 0;
		Ref.Dummy = 0;
		More.None = 0;
	}
	PE_MEMBER(PCTSTR pszName, PE_TYPE dt)
	{
		Name = pszName;
		Type = dt;
		Attr = 0;
		Ref.Dummy = 0;
		More.None = 0;
	}
	PE_MEMBER(PE_MEMBER& pm)
	{
		Name = pm.Name;
		Type = pm.Type;
		Attr = pm.Attr;

		if (pm.HasInfo)
		{
			Ref.Info = new TCHAR[wcslen(pm.Ref.Info) + 1];
			wcscpy_s(Ref.Info, wcslen(pm.Ref.Info) + 1, pm.Ref.Info);
		}
		else
			Ref = pm.Ref;
		More.None = pm.More.None;
	}
	~PE_MEMBER()
	{
		if (HasInfo)
			delete[] Ref.Info;
	}

	DWORD GetSize();
};
typedef PE_MEMBER* PPE_MEMBER;
typedef std::vector<PPE_MEMBER> MBR_LIST;

struct PE_STRUCT
{
	CString	 Name;
	MBR_LIST Members;

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

	DWORD GetSize();
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
	static CString GetEnumFlags(UINT64 uv, PPE_ENUM pe);

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
