#pragma once
#include "PESchema.h"
#include "TreeListView\TreeListCtrl.h"

#define IMG_IDX_PE		0
#define IMG_IDX_NODE	1
#define IMG_IDX_LEAF	2
#define IMG_IDX_LFARR	3
#define IMG_IDX_ARRAY	4
#define IMG_IDX_STRING	5
#define IMG_IDX_ENUM	6
#define IMG_IDX_RVA		7
#define IMG_IDX_RVADIR	8
#define IMG_IDX_XBOX	9
#define IMG_IDX_SECT	10
#define IMG_IDX_DIRS	11
#define IMG_IDX_LFNO	12
#define IMG_IDX_TH		13
#define IMG_IDX_EH		14
#define IMG_IDX_ETH		15
#define IMG_IDX_DIRC	16
#define IMG_IDX_CS		17
#define IMG_IDX_CS2		18
#define IMG_IDX_FUNC	19
#define IMG_IDX_BLOCK	20
#define IMG_IDX_IMPORT	21
#define IMG_IDX_THUNK	22
#define IMG_IDX_VA		23

#define COL_IDX_NAME	0
#define COL_IDX_TYPE	1
#define COL_IDX_OFFSET	2
#define COL_IDX_SIZE	3
#define COL_IDX_VALUE	4
#define COL_IDX_INFO	5
#define COL_IDX_MAX		6

#define VAR_CAT_GLOBAL	0
#define VAR_CAT_CONST	1
#define VAR_CAT_STATIC	2

#define PE_KIND_CODE	0x01
#define PE_KIND_DATA	0x02

#define NK_BASE		IMAGE_NUMBEROF_DIRECTORY_ENTRIES + 1
#define NK_ROOT		(NK_BASE + 0)
#define NK_HDR		(NK_BASE + 1)
#define NK_SECT		(NK_BASE + 2)
#define NK_DDIR		(NK_BASE + 3)
#define NK_FUNC		(NK_BASE + 4)
#define NK_LEAF		0xFF

class PEAnals;
struct PE_NODE : PE_MEMBER
{
	DWORD		Offset;
	DWORD		Size;
	SHORT		Index;

	BYTE		Kind;
	BYTE		SubT;
	LPARAM		Tag;
	HTREEITEM	Node;

	PE_NODE(PEAnals* ppe);
	PE_NODE(short nSectIdx, DWORD dwOffset, PCTSTR pszName, PCTSTR pszType, int nArrLen = 0);
	PE_NODE(short nSectIdx, DWORD dwOffset, PCTSTR pszName, PE_TYPE dt, int nArrLen = 0);
	PE_NODE(short nSectIdx, DWORD dwOffset, PPE_MEMBER pm);
	~PE_NODE()
	{
		if (Kind == NK_FUNC && Tag != 0)
		{
			CComPtr<IDiaSymbol>* pISym = (CComPtr<IDiaSymbol>*)Tag;
			delete pISym;
		}
	}

};
typedef PE_NODE* PPE_NODE;


struct PE_BLOCK
{
	DWORD		Rva, Size;
	short		Depth;
	short		CodeIdx, UnwdIdx, Prolog;
	PE_BLOCK*	Parent;

	PE_BLOCK()
	{
		Rva		= Size = 0;
		Depth	= 1;
		CodeIdx = UnwdIdx = Prolog = 0;
		Parent	= NULL;
	}
};
typedef PE_BLOCK* PPE_BLOCK;
typedef std::vector<PPE_BLOCK> BLOCK_LIST;
typedef BLOCK_LIST* PBLOCK_LIST;

struct PE_FUNC : PE_BLOCK
{
	PWSTR				Name;
	DWORD				CodeLen;
	PBLOCK_LIST			Blocks;
	CComPtr<IDiaSymbol>	DiaPtr;

	PE_FUNC() : PE_BLOCK()
	{
		Name	= NULL;
		CodeLen = 0;
		Blocks	= NULL;
		DiaPtr	= 0;
		Depth	= 0;
	}
	~PE_FUNC()
	{
		if (Name != NULL)
			delete[] Name;
		if (DiaPtr != NULL)
			DiaPtr = 0;
		if (Blocks != NULL)
			delete Blocks;
	}

	void SetName(PCWSTR pszName)
	{
		int nLen = (int)wcslen(pszName);
		Name = new __wchar_t[nLen + 1];
		wcscpy_s(Name, nLen + 1, pszName);
	}
};
typedef PE_FUNC* PPE_FUNC;
typedef std::map<DWORD, PPE_BLOCK> RVA_FNC_MAP;

