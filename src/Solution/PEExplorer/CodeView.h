#pragma once
#include "afxcmn.h"

typedef std::map<CString, CString> LOCV_MAP;

class CCodeView : public CDialogEx
{
	DECLARE_DYNAMIC(CCodeView)

	//CFont	 m_fnt;
	CSize	 m_siMargin;

	PPE_NODE	m_pPN;
	PEAnals*	m_pPE;
	CListCtrl	m_lv;

	void GetLocalVars(IDiaSymbol*, LOCV_MAP&);
	void ScanLocalVars(IDiaSymbol*, LOCV_MAP&);
	void BuildCodeList(PIMAGE_SECTION_HEADER psh, IDiaSymbol* pIFunc, LOCV_MAP& mapLocs);

public:
	CCodeView(PPE_NODE pNode, PEAnals* pPE, CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CCodeView();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_CODE_VIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	virtual BOOL OnInitDialog();

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnLvnGetdispinfoLvData(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnKeydownLvData(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()

public:

};
