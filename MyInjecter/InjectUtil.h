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


// ע�� dll
// hWnd-Ŀ����̴��� 
// strDllName-׼��ע���dll����
BOOL InjectByWnd(HWND hWnd, const char* strDllPathName);
// ж��ע���dll
// hWnd-Ŀ����̴��� 
// strDllName-׼��ж�ص�dll����
BOOL UnInjectByWnd(HWND hWnd, const char* strDllPathName);

// ע�� dll
// wPid-Ŀ�����id 
// strDllName-׼��ע���dll����
BOOL InjectByPid(DWORD dwPid, const char* strDllPathName);
// ж��ע���dll
// dwPid-Ŀ�����id 
// strDllName-׼��ж�ص�dll����
BOOL UnInjectByPid(DWORD dwPid, const char* strDllPathName);

// tzExePathName - Ҫ���е�exe·���� 
// pPatchBaseAddr - Ҫpatch�Ļ���ַ 
// pPatchData - Ҫpatch������
// nPatchLen - Ҫpatch�ĳ���
// tzDllPathName - Ҫע���dll·����, 
BOOL LanchProcess(LPCTSTR szExePathName, PVOID pPatchBaseAddr, BYTE* pPatchData, BYTE nPatchLen, 
    const char* strDllPathName, DWORD& pid, CString& strError);


