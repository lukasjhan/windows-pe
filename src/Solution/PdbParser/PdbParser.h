#pragma once
#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif
#include "resource.h"       // �� ��ȣ�Դϴ�.

class CPdbApp : public CWinApp
{
	CFont m_fnt;

public:
	CPdbApp();
	CFont* GetFont() { return &m_fnt; }

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};
extern CPdbApp theApp;
