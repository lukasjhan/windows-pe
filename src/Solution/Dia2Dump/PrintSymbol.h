inline int myDebugBreak( int ){
    DebugBreak();
    return 0;
}
#define MAXELEMS(x)     (sizeof(x)/sizeof(x[0]))
#define SafeDRef(a, i)  ((i < MAXELEMS(a)) ? a[i] : a[myDebugBreak(i)])

#define MAX_TYPE_IN_DETAIL 5
#define MAX_RVA_LINES_BYTES_RANGE 0x100

extern PCWSTR const GSZ_BASE_TYPES[];
extern PCWSTR const GSZ_SYM_TAGS[];
extern PCWSTR const GSZ_FLOAT_PACKS[];
extern PCWSTR const GSZ_CPU_TYPES[];
extern PCWSTR const GSZ_DATA_KINDS[];
extern PCWSTR const GSZ_UDT_KINDS[];
extern PCWSTR const GSZ_ACCESS_SPECS[];
extern PCWSTR const GSZ_CALL_CONVS[];
extern PCWSTR const GSZ_LANG_SPECS[];
extern PCWSTR const GSZ_LOC_TYPES[];
extern PCWSTR const GSZ_THUNK_ORDS[];

void PrintPublicSymbol(IDiaSymbol*);
void PrintGlobalSymbol(IDiaSymbol*);
void PrintFunctionOrBlock(IDiaSymbol*, DWORD, DWORD);
void PrintSymbol(IDiaSymbol*, DWORD);
void PrintSymTag(DWORD );
void PrintName(IDiaSymbol*);
void PrintUndName(IDiaSymbol*);
void PrintThunk(IDiaSymbol*);
void PrintCompilandDetails(IDiaSymbol*);
void PrintCompilandEnv(IDiaSymbol*);
void PrintLocation(IDiaSymbol*);
void PrintConst(IDiaSymbol*);
void PrintUDT(IDiaSymbol*);
void PrintSymbolType(IDiaSymbol*);
void PrintType(IDiaSymbol*);
void PrintBound(IDiaSymbol*);
void PrintData(IDiaSymbol*, DWORD);
void PrintVariant(VARIANT);
void PrintUdtKind(IDiaSymbol*);
void PrintTypeInDetail(IDiaSymbol*, DWORD);
void PrintFunctionType(IDiaSymbol*);
void PrintSourceFile(IDiaSourceFile*);
void PrintLines(IDiaSession*, IDiaSymbol*);
void PrintLines(IDiaEnumLineNumbers*);
void PrintSource(IDiaSourceFile*);
void PrintSecContribs(IDiaSession*, IDiaSectionContrib*);
void PrintStreamData(IDiaEnumDebugStreamData*);
void PrintFrameData(IDiaFrameData*);

void PrintPropertyStorage( IDiaPropertyStorage* );
DWORD GetPlatormId(IDiaSymbol* pISymbol);

template<class T> void PrintGeneric( T t )
{
	IDiaPropertyStorage* pPropertyStorage;
	if(t->QueryInterface( __uuidof(IDiaPropertyStorage), (PVOID*)&pPropertyStorage ) == S_OK)
	{
		PrintPropertyStorage(pPropertyStorage);
		pPropertyStorage->Release();
	}
}
