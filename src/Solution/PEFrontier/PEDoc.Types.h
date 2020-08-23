#pragma once

#define PAGE_SIZE			0x1000		// 4K
#define WOW64_VA_MAX		0x7FFEFFFF
#define TF_BIT				0x100
#define OP_INT3				0xCC
#define OP_JMP_IMP			0xFF
#define OP_JMP_IN			0xE9

typedef std::map<UINT, HWND> DOCK_MAP;

#define TM_PEPLUS_MSG_BASE		100
#define TM_CLOSE_DEBUGGER		(TM_PEPLUS_MSG_BASE + 0)
#define TM_SUSPEND_DEBUGGEE 	(TM_PEPLUS_MSG_BASE + 1)
#define TM_RESUME_DEBUGGEE		(TM_PEPLUS_MSG_BASE + 2)
#define TM_READ_MEMORY			(TM_PEPLUS_MSG_BASE + 3)
#define TM_WRITE_MEMORY			(TM_PEPLUS_MSG_BASE + 4)
#define TM_WALK_PROCMEM			(TM_PEPLUS_MSG_BASE + 5)
#define TM_DISASSEMBLE			(TM_PEPLUS_MSG_BASE + 6)
#define DASM_MODULE		0
#define DASM_RANGE		1

#define TM_BREAK_EVENT			(TM_PEPLUS_MSG_BASE + 100)
#define TM_BREAK_RESUME			(TM_PEPLUS_MSG_BASE + 101)
#define TM_SINGLE_RESUME		(TM_PEPLUS_MSG_BASE + 102)

#define WM_PROJECT_EVENT		(WM_USER + 100)
#define PRJ_NOTI_OPEN	 0	
#define PRJ_NOTI_CLOSED	 1	
#define PRJ_DBG_BEGUN	 2	
#define PRJ_DBG_ENDED	 3
#define PRJ_DBG_HALT	 4	
#define PRJ_DBG_RESUME	 5


#define WM_DEBUG_MESSAGE			(WM_USER + 101)
#define DBG_NOTI_CLEAR	-1	
#define DBG_NOTI_REMOVE	 0	
#define DBG_NOTI_APPEND  1	

#define DBG_REG_INIT	0	
#define DBG_REG_UPDATE	1	
#define DBG_REG_UNINIT  2	

#define WM_DEBUG_HALT			(WM_USER + 102)
#define WM_DEBUG_EXCEPT			(WM_USER + 103)
#define DBG_BP_CREATE	1	
#define DBG_BP_SET		2	
#define DBG_BP_CLEAR	3	
#define DBG_BP_REMOVE	4	



////////////////////////////////////////////////////////////////////////////////////////////////////
//
struct MEM_IO_PRM
{
	PVOID	Buff;
	DWORD	Size;
	HRESULT	Error;
};
typedef MEM_IO_PRM* PMEM_IO_PRM;

enum PAGE_TYPE
{
	PT_NONE, PT_HEAP, PT_STACK, PT_EXE, PT_DLL
};

struct DBG_HEAP
{
	ULONG_PTR	HeapID;
	DWORD		Flags;

	DBG_HEAP()
	{
		HeapID = 0;
		Flags = 0;
	}
};
typedef DBG_HEAP* PDBG_HEAP;
typedef std::map<ULONG_PTR, PDBG_HEAP> DBG_HEAP_MAP;
typedef DBG_HEAP_MAP* PDBG_HEAP_MAP;

struct DBG_VMOBJ
{
	MEMORY_BASIC_INFORMATION	Mbi;
	PAGE_TYPE					ObjType;
	BOOL						IsNew;
	CString						Module;
	CString						Section;

	DBG_VMOBJ();
};
typedef DBG_VMOBJ* PDBG_VMOBJ;
typedef std::map<PVOID, PDBG_VMOBJ>	DBG_VMOBJ_MAP;
typedef DBG_VMOBJ_MAP* PDBG_VMOBJ_MAP;
////////////////////////////////////////////////////////////////////////////////////////////////////


struct DBG_CBASE;
struct DBG_MODULE;
struct DBG_BLOCK;
struct DBG_PROCESS;
struct DBG_BRKPNT;


struct DBG_DASM
{
	DECODED_INS Code;
	short		SecIdx;
	WORD		HasBPoint : 1;
	WORD		HasBMark  : 1;
	WORD		Reserved  : 14;
	DBG_CBASE*	Parent;

	DBG_DASM();
};
typedef DBG_DASM* PDBG_DASM;
typedef std::vector<PDBG_DASM> DASM_LIST;
typedef DASM_LIST* PDASM_LIST;


enum CODE_TYPE : BYTE
{
	CT_NONE = 0,
	CT_THUNK,
	CT_DUMMY,
	CT_BLOCK,
	CT_FUNC
};
struct DBG_CBASE
{
	DWORD		BeginRVA;
	DWORD		CodeSize;
	DBG_MODULE*	Module;
	CODE_TYPE	CodeType;
	BYTE		SectIdx;