struct PE_THUNK
{
	PWSTR Name;
	DWORD Rva, Size;
	CComPtr<IDiaSymbol>	DiaPtr;
	int		CodeIdx, CodeCnt;

	PE_THUNK()
	{
		Name = NULL;
		DiaPtr = NULL;
		Rva = Size = 0;
		CodeIdx = CodeCnt = 0;
	}
	PE_THUNK(PCWSTR name, DWORD rva, DWORD size)
	{
		int nLen = (int)wcslen(name);
		Name = new __wchar_t[nLen + 1];
		wcscpy_s(Name, nLen + 1, name);
		Rva = rva, Size = size;
		DiaPtr = NULL;
		CodeIdx = CodeCnt = 0;
	}
	PE_THUNK(const PE_THUNK& pf)
	{
		Name = NULL;
		if (pf.Name != NULL)
		{
			int nLen = (int)wcslen(pf.Name);
			Name = new __wchar_t[nLen + 1];
			wcscpy_s(Name, nLen + 1, pf.Name);
		}
		Rva = pf.Rva, Size = pf.Size;
		CodeIdx = pf.CodeIdx, CodeCnt = pf.CodeCnt;
		DiaPtr = pf.DiaPtr;
	}
	~PE_THUNK()
	{
		if (Name != NULL)
			delete[] Name;
		if (DiaPtr != NULL)
			DiaPtr = 0;
	}

	void operator=(PCWSTR name)
	{
		if (Name != NULL)
			delete[] Name;

		int nLen = (int)wcslen(name);
		Name = new __wchar_t[nLen + 1];
		wcscpy_s(Name, nLen + 1, name);
	}
};
typedef PE_THUNK* PPE_THUNK;
typedef std::map<DWORD, PPE_THUNK> RVA_THK_MAP;


typedef std::set<HTREEITEM> SORT_SET;
typedef std::vector<PPE_NODE> SECT_LIST;
typedef std::map<DWORD, PPE_NODE> SYMN_MAP;
typedef std::set<DWORD> SYMN_SET;

class PEAnals
{
	HANDLE			m_hImgFile;
	HANDLE			m_hImgMap;
	PBYTE			m_pImgView;
	CString			m_szPEPath;

	bool					m_bIs32Bit;
	PIMAGE_DATA_DIRECTORY	m_pdds;
	PIMAGE_SECTION_HEADER	m_pshs;

	SECT_LIST				m_pnSects;
	PPE_NODE				m_pnRoot;
	CTreeListCtrl*			m_pView;
	SYMN_SET				m_ilts;

	typedef bool (PEAnals::*PFN_PARSE_DATADIR)(PPE_NODE, PIMAGE_DATA_DIRECTORY);
	PFN_PARSE_DATADIR m_pfnddrs[IMAGE_NUMBEROF_DIRECTORY_ENTRIES - 1];
	bool ParseDirEntryExport(PPE_NODE, PIMAGE_DATA_DIRECTORY);
	bool ParseDirEntryImport(PPE_NODE, PIMAGE_DATA_DIRECTORY);
	bool ParseDirEntryResource(PPE_NODE, PIMAGE_DATA_DIRECTORY);
#ifdef _WIN64
	bool ParseDirEntryException(PPE_NODE, PIMAGE_DATA_DIRECTORY);
#endif
	bool ParseDirEntrySecurity(PPE_NODE, PIMAGE_DATA_DIRECTORY);
	bool ParseDirEntryBaseReloc(PPE_NODE, PIMAGE_DATA_DIRECTORY);
	bool ParseDirEntryDebug(PPE_NODE, PIMAGE_DATA_DIRECTORY);
	bool ParseDirEntryTLS(PPE_NODE, PIMAGE_DATA_DIRECTORY);
	bool ParseDirEntryLoadConfig(PPE_NODE, PIMAGE_DATA_DIRECTORY);
	bool ParseDirEntryBoundImport(PPE_NODE, PIMAGE_DATA_DIRECTORY);
	bool ParseDirEntryIAT(PPE_NODE, PIMAGE_DATA_DIRECTORY);
	bool ParseDirEntryDelayImport(PPE_NODE, PIMAGE_DATA_DIRECTORY);
	bool ParseDirEntryComDescript(PPE_NODE, PIMAGE_DATA_DIRECTORY);

