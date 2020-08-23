#pragma once
#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif
#include "resource.h"       // �� ��ȣ�Դϴ�.


// CPEApp:
class CPEApp : public CWinAppEx
{
	static bool	_STATIC_FIRST;
	CFont		m_fnt;
	CString		m_szSymPath;

	bool LoadSchema();
	void LoadSymbolPath(PCTSTR pszSymPath = NULL);

public:
	CPEApp();
	CFont* GetFont() { return &m_fnt; }

	PCTSTR GetSymbolPath()
	{
		return m_szSymPath;
	}
	void SetSymbolPath(PCTSTR pszSymPath)
	{
		LoadSymbolPath(pszSymPath);
	}
	CMDIChildWndEx* AppendView(CDocument* pDoc, UINT nViewID, CRuntimeClass* pRCV);

public:
	BOOL  m_bHiColorIcons;

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);

	afx_msg void OnAppAbout();
	afx_msg void OnFileNew();
	DECLARE_MESSAGE_MAP()
};
extern CPEApp G_PE_APP;
