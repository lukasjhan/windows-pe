#pragma once
#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif
#include "resource.h"


// CPEExplorerApp:
class CPEExplorerApp : public CWinApp
{
	bool LoadSchema();

public:
	CPEExplorerApp();


// �������Դϴ�.
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// �����Դϴ�.
public:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CPEExplorerApp theApp;
