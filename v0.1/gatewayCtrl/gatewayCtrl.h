
// gatewayCtrl.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CgatewayCtrlApp:
// �� Ŭ������ ������ ���ؼ��� gatewayCtrl.cpp�� �����Ͻʽÿ�.
//

class CgatewayCtrlApp : public CWinApp
{
public:
	CgatewayCtrlApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CgatewayCtrlApp theApp;