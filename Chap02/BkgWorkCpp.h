
// BkgWorkCpp.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifdef __AFXWIN_H__
#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"

// CBkgWorkCppApp:
// �� Ŭ������ ������ ���ؼ��� BkgWorkCpp.cpp�� �����Ͻʽÿ�.
//
class CBkgWorkCppApp : public CWinApp
{
public:
	CBkgWorkCppApp();
//������ �Դϴ�.
public :
	virtual BOOL InitInstance();

//�����Դϴ�.
	DECLARE_MESSAGE_MAP()
};


extern CBkgWorkCppApp theApp;

