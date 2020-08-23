#pragma once
#include "afxcmn.h"



class CDataView : public CDialogEx
{
	DECLARE_DYNAMIC(CDataView)

	CSize m_siMargin;

	PPE_NODE	m_pPN;
	PEAnals*	m_pPE;
	CListCtrl	m_lv;

	BOOL BuildDataList();
	int BuildPublicSymbol(IDiaSymbol* pISymbol, PCWSTR pszName, DWORD dwOffset, CString& szType, CString& szVal);
	int BuildDataSymbol(IDiaSymbol* pISymbol, DWORD dwOffset, CString& szType, CString& szVal);
	CString GetPrimitiveVal(IDiaSymbol* pISymbol, int nSize, DWORD dwOffset);

public:
	CDataView(PPE_NODE pNode, PEAnals* pPE, CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDataView();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DATA_VIEW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

};
