// Puppet.h : Puppet DLL ����ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CPuppetApp
// �йش���ʵ�ֵ���Ϣ������� Puppet.cpp
//

class CPuppetApp : public CWinApp
{
public:
	CPuppetApp();

// ��д
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
	virtual int ExitInstance();
};
