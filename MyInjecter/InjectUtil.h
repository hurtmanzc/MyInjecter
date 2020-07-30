#pragma once

enum OSType
{
	Unknown = 0,
	WindowsXP = 50,
	WindowsVista = 60,
	Windows7 = 61,
	Windows10 = 62
};

OSType CheckOS();

BOOL InjectDll4Win10(DWORD dwProcessId, LPCTSTR szDllPath, CString& strError);


// 注入 dll
// hWnd-目标进程窗口 
// strDllName-准备注入的dll名称
BOOL InjectByWnd(HWND hWnd, const char* strDllPathName);
// 卸载注入的dll
// hWnd-目标进程窗口 
// strDllName-准备卸载的dll名称
BOOL UnInjectByWnd(HWND hWnd, const char* strDllPathName);

// 注入 dll
// wPid-目标进程id 
// strDllName-准备注入的dll名称
BOOL InjectByPid(DWORD dwPid, const char* strDllPathName);
// 卸载注入的dll
// dwPid-目标进程id 
// strDllName-准备卸载的dll名称
BOOL UnInjectByPid(DWORD dwPid, const char* strDllPathName);

// tzExePathName - 要运行的exe路径名 
// pPatchBaseAddr - 要patch的基地址 
// pPatchData - 要patch的内容
// nPatchLen - 要patch的长度
// tzDllPathName - 要注入的dll路径名, 
BOOL LanchProcess(LPCTSTR szExePathName, PVOID pPatchBaseAddr, BYTE* pPatchData, BYTE nPatchLen, 
    const char* strDllPathName, DWORD& pid, CString& strError);


