// Puppet.cpp : ���� DLL �ĳ�ʼ�����̡�
//

#include "stdafx.h"
#include "Puppet.h"
#include "Utility.h"
#include "MyDetour.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// inline hook һЩ����
BOOL HookFunctions()
{
	DWORD dwMainTid = GetMainThread(GetCurrentProcessId());
	DWORD dwCurTid = GetCurrentThreadId();
	DBGPRINT("HookFunctions CurTid=%d MainTid=%d", dwCurTid, dwMainTid);

	DoDetour();

	return TRUE;
}

// inline hook һЩ����
BOOL UnHookFunctions()
{
	DWORD dwMainTid = GetMainThread(GetCurrentProcessId());
	DWORD dwCurTid = GetCurrentThreadId();
	DBGPRINT("UnHookFunctions CurTid=%d MainTid=%d", dwCurTid, dwMainTid);

	DoUndetour();

	return TRUE;
}
//
//TODO: ����� DLL ����� MFC DLL �Ƕ�̬���ӵģ�
//		��Ӵ� DLL �������κε���
//		MFC �ĺ������뽫 AFX_MANAGE_STATE ����ӵ�
//		�ú�������ǰ�档
//
//		����:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// �˴�Ϊ��ͨ������
//		}
//
//		�˺������κ� MFC ����
//		������ÿ��������ʮ����Ҫ������ζ��
//		��������Ϊ�����еĵ�һ�����
//		���֣������������ж������������
//		������Ϊ���ǵĹ��캯���������� MFC
//		DLL ���á�
//
//		�й�������ϸ��Ϣ��
//		����� MFC ����˵�� 33 �� 58��
//

// CPuppetApp

BEGIN_MESSAGE_MAP(CPuppetApp, CWinApp)
END_MESSAGE_MAP()


// CPuppetApp ����

CPuppetApp::CPuppetApp()
{
	// TODO: �ڴ˴���ӹ�����룬
	// ��������Ҫ�ĳ�ʼ�������� InitInstance ��
}


// Ψһ��һ�� CPuppetApp ����
CPuppetApp theApp;


// CPuppetApp ��ʼ��
BOOL CPuppetApp::InitInstance()
{
	CWinApp::InitInstance();

	DWORD dwCurPid = GetCurrentProcessId();
	DWORD dwCurTid = GetCurrentThreadId();
	DWORD dwMainTid = GetMainThread(dwCurPid);
	DBGPRINT("GHDll DLL_PROCESS_ATTACH CurPid=%d CurTid=%d MainTid=%d", dwCurPid, dwCurTid, dwMainTid);

	HookFunctions();

	return TRUE;
}


int CPuppetApp::ExitInstance()
{

	DWORD dwMainTid = GetMainThread(GetCurrentProcessId());
	DWORD dwCurTid = GetCurrentThreadId();
	DBGPRINT("GHDll DLL_PROCESS_DETACH CurTid=%d MainTid=%d", dwCurTid, dwMainTid);

	UnHookFunctions();

	return CWinApp::ExitInstance();
}
