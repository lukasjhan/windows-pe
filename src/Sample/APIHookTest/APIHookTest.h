// APIHookTest.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error PCH���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����Ͻʽÿ�.
#endif

#include "resource.h"		// �� ��ȣ


// CAPIHookTestApp:
// �� Ŭ������ ������ ���ؼ��� APIHookTest.cpp�� �����Ͻʽÿ�.
//

class CAPIHookTestApp : public CWinApp
{
public:
	CAPIHookTestApp();

// ������
	public:
	virtual BOOL InitInstance();

// ����

	DECLARE_MESSAGE_MAP()
};

extern CAPIHookTestApp theApp;
