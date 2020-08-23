#pragma once

struct LINE_INFO
{
	bool	IsAsm;
	bool	Expanded;
	PVOID	Tag;

	LINE_INFO()
	{
		IsAsm = Expanded = false;
		Tag = NULL;
	}
	LINE_INFO(bool bIsAsm, PVOID pTag, bool bExpand = FALSE)
	{
		IsAsm	 = bIsAsm;
		Expanded = bExpand;
		Tag		 = pTag;
	}
};
typedef LINE_INFO* PLINE_INFO;
#define DASM_SELECT		0
#define DASM_BRKPTR		1
#define DASM_SETPOS		2
#define DASM_SETBRK		3
typedef int DASM_POS;


class CDAsmView : public CListView
{
	CImageList	m_imgView;

	bool		m_bShowRva;
	bool		m_bShowCode;

	PDBG_MODULE	m_pdm;
	DWORD		m_dwDbgThId;
	int			m_nBrkItem, m_nCurItem;

	int InsertItem(int nIndex, bool bIsAsm, PVOID pTag);
	CString GetMnemonic(DECODED_INS& di);
	CString GetOperand(DECODED_INS& di);
	int GetImageIndex(PLINE_INFO pci, int nItem);
	void GetDispDAsmStr(PDBG_DASM pasm, int nSubIstm, CString& sz);
	void GetDispCodesStr(PDBG_CBASE pdc, int nSubIstm, CString& sz);
	int FindItem(DWORD dwCodeRva, PLINE_INFO& pInf);
	int FindCodeItem(DWORD dwCodeRva, int base, int size, PLINE_INFO* ppInf=NULL);

protected:
	CDAsmView();
	DECLARE_DYNCREATE(CDAsmView)

	virtual void OnInitialUpdate();
	// 생성된 메시지 맵 함수
	afx_msg void OnDestroy();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLvnDeleteitem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMCustomdraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnHotTrack(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDebugRun2Cursor();
	afx_msg void OnUpdateDebugRun2Cursor(CCmdUI *pCmdUI);
	afx_msg void OnDebugGotoCurBP();
	afx_msg void OnUpdateDebugGotoBP(CCmdUI *pCmdUI);
	afx_msg void OnExplorGotoCurPos();
	afx_msg void OnUpdateExplorGotoCurPos(CCmdUI *pCmdUI);
	afx_msg void OnExplorGotoEntryPoint();
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI *pCmdUI);
	afx_msg void OnDebugSaveAsm();
	afx_msg void OnDebugSetBreak();
	afx_msg void OnUpdateDebugSetBreak(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

public:
	virtual ~CDAsmView();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CPEDoc* GetDocument() const;
	PDBG_MODULE GetModule() { return m_pdm; }
	void UpdateBreakPoint(DWORD dwBpCode, DWORD dwHaltRva, DWORD dwThreadId = 0);
	void UpdateBreakPoint(DWORD dwBpCode, PBYTE pHaltAddr, DWORD dwThreadId = 0);
	void UpdateDAsmItem(DWORD dwBrkRva);
	void ScrollDAsmLine(DWORD dwWantRva, DASM_POS eDasmPos);

	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
};

#ifndef _DEBUG
inline CPEDoc* CDAsmView::GetDocument() const
   { return reinterpret_cast<CPEDoc*>(m_pDocument); }
#endif

inline void CDAsmView::UpdateBreakPoint(DWORD dwBpCode, PBYTE pHaltAddr, DWORD dwThreadId)
{
	UpdateBreakPoint(dwBpCode, (DWORD)(pHaltAddr - m_pdm->ImageBase), dwThreadId);
}
