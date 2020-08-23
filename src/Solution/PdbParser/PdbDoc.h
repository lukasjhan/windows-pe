#pragma once
#include <map>

typedef enum SymTagEnum DIA_SYMTAGS;

#define NODE_GRPTBL	0
#define NODE_TABLE	1
#define NODE_GRPSYM	2
#define NODE_SYMBOL	3
typedef WORD NODE_TYPE;

#define DIA_CAT_SYMBOL		0
#define DIA_CAT_SECTION		1
#define DIA_CAT_SRCFILE		2
#define DIA_CAT_LINENUM		3
#define DIA_CAT_SEGMENT		4
#define DIA_CAT_FRMDATA		5
#define DIA_CAT_JNJSRC		6
#define DIA_CAT_INPUTASM	7

#define DIA_CAT_EXE			9
typedef short DIA_CAT;

struct DIA_NODE
{
	WORD		IsScaned : 6;
	WORD		NodeType : 6;
	WORD		Category : 9;
	IUnknown*	IDiaPtr;

	DIA_NODE()
	{
		IsScaned = false;
		NodeType = NODE_GRPTBL;
		Category = 0;
		IDiaPtr  = NULL;
	}
	DIA_NODE(NODE_TYPE nt, WORD wCat)
	{
		IsScaned = false;
		NodeType = nt;
		Category = wCat;
		IDiaPtr  = NULL;
	}
	DIA_NODE(NODE_TYPE nt, WORD wCat, IUnknown* pIPtr)
	{
		IsScaned = false;
		NodeType = nt;
		Category = wCat;
		IDiaPtr  = pIPtr;
		IDiaPtr->AddRef();
	}
	~DIA_NODE()
	{
		if (IDiaPtr != NULL)
			IDiaPtr->Release();
	}
};
typedef DIA_NODE* PDIA_NODE;

#define GSZ_CRLF L"\xd\xa"

class CPdbDoc : public CDocument
{
protected: // serialization에서만 만들어집니다.
	CPdbDoc();
	DECLARE_DYNCREATE(CPdbDoc)

	//void BuildSymbolMap(DIA_SYMTAGS eTag, IDiaSymbol* pIBase, SYM_ID_MAP& symMap);

// 특성입니다.
public:
	//static PCWSTR GSZ_LOC_TYPES[];
	//static PCWSTR GSZ_BASE_TYPES[];
	//static PCWSTR GSZ_SYM_TAGS[];
	//static PCWSTR GSZ_DATA_KINDS[];
	//static PCWSTR GSZ_UDT_KINDS[];
	//static PCWSTR GSZ_ACCESS_SPECS[];
	//static PCWSTR GSZ_CALL_CONVS[];
	//static PCWSTR GSZ_LANG_SPECS[];
	//static PCWSTR GSZ_CPU_TYPES[];
	//static PCWSTR GSZ_FLOAT_PACKS[];
	//static PCWSTR GSZ_THUNK_ORDS[];

	static DWORD			MACHINE_TYPE;
	CComPtr<IDiaSession>	DIA_SESSION;
	CComPtr<IDiaDataSource> DIA_SOURCE;
	CComPtr<IDiaSymbol>		DIA_PESCOPE;

// 작업입니다.
public:
	virtual ~CPdbDoc();

	//static CString ToStr_Variant(LPVARIANT pv);

	//static PCWSTR  ToStr_C7REG(USHORT uReg);
	//static CString ToStr_Name(IDiaSymbol* pISymbol);
	//static CString ToStr_Bound(IDiaSymbol* pISymbol);
	//static CString ToStr_Location(IDiaSymbol* pISymbol);
	//static CString ToStr_ArrayType(IDiaSymbol* pISymbol);
	//static CString ToStr_FunctionType(IDiaSymbol* pISymbol, PCWSTR pszName);
	//static CString ToStr_PointerType(IDiaSymbol* pISymbol);
	//static CString ToStr_BaseType(IDiaSymbol* pISymbol);
	//static CString ToStr_CustomType(IDiaSymbol* pISymbol);
	//static CString ToStr_Type(IDiaSymbol* pISymbol);
	//static CString ToStr_UndName(IDiaSymbol* pISymbol);
	//static CString ToStr_VTableSahpe(IDiaSymbol* pISymbol);

	//static CString ToStr_CompilandEnv(IDiaSymbol* pISymbol);
	//static CString ToStr_Thunk(IDiaSymbol* pISymbol);
	//static CString ToStr_CallSite(IDiaSymbol* pISymbol);
	//static CString ToStr_Symbol(IDiaSymbol* pISymbol, DWORD dwIndent);

	//static CString ToStr_Data(IDiaSymbol* pISymbol);
	//static CString ToStr_Function(IDiaSymbol* pISymbol);
	//static CString ToStr_CompilandDetails(IDiaSymbol* pISymbol);

// 재정의입니다.
public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(PCTSTR pszPathName);
	virtual BOOL OnSaveDocument(PCTSTR pszPathName);

#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// 검색 처리기에 대한 검색 콘텐츠를 설정하는 도우미 함수
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
public:
};
