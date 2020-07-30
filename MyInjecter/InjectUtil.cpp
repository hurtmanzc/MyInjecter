#include "StdAfx.h"
#include <tlhelp32.h>
#include "Utility.h"
#include "InjectUtil.h"

OSType CheckOS()
{
	OSVERSIONINFO os_version;
	os_version.dwOSVersionInfoSize = sizeof(os_version);
	if (GetVersionEx(&os_version))
	{
		if (os_version.dwMajorVersion == 5)
		{
			return WindowsXP;
		}
		if (os_version.dwMajorVersion == 6 && os_version.dwMinorVersion == 0)
		{
			return WindowsVista;
		}
		if (os_version.dwMajorVersion == 6 && os_version.dwMinorVersion == 1)
		{
			return Windows7;
		}
		if (os_version.dwMajorVersion == 6 && os_version.dwMinorVersion == 2)
		{
			return Windows10;
		}
	}

	return Unknown;
}

BOOL InjectDll4Win10(DWORD dwProcessId, LPCTSTR szDllPath, CString& strError) 
{
    strError.Empty();
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	HMODULE hMod = NULL;
	LPVOID pRemoteBuf = NULL;
	DWORD dwBufSize = (DWORD)(_tcslen(szDllPath) + 1) * sizeof(TCHAR); 
	LPTHREAD_START_ROUTINE pThreadProc;
	char cTmp[100] = { 0x00 };

	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId))) {
		sprintf_s(cTmp, "OpenProcess failed, errcode[%d]", GetLastError());
        strError = cTmp;
		return FALSE;
	}

	pRemoteBuf = VirtualAllocEx(hProcess, NULL, dwBufSize, MEM_COMMIT, PAGE_READWRITE);
	if (pRemoteBuf == NULL)
	{
        sprintf_s(cTmp, "VirtualAllocEx failed, errcode[%d]", GetLastError());
        strError = cTmp;
		return FALSE;
	}
	if (!WriteProcessMemory(hProcess, pRemoteBuf, (LPVOID)szDllPath, dwBufSize, NULL))
	{
        sprintf_s(cTmp, "WriteProcessMemory failed, errcode[%d]", GetLastError());
        strError = cTmp;
        return FALSE;
	}

	hMod = GetModuleHandle(_T("kernel32.dll"));
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, "LoadLibraryW"); 
	if (pThreadProc == NULL)
	{
        sprintf_s(cTmp, "GetProcAddress failed, errcode[%d]", GetLastError());
        strError = cTmp;
        return FALSE;
	}

	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, pRemoteBuf, 0, NULL);
	if (hThread == NULL)
	{
        sprintf_s(cTmp, "CreateRemoteThread failed, errcode[%d]", GetLastError());
        strError = cTmp;
        return FALSE;
	}
	WaitForSingleObject(hThread, INFINITE);

	//if (pRemoteBuf != NULL)
	//	VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);

	CloseHandle(hThread);
	CloseHandle(hProcess);

	return TRUE;

}

DWORD FindProcessId(LPCTSTR lpszProcPathName)   
{   
    DWORD dwRet = 0;   
    HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );   
    PROCESSENTRY32 pe32;   
    pe32.dwSize = sizeof( PROCESSENTRY32 );   
    Process32First( hSnapshot, &pe32 );   
    do  
    {   
        if ( lstrcmpi( pe32.szExeFile, lpszProcPathName ) == 0 )   
        {   
            dwRet = pe32.th32ProcessID;   
            break;   
        }   
    } while ( Process32Next( hSnapshot, &pe32 ) );   
    CloseHandle( hSnapshot );   
    return dwRet;   
} 

BOOL EnablePrivilege()
{
    HANDLE hToken;
    BOOL bOK = FALSE;
    if(::OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
    {
        TOKEN_PRIVILEGES tkp;
        ::LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
        tkp.PrivilegeCount = 1;
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        bOK = ::AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL);
        if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
        {
            //printf("The token does not have the specified privilege. \n");
            bOK = FALSE;
        }
    }
    return bOK;
}

// ע�� dll
// hWnd-Ŀ����̴��� 
// strDllFullPath-׼��ע���dll����
BOOL InjectByWnd(HWND hWnd, const char* strDllPathName)
{
    DWORD  dwPid = 0;
    HANDLE hProcess = NULL, hThread = NULL;
    DWORD  dwSize = 0, dwWritten = 0;   
    LPVOID lpBuf = NULL;

    ::GetWindowThreadProcessId(hWnd, &dwPid);
    return InjectByPid(dwPid, strDllPathName);
} 

// ж��ע���dll
// hWnd-Ŀ����̴��� 
// strDllFullPath-׼��ж�ص�dll����
BOOL UnInjectByWnd(HWND hWnd, const char* strDllPathName)
{
    DWORD dwPid = 0;
    ::GetWindowThreadProcessId(hWnd, &dwPid);
    return UnInjectByPid(dwPid, strDllPathName);
} 

