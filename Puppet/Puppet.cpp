// Puppet.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "Puppet.h"
#include "Utility.h"
#include "MyDetour.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// inline hook 一些函数
BOOL HookFunctions()
{
	DWORD dwMainTid = GetMainThread(GetCurrentProcessId());
	DWORD dwCurTid = GetCurrentThreadId();
	DBGPRINT("HookFunctions CurTid=%d MainTid=%d", dwCurTid, dwMainTid);

	DoDetour();

	return TRUE;
}

// inline hook 一些函数
BOOL UnHookFunctions()
{
	DWORD dwMainTid = GetMainThread(GetCurrentProcessId());
	DWORD dwCurTid = GetCurrentThreadId();
	DBGPRINT("UnHookFunctions CurTid=%d MainTid=%d", dwCurTid, dwMainTid);

	DoUndetour();

	return TRUE;
}
//
//TODO: 如果此 DLL 相对于 MFC DLL 是动态链接的，
//		则从此 DLL 导出的任何调入
//		MFC 的函数必须将 AFX_MANAGE_STATE 宏添加到
//		该函数的最前面。
//
//		例如:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// 此处为普通函数体
//		}
//
//		此宏先于任何 MFC 调用
//		出现在每个函数中十分重要。这意味着
//		它必须作为函数中的第一个语句
//		出现，甚至先于所有对象变量声明，
//		这是因为它们的构造函数可能生成 MFC
//		DLL 调用。
//
//		有关其他详细信息，
//		请参阅 MFC 技术说明 33 和 58。
//

// CPuppetApp

BEGIN_MESSAGE_MAP(CPuppetApp, CWinApp)
END_MESSAGE_MAP()


// CPuppetApp 构造

CPuppetApp::CPuppetApp()
{
	// TODO: 在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}


// 唯一的一个 CPuppetApp 对象
CPuppetApp theApp;


// CPuppetApp 初始化
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
