#pragma once

class CPdbDoc;
class CPdbListView : public CListView
{
	CString SplitFilePath(PCWSTR pszName, CString* pszPath = NULL);

	void IterateTableSymbols(IDiaTable* pITable);
	void IterateTableSectionContribs(IDiaTable* pITable);
	void IterateTableSourceFiles(IDiaTable* pITable);
	void IterateTableLineNumbers(IDiaTable* pITable);
	void IterateTableSegments(IDiaTable* pITable);
	void IterateTableFrameData(IDiaTable* pITable);
	
	typedef void(CPdbListView::*PFN_ITERTABLE)(IDiaTable*);
	PFN_ITERTABLE	m_pfnItrTbls[6];
	static PCWSTR*	PSZ_COLUMS[6];
	static INT		PCNT_COLUMS[6];

	typedef void(CPdbListView::*PFN_DIAPARSE)(IDiaSession*, IDiaSymbol*);
	typedef std::map<DIA_SYMTAGS, PFN_DIAPARSE> DIA_MAP;
	DIA_MAP m_dias;

	typedef void(CPdbListView::*PFN_TYPE_PARSE)(IDiaSymbol*, int);
	typedef std::map<DIA_SYMTAGS, PFN_TYPE_PARSE> PFN_TYPE_MAP;
	PFN_TYPE_MAP m_pfnTypes;

protected: // serialization에서만 만들어집니다.
	CPdbListView();
	DECLARE_DYNCREATE(CPdbListView)

	void ParseSymbolList(IDiaSymbol* pISymbol, int nItemIdx);
	void ParseTypedDefList(IDiaSymbol*, int);
	void ParseUdtList(IDiaSymbol*, int);
	void ParseEnumList(IDiaSymbol*, int);
	void ParseFunctionList(IDiaSymbol*, int);
	void ParseDataList(IDiaSymbol*, int);
	void ParseThunkList(IDiaSymbol*, int);
	void ParseCompliandList(IDiaSymbol*, int);
	void ParsePublicSymbolList(IDiaSymbol*, int);

	void ParseModules(IDiaSession*, IDiaSymbol*);
	void ParsePublics(IDiaSession*, IDiaSymbol*);
	void ParseSymbols(IDiaSession*, IDiaSymbol*);
	void ParseGlobals(IDiaSession*, IDiaSymbol*);
	void ParselLines(IDiaSession*, IDiaSymbol*);
	void ParseSecContribs(IDiaSession*, IDiaSymbol*);
	void ParseDebugStreams(IDiaSession*, IDiaSymbol*);


// 특성입니다.
public:
	CPdbDoc* GetDocument() const;

// 작업입니다.
public:
	void CleareListView(bool bDelCols = true);
	void UpdateDIA(PDIA_NODE pdn);
	bool UpdateTableList(PDIA_NODE pdn);
	bool UpdateSymbolGroup(PDIA_NODE pdn);
	bool UpdateSymbolList(PDIA_NODE pdn);

// 재정의입니다.
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // 생성 후 처음 호출되었습니다.

// 구현입니다.
public:
	virtual ~CPdbListView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 생성된 메시지 맵 함수
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);
};

#ifndef _DEBUG  // PdbDetail.cpp의 디버그 버전
inline CPdbDoc* CPdbListView::GetDocument() const
   { return reinterpret_cast<CPdbDoc*>(m_pDocument); }
#endif

