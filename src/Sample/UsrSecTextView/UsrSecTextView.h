
// UsrSecTextView.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CUsrSecTextViewApp:
// �� Ŭ������ ������ ���ؼ��� UsrSecTextView.cpp�� �����Ͻʽÿ�.
//

class CUsrSecTextViewApp : public CWinApp
{
public:
	CUsrSecTextViewApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CUsrSecTextViewApp theApp;