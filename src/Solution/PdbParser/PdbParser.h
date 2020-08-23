#pragma once
#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif
#include "resource.h"       // 주 기호입니다.

class CPdbApp : public CWinApp
{
	CFont m_fnt;

public:
	CPdbApp();
	CFont* GetFont() { return &m_fnt; }

// 재정의입니다.
public:
	virtual BOOL InitInstance();

// 구현입니다.
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};
extern CPdbApp theApp;