	int ParsePEHeaders();
	void ParseTextSection(int nNumOfSec);
	int BuildFunctionFromPDB();
	int BuildFunctionDirect(PIMAGE_SECTION_HEADER, PPE_NODE);
	int BuildThunkFromPDB(bool bThunkILT, PCWSTR pszDll = NULL);
	int BuildThunkDirect(PIMAGE_SECTION_HEADER psh, PPE_NODE pnUp);
	bool SectionHasData(int nSectIdx);

	int GetImageIndex(PPE_MEMBER pm);
	CString GetTypeName(PPE_MEMBER pm);

	void InsertNode(PPE_NODE pn, HTREEITEM hParent, int nImgIdx = -1);
	DWORD AppendStructMembers(PPE_NODE pn, DWORD dwVarSize = 0);
	PPE_NODE InsertPERootNode();
	PPE_NODE InsertStructNode(HTREEITEM hParent, short nSectIdx, DWORD dwOffset,
		PCTSTR pszName, PCTSTR pszType, int nArrLen = 0, int nImgIdx = -1);
	PPE_NODE InsertFieldNode(HTREEITEM hParent, short nSectIdx, DWORD dwOffset,
		PCTSTR pszName, PE_TYPE dt, int nArrLen = 0, int nImgIdx = -1);
	PPE_NODE InsertRVANode(HTREEITEM hParent, short nSectIdx, DWORD dwOffset,
		PCTSTR pszName, PE_TYPE dt = UInt32);
	PPE_NODE InsertStrNode(HTREEITEM hParent, short nSectIdx, DWORD dwOffset,
		PCTSTR pszName, int nLen, PE_TYPE dt = UInt8);
	PPE_NODE InsertCodeNode(HTREEITEM hParent, short nSectIdx, DWORD dwOffset,
		PCTSTR pszName, int nLen, int nImgIdx = -1);

	PPE_NODE FindRVANode(HTREEITEM hNode, DWORD dwRVA);
	PPE_NODE UpdateRVANode(PPE_NODE pn, DWORD dwRVA, HTREEITEM hFind=NULL);

	PIMAGE_IMPORT_BY_NAME BuildHintNameNodes(DWORD dwRvaVal, PPE_NODE pnUp);
	CString GetNameOrOrd(DWORD dwIntRva, PIMAGE_SECTION_HEADER psh);
	PPE_NODE InsertImpRVARefNode(PIMAGE_SECTION_HEADER psh, int nIdx, DWORD dwOffset,
		PPE_NODE pnUp, DWORD dwNameOff = 0, bool bDelay=false);

	static PCWSTR GSZ_RES_TYPES[];
	DWORD ParseResDirEntry(DWORD, PIMAGE_RESOURCE_DIRECTORY, DWORD, PPE_NODE, int, PCTSTR);


	void UpdateNodeText(HTREEITEM hNode, PCWSTR pszTxt, int nCol, bool bAppend = false)
	{
		if (bAppend)
		{
			CString sz = m_pView->GetItemText(hNode, nCol);
			if (!sz.IsEmpty())
				sz.AppendFormat(L", %s", pszTxt);
			else
				sz = pszTxt;
			m_pView->SetItemText(hNode, sz, nCol);
		}
		else
			m_pView->SetItemText(hNode, pszTxt, nCol);
	}
	void SetNodeTextColor(HTREEITEM hNode, COLORREF clr, int nCol = -1)
	{
		if (nCol < 0)
		{
			for (int i = 0; i < COL_IDX_MAX; i++)
				m_pView->SetItemTextColor(hNode, i, clr);
		}
		else
			m_pView->SetItemTextColor(hNode, nCol, clr);
	}

	struct RTF_INFO
	{
		INT				Index;
		DWORD			Parent;
		short			UwSecIdx;
		PUNWIND_INFO	Unwind;
		PPE_NODE		RtfNode, FncNode, UnwNode;

		RTF_INFO()
		{
			Index = -1;
			Parent = 0;
			UwSecIdx = INVALID_SECT_IDX;
			Unwind = NULL;
			RtfNode = FncNode = UnwNode = NULL;
		}
		RTF_INFO(int nRtfIdx) : RTF_INFO()
		{
			Index = nRtfIdx;
		}
	};
	typedef RTF_INFO* PRTF_INFO;
	typedef std::map<DWORD, PRTF_INFO> RTF_MAP;
	void InsertRTFNode(PRTF_INFO pri, HTREEITEM hUp, short nSectIdx, PRUNTIME_FUNCTION prfs,
		PIMAGE_SECTION_HEADER psh, RTF_MAP& rtfs, bool bIsRoot);
	int ParseUnwindCodes(PUNWIND_INFO pui, short nSectIdx, DWORD dwUwiOff, HTREEITEM hUp);
	CString GetUnwindCode(PUNWIND_INFO pui, DWORD dwUwiOff, int nIdx, int& nCnt);
	void ParseExceptionData(PCWSTR pszHdlrName, PPE_NODE pnui, DWORD dwUwiOff);

