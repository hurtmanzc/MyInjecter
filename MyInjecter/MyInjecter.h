
// MyInjecter.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMyInjecterApp:
// �йش����ʵ�֣������ MyInjecter.cpp
//

class CMyInjecterApp : public CWinApp
{
public:
	CMyInjecterApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMyInjecterApp theApp;