	DBG_CBASE();
	DBG_CBASE(DWORD dwBeginRVA, DWORD dwCodeSize, BYTE btSecIdx, DBG_MODULE* pdm);
	virtual ~DBG_CBASE() {}

};
typedef DBG_CBASE* PDBG_CBASE;
typedef std::vector<PDBG_CBASE> DBG_CODES_LIST;
typedef std::map<DWORD, PDBG_CBASE> DBG_CODES_MAP;

struct DBG_THUNK : DBG_CBASE
{
	PDBG_DASM	AsmCode;
	BOOL		ThunkType;
	CString		ThunkName;

	DBG_THUNK();
	DBG_THUNK(BOOL bType, DWORD dwBeginRVA, DWORD dwCodeSize, BYTE btSecIdx, DBG_MODULE* pdm);
	virtual ~DBG_THUNK();
};
typedef DBG_THUNK* PDBG_THUNK;
typedef std::map<DWORD, PDBG_THUNK> DBG_THUNK_MAP;

struct DBG_DUMMY : DBG_CBASE
{
	PDBG_DASM*	AsmCodes;
	int			CodeCount;

	DBG_DUMMY();
	DBG_DUMMY(DWORD dwBeginRVA, DWORD dwCodeSize, BYTE btSecIdx, DBG_MODULE* pdm);
	virtual ~DBG_DUMMY();
};
typedef DBG_DUMMY* PDBG_DUMMY;

typedef std::vector<DBG_BLOCK*> BLOCK_LIST;
typedef BLOCK_LIST* PBLOCK_LIST;
struct DBG_BLOCK : DBG_DUMMY
{
	PBLOCK_LIST		Childs;
	DBG_BLOCK*		Parent;

	DBG_BLOCK();
	DBG_BLOCK(DWORD dwCodeRva, DWORD dwCodeSize, BYTE btSecIdx, DBG_MODULE* pdm);
	virtual ~DBG_BLOCK();
};
typedef DBG_BLOCK* PDBG_BLOCK;
typedef std::map<DWORD, PDBG_BLOCK> DBG_FUNC_MAP;

struct DBG_FUNC : DBG_BLOCK
{
	CString		FuncName;

	DBG_FUNC();
	DBG_FUNC(DWORD dwCodeRva, DWORD dwCodeSize, BYTE btSecIdx, DBG_MODULE* pdm);
};
typedef DBG_FUNC* PDBG_FUNC;


struct DBG_SECTION
{
	PIMAGE_SECTION_HEADER	SecHdr;
	PBYTE					SecData;
	CString					SecName;

	DBG_SECTION()
	{
		SecHdr = NULL;
		SecData = NULL;
	}
};
typedef DBG_SECTION* PDBG_SECTION;
typedef std::map<short, PDBG_SECTION> DBG_SECTION_MAP;


struct DBG_CODESET
{
	PIMAGE_SECTION_HEADER	CodeHdr;
	PBYTE					CodeBase;
	int						DAsmCnt;

	DBG_CODESET()
	{
		CodeHdr = NULL, CodeBase = NULL;
		DAsmCnt = 0;
	}
	~DBG_CODESET()
	{
		if (CodeBase != NULL)
			VirtualFree(CodeBase, 0, MEM_RELEASE);
	}
};
typedef DBG_CODESET* PDBG_CODESET;

struct DBG_MODULE
{
	HANDLE			ImageFile;
	HANDLE			ImageMap;
	PBYTE			ImageBase;
	DWORD			ImageSize;
	DWORD			EntryPoint;
	DWORD			IsExe	: 1;
	DWORD			Unicode : 1;
	DWORD			FromPE	: 1;
	DWORD			Is32Bit : 1;
	DWORD			CSecCnt : 5;

	PBYTE			Header;
	PDBG_CODESET	CodeSecs;
#ifdef _WIN64
	PBYTE			ExptData;
#endif
	PDBG_CBASE*		CBaseList;
	int				CBaseCnt;
	DBG_THUNK_MAP	ThunkMap;
	DBG_FUNC_MAP	FuncMap;

	PEPdb*			DiaRef;
	DBG_PROCESS*	Process;
	CString			ModuleName;
	CString			ModulePath;

	DBG_MODULE();
	DBG_MODULE(DBG_PROCESS* pdp);
	~DBG_MODULE();
};
typedef DBG_MODULE* PDBG_MODULE;
typedef std::map<PBYTE, PDBG_MODULE> DBG_MODULE_MAP;
typedef DBG_MODULE_MAP* PDBG_MODULE_MAP;


struct DBG_CALLSITE
{
	PBYTE	ExecAddr;
	PBYTE	EstFrame;
#ifdef _X86_
	CONTEXT	Context;
#else
	union
	{
		WOW64_CONTEXT	CtxWow;
		CONTEXT			Context;
	};
#endif
	CString	FuncName;