// ע�� dll
// wPid-Ŀ�����id 
// strDllName-׼��ע���dll����
BOOL InjectByPid(DWORD dwProcessId, const char* strDllPathName) 
{ 
    BOOL   bResult          = FALSE; 
    HANDLE hProcess         = NULL; 
    HANDLE hThread          = NULL; 
    PSTR   pszDllFileBufRemote = NULL;

    __try 
    { 
        EnablePrivilege();

        // �����Ҫע�����Ľ��̵ľ��. 
        hProcess = ::OpenProcess( 
            PROCESS_ALL_ACCESS, 
            FALSE, 
            dwProcessId 
            );

        if (hProcess == NULL) 
            __leave; 

        // ����DLL·������Ҫ���ֽ���. 
        int cch = 1 + strlen(strDllPathName);

        // ��Զ���߳���Ϊ·��������ռ�. 
        pszDllFileBufRemote = (PSTR)::VirtualAllocEx( 
            hProcess, 
            NULL, 
            cch, 
            MEM_COMMIT, 
            PAGE_READWRITE 
            );

        if (pszDllFileBufRemote == NULL) 
            __leave; 

        // ��DLL��·�������Ƶ�Զ�̽��̵��ڴ�ռ�. 
        if (!::WriteProcessMemory( 
            hProcess, 
            (PVOID)pszDllFileBufRemote, 
            (PVOID)strDllPathName, 
            cch, 
            NULL)) 
            __leave; 


        // ���LoadLibraryA��Kernel32.dll�е�������ַ
        // ע�⣺GetProcAddress������û������GetProcAddressW/GetProcAddressA���������֣�
        // ��GetProcAddress�ĵڶ���������LPCSTR��������ΪWin32ƽ̨�����ڵ�����ʱ��������ANSI�ַ���
        // ������ȷ��д����������
        // GetProcAddress(hModComCtl,(LPCSTR)("LoadLibraryA"));
        // �ڶ�����������ǿ��ת��,�Ա�����C++�ϸ����﷨ʱ����
        PTHREAD_START_ROUTINE pfnThreadRtn = 
            (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32")), (LPCSTR)("LoadLibraryA"));

        if (pfnThreadRtn == NULL) 
            __leave;

        // ����Զ���̣߳���ͨ��Զ���̵߳����û���DLL�ļ�. 
        hThread = ::CreateRemoteThread( 
            hProcess, 
            NULL, 
            0, 
            pfnThreadRtn, 
            (PVOID)pszDllFileBufRemote, 
            0, 
            NULL 
            ); 
        if (hThread == NULL) 
            __leave;

        // �ȴ�Զ���߳���ֹ. 
        ::WaitForSingleObject(hThread, INFINITE);

        bResult = TRUE; 
    } 
    __finally 
    { 
        // �رվ��. 
        if (pszDllFileBufRemote != NULL) 
            ::VirtualFreeEx(hProcess, (PVOID)pszDllFileBufRemote, 0, MEM_RELEASE);

        if (hThread != NULL) 
            ::CloseHandle(hThread);

        if (hProcess != NULL) 
            ::CloseHandle(hProcess); 
    } 
    return bResult; 
}

// ж��ע���dll
// dwPid-Ŀ�����id 
// strDllName-׼��ж�ص�dll����
BOOL UnInjectByPid(DWORD dwProcessId, const char* strDllPathName) 
{ 
    BOOL   bResult     = FALSE; 
    HANDLE hProcess    = NULL; 
    HANDLE hThread     = NULL; 
    HANDLE hthSnapshot = NULL; 
    MODULEENTRY32 hMod = {sizeof(hMod)}; 

    __try 
    { 
        // �򿪽���. 
        hProcess = ::OpenProcess( 
            PROCESS_CREATE_THREAD|PROCESS_VM_OPERATION, 
            FALSE, 
            dwProcessId 
            ); 

        if (hProcess == NULL) 
            __leave; 

        // ȡ��FreeLibrary������Kernel32.dll�еĵ�ַ
        // ע�⣺GetProcAddress������û������GetProcAddressW/GetProcAddressA���������֣�
        // ��GetProcAddress�ĵڶ���������LPCSTR��������ΪWin32ƽ̨�����ڵ�����ʱ��������ANSI�ַ���
        // ������ȷ��д����������
        // GetProcAddress(hModComCtl,(LPCSTR)("LoadLibraryA"));
        // �ڶ�����������ǿ��ת��,�Ա�����C++�ϸ����﷨ʱ����
        PTHREAD_START_ROUTINE pfnThreadRtn = 
            (PTHREAD_START_ROUTINE)::GetProcAddress(::GetModuleHandle(_T("Kernel32.dll")), (LPCSTR)("FreeLibrary"));

        if (pfnThreadRtn == NULL) 
            __leave;

        // ȡ��ָ�����̵�����ģ��ӳ��. 
        hthSnapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId); 
        if (hthSnapshot == NULL) 
            __leave; 

        // ȡ������ģ���б��е�ָ����ģ��. 
        BOOL bMoreMods = ::Module32First(hthSnapshot, &hMod); 
        if (bMoreMods == FALSE) 
            __leave; 

        TCHAR* pDllPathName = NULL;
#ifdef UNICODE
        wchar_t* pWideDllPathName = AnsiStringToWideString(strDllPathName);
        pDllPathName = pWideDllPathName;
#else
        pDllPathName = (LPTSTR)strDllPathName;
#endif
        // ѭ��ȡ����Ҫ��ģ��. 
        for (;bMoreMods; bMoreMods = ::Module32Next(hthSnapshot, &hMod)) 
        {
            if ((_tcsicmp(hMod.szExePath, pDllPathName) == 0) 
                || (_tcsicmp(hMod.szModule, pDllPathName) == 0))
            { 
                break; 
            } 
        } 
#ifdef UNICODE
        free(pWideDllPathName); pWideDllPathName = NULL;
#endif

        // ����Զ���߳���ִ��FreeLibrary����. 
        hThread = ::CreateRemoteThread(hProcess, 
            NULL, 
            0, 
            pfnThreadRtn, 
            hMod.modBaseAddr, 
            0, 
            NULL); 
        if (hThread == NULL) 
            __leave; 

        // �ȴ�Զ���߳���ֹ. 
        ::WaitForSingleObject(hThread, INFINITE); 

        bResult=TRUE; 

    } 
    __finally 
    { 
        // �رվ��. 
        if (hThread != NULL) 
            ::CloseHandle(hThread);

        if (hthSnapshot != NULL) 
            ::CloseHandle(hthSnapshot); 

        if (hProcess != NULL) 
            ::CloseHandle(hProcess); 
    } 

    return bResult; 
}

