
// BkgWorkCpp.h : PROJECT_NAME 응용 프로그램에 대한 주 헤더 파일입니다.
//

#pragma once

#ifdef __AFXWIN_H__
#error "PCH에 대해 이 파일을 포함하기 전에 'stdafx.h'를 포함합니다."
#endif

#include "resource.h"

// CBkgWorkCppApp:
// 이 클래스의 구현에 대해서는 BkgWorkCpp.cpp을 참조하십시오.
//
class CBkgWorkCppApp : public CWinApp
{
public:
	CBkgWorkCppApp();
//재정의 입니다.
public :
	virtual BOOL InitInstance();

//구현입니다.
	DECLARE_MESSAGE_MAP()
};


extern CBkgWorkCppApp theApp;