	DBG_CALLSITE();
	DBG_CALLSITE(PBYTE pFA, PBYTE pEF, PCONTEXT pCtx, PCWSTR pszName);
#ifndef _X86_
	DBG_CALLSITE(PBYTE pFA, PBYTE pEF, PWOW64_CONTEXT pCtx, PCWSTR pszName);
#endif

};
typedef DBG_CALLSITE* PDBG_CALLSITE;
typedef std::vector<PDBG_CALLSITE> DBG_CALL_STACK;
typedef DBG_CALL_STACK* PDBG_CALL_STACK;

struct DBG_THREAD
{
	DWORD			ThreadId;
	HANDLE			Thread;
	PBYTE			StartAddr;
	short 			Priority;
	WORD			IsMain : 1;
	WORD			Active : 1;
	WORD			InTrap : 1;
	WORD			Reserved : 13;
	DWORD			ExitCode;
	DBG_PROCESS*	Process;
	DBG_BRKPNT*		PrevBP;

	PBYTE			StackBase;
	PBYTE			StackPtr;
	PBYTE			InstPtr;
	DBG_CALL_STACK	CallStack;
	CString			ThreadName;

	DBG_THREAD();
	DBG_THREAD(DBG_PROCESS* pdp);
	~DBG_THREAD();
};
typedef DBG_THREAD* PDBG_THREAD;
typedef std::map<DWORD, PDBG_THREAD> DBG_THREAD_MAP;
typedef DBG_THREAD_MAP* PDBG_THREAD_MAP;

struct DBG_PROCESS : DBG_MODULE
{
	DWORD			ProcessId;
	HANDLE			Process;
	PDBG_THREAD		MainThread;
	DWORD			Priority;
	BOOL			Active;
	DWORD			ExitCode;

	DBG_PROCESS();
};
typedef DBG_PROCESS* PDBG_PROCESS;



struct DBG_BRKPNT
{
	DWORD		HaltRVA;
	BYTE		OrgCode;
	BYTE		Active : 1;
	BYTE		IsTemp : 1;
	BYTE		Loaded : 1;
	BYTE		Resvrd : 5;
	union 
	{
		PDBG_MODULE	Module;
		PWSTR		ModPath;
	};
	CString		HaltIf;
	CString		Label;

	DBG_BRKPNT();
	DBG_BRKPNT(DWORD dwHaltRva, PCWSTR pModPath, int nPathLen, PCWSTR pszLabel = NULL);
	DBG_BRKPNT(DWORD dwHaltRva, PDBG_MODULE pdm, bool bIsTemp = false);
	DBG_BRKPNT(PBYTE pHaltAddr, PDBG_MODULE pdm, bool bIsTemp = false);
	~DBG_BRKPNT();

	PBYTE HaltAddr() { return Module->ImageBase + HaltRVA; }

};
typedef DBG_BRKPNT* PDBG_BRKPNT;
typedef std::map<PBYTE, PDBG_BRKPNT> DBG_BREAK_MAP;
typedef DBG_BREAK_MAP* PDBG_BREAK_MAP;
typedef std::map<DWORD, PDBG_BRKPNT> DBG_MODBRK_MAP;
typedef DBG_MODBRK_MAP* PDBG_MODBRK_MAP;
typedef std::map<CString, PDBG_MODBRK_MAP> DBG_NLBRK_MAP;
typedef DBG_NLBRK_MAP* PDBG_NLBRK_MAP;


struct DBG_BKMARK
{
	DWORD	MarkRVA;
	bool	Loaded;
	union
	{
		PDBG_MODULE	Module;
		PWSTR		ModPath;
	};
	CString	Label;
	CString	Desc;

	DBG_BKMARK();
	DBG_BKMARK(DWORD dwMarkRva, PCWSTR pModPath, int nPathLen, PCWSTR pszLabel = NULL);
	DBG_BKMARK(DWORD dwMarkRva, PDBG_MODULE pdm);
	~DBG_BKMARK();

	PBYTE MarkAddr() { return Module->ImageBase + MarkRVA; }

};
typedef DBG_BKMARK* PDBG_BKMARK;
typedef std::map<PBYTE, PDBG_BKMARK> DBG_BKMARK_MAP;
typedef DBG_BKMARK_MAP* PDBG_BKMARK_MAP;
typedef std::map<DWORD, PDBG_BKMARK> DBG_MODBM_MAP;
typedef DBG_MODBM_MAP* PDBG_MODBM_MAP;
typedef std::map<CString, PDBG_MODBM_MAP> DBG_NLBM_MAP;
typedef DBG_NLBM_MAP* PDBG_NLBM_MAP;


struct DASM_VIEW_PRM
{
	PDBG_MODULE Module;
	DWORD		StartRva;
	DWORD		ThreadId;

	DASM_VIEW_PRM(PDBG_MODULE pdm, DWORD dwStartRVA, DWORD dwThreadId)
	{
		Module	 = pdm;
		StartRva = dwStartRVA;
		ThreadId = dwThreadId;
	}
};
typedef DASM_VIEW_PRM* PDASM_VIEW_PRM;