// tzExePathName - Ҫ���е�exe·���� 
// pPatchBaseAddr - Ҫpatch�Ļ���ַ 
// pPatchData - Ҫpatch������
// nPatchLen - Ҫpatch�ĳ���
// tzDllPathName - Ҫע���dll·����, 
BOOL LanchProcess(LPCTSTR szExePathName, PVOID pPatchBaseAddr, BYTE* pPatchData, BYTE nPatchLen, 
    const char* strDllPathName, DWORD& pid, CString& strError)
{
    strError.Empty();
    pid = 0;
    TCHAR tzMessage[260] = {0};

    STARTUPINFO	si;
    PROCESS_INFORMATION	pi;
    TCHAR tzFileFolder[MAX_PATH] = {0};

    _tcsncpy(tzFileFolder, szExePathName, MAX_PATH);
    PathRemoveFileSpec(tzFileFolder);

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    BOOL bOK = CreateProcess(NULL, 
        (TCHAR*)szExePathName, 
        NULL, 
        NULL, 
        TRUE, 
        CREATE_SUSPENDED | CREATE_DEFAULT_ERROR_MODE, 
        NULL, 
        (TCHAR*)tzFileFolder, 
        &si, 
        &pi);

    if(!bOK)
    {
        _stprintf(tzMessage, _T("CreateProcess fail:0x%.8x\n"), GetLastError());
        strError = tzMessage;
        return FALSE;
    }
    pid = pi.dwProcessId;
    if(pPatchData != NULL && nPatchLen > 0)
    {
        DWORD dwNumberOfBytesWritten;
        bOK = ::WriteProcessMemory(pi.hProcess, pPatchBaseAddr, pPatchData, nPatchLen, &dwNumberOfBytesWritten);
        if(!bOK)
        {
            _stprintf(tzMessage, _T("PatchData fail:0x%.8x\n"), GetLastError());
            strError = tzMessage;
            return FALSE;
        }
    }

    // Ŀ����ָ̻�����
    ResumeThread(pi.hThread);

    if(strDllPathName != NULL && strlen(strDllPathName) > 0)
    {
        Sleep(2000);
//#ifdef DEBUG
//        HMODULE hHookDll = NULL;
//        hHookDll = ::LoadLibraryA(strDllPathName);
//        if(hHookDll == NULL)
//        {
//            _tcsncpy(tzMessage, _T("����ָ�����ļ�ʧ��"), 260);
//            strError = tzMessage;
//            return FALSE;
//        }
//        TCHAR strDll[MAX_PATH] = {0};
//        ::GetModuleFileName(hHookDll, strDll, MAX_PATH);
//#endif
        InjectByPid(pi.dwProcessId, strDllPathName);
        if(!bOK)
        {
            _stprintf(tzMessage, _T("Inject dll fail:0x%.8x\n"), GetLastError());
            strError = tzMessage;
            return FALSE;
        }
    }

    return TRUE;
}

