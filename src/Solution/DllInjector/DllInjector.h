
// DllInjector.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CDllInjectorApp:
// �� Ŭ������ ������ ���ؼ��� DllInjector.cpp�� �����Ͻʽÿ�.
//

class CDllInjectorApp : public CWinApp
{
public:
	CDllInjectorApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CDllInjectorApp theApp;