	PEAnals();
public:
	PEAnals(PCTSTR pszPEFile, CTreeListCtrl* pView);
	~PEAnals();

	PCTSTR	GetPEPath()  { return m_szPEPath; }
	HANDLE	GetImgFile() { return m_hImgFile; }
	PBYTE	GetImgView() { return m_pImgView; }
	bool	Is32Bit()	 { return m_bIs32Bit; }
	PIMAGE_SECTION_HEADER GetSectHdrs()
	{
		return m_pshs;
	}
	int GetSectCount()
	{
		return (int)PEPlus::GetFileHdr(m_pImgView)->NumberOfSections;
	}

public:
	static PE_SCHEMA	SCHEMA;
	static TCHAR		SYM_PATH[MAX_PATH];

	CString GetSymbolName(IDiaSession*, DWORD dwRVA);

	CString PDB_INFO;
	PEPdb	DIA_PDB;

	RVA_FNC_MAP		RAV_FUNCS;
	RVA_THK_MAP		RAV_THNKS;

public:
	PPE_NODE ParsePEImage(SORT_SET& sorts);
	PPE_NODE FindNode(HTREEITEM hNode, PCTSTR pszName, bool bSibling = false);
	PPE_NODE FindNode(HTREEITEM hNode, DWORD dwOffset, bool bSibling = false);
	void ViewResDataEntry(PPE_NODE ppn);

};
typedef std::map<CString, PEAnals*> ANAL_MAP;
typedef std::map<DWORD, int> SORT_MAP;




inline PPE_NODE PEAnals::InsertPERootNode()
{
	PPE_NODE pn = new PE_NODE(this);
	InsertNode(pn, TVI_ROOT);
	return pn;
}
inline PPE_NODE PEAnals::InsertStructNode(HTREEITEM hParent, short nSectIdx, DWORD dwOffset,
	PCTSTR pszName, PCTSTR pszType, int nArrLen, int nImgIdx)
{
	PPE_NODE pn = new PE_NODE(nSectIdx, dwOffset, pszName, pszType, nArrLen);
	InsertNode(pn, hParent, nImgIdx);
	return pn;
}
inline PPE_NODE PEAnals::InsertFieldNode(HTREEITEM hParent, short nSectIdx, DWORD dwOffset,
	PCTSTR pszName, PE_TYPE dt, int nArrLen, int nImgIdx)
{
	ASSERT(dt < PE_TYPE::Struct);
	PPE_NODE pn = new PE_NODE(nSectIdx, dwOffset, pszName, dt, nArrLen);
	InsertNode(pn, hParent, nImgIdx);
	return pn;
}
inline PPE_NODE PEAnals::InsertRVANode(HTREEITEM hParent, short nSectIdx, DWORD dwOffset,
	PCTSTR pszName, PE_TYPE dt)
{
	ASSERT(dt < PE_TYPE::Struct);
	PPE_NODE pn = new PE_NODE(nSectIdx, dwOffset, pszName, dt);
	pn->IsRva = true;
	InsertNode(pn, hParent);
	return pn;
}
inline PPE_NODE PEAnals::InsertStrNode(HTREEITEM hParent, short nSectIdx, DWORD dwOffset,
	PCTSTR pszName, int nLen, PE_TYPE dt)
{
	ASSERT(dt < PE_TYPE::Struct);
	PPE_NODE pn = new PE_NODE(nSectIdx, dwOffset, pszName, dt, nLen);
	pn->IsStr = true;
	InsertNode(pn, hParent);
	return pn;
}
inline PPE_NODE PEAnals::InsertCodeNode(HTREEITEM hParent, short nSectIdx, DWORD dwOffset,
	PCTSTR pszName, int nLen, int nImgIdx)
{
	PPE_NODE pn = new PE_NODE(nSectIdx, dwOffset, pszName, PE_TYPE::UInt8, nLen);
	pn->IsCode = true;
	InsertNode(pn, hParent, nImgIdx);
	return pn;
}
