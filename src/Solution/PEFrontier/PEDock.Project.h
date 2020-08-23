#pragma once
#include "PEWrapCtrl.h"
#include "PEDoc.h"

typedef std::map<PBYTE, CView*> MFCVIEW_MAP;

class CPrjMgrToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*)GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};


#define PRJ_NODE_DEFAULT	0
#define PRJ_NODE_PROJECT	1
#define PRJ_NODE_MODULE		2
#define PRJ_NODE_FOLDER		3
#define PRJ_NODE_FUNCTION	4
struct PRJ_NODE
{
	int			Type;
	UINT		PopupID;
	PVOID		Dbg;
	HTREEITEM	Node;

	PRJ_NODE()
	{
		Type	= -1;
		PopupID = 0;
		Dbg		= NULL;
		Node	= NULL;
	}
	PRJ_NODE(int nType, UINT uMnuId, PVOID pdbg)
	{
		Type	= nType;
		PopupID = uMnuId;
		Dbg		= pdbg;
		Node	= NULL;
	}
	PRJ_NODE(int nType, UINT uMnuId, PVOID pdbg, HTREEITEM hNode)
		: PRJ_NODE(nType, uMnuId, pdbg)
	{
		Node = hNode;
	}
};
typedef PRJ_NODE* PPRJ_NODE;

class CPEDoc;
class CProjectMgr : public CDockablePane
{
	void AddChildBlock(PDBG_BLOCK pdb, PCWSTR pFncName, HTREEITEM hFunc);

public:
	CProjectMgr();
	virtual ~CProjectMgr();

	void UpdateModule(int nOpt, PDBG_MODULE pdm);
	void UpdateProcess(int nOpt, PDBG_PROCESS pdp);

	void InitView(CPEDoc* pDoc);
	void UninitView(CPEDoc* pDoc);
	void AppenDAsmView(PDBG_MODULE pdm, PBYTE pHaltAddr = NULL, DWORD dwThrId = 0);
	void InsertViewInst(PBYTE pAddr, CView* pView)
	{
		m_mapView.insert(std::make_pair(pAddr, pView));
	}
	CView* FindViewInst(PBYTE pAddr)
	{
		MFCVIEW_MAP::iterator it = m_mapView.find(pAddr);
		if (it == m_mapView.end())
			return NULL;
		return it->second;
	}
	void RemoveViewInst(PBYTE pAddr)
	{
		m_mapView.erase(pAddr);
	}
	int GetViewInstCount()
	{
		return (int)m_mapView.size();
	}
	void SetPEDoc(CPEDoc* pDoc) { m_pPEDoc = pDoc; }
	CPEDoc* GetPEDoc() { return m_pPEDoc; }

protected:
	CViewTree		m_tvPrjMgr;
	CImageList		m_imgView;
	CPrjMgrToolBar	m_tbMain;
	CPEDoc*			m_pPEDoc;

	HTREEITEM		m_hDlls;
	HTREEITEM		m_hExec;
	MFCVIEW_MAP		m_mapView;

	void AdjustLayout();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnTvnItemChangedTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnDeleteitemTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnProperties();
	afx_msg void OnFileOpen();
	afx_msg void OnUpdateFileOpen(CCmdUI *pCmdUI);
	afx_msg void OnViewProcVAS();
	afx_msg void OnUpdateViewProcVAS(CCmdUI *pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	DECLARE_MESSAGE_MAP()

public:

};
