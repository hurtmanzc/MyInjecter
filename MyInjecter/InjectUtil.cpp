#include "StdAfx.h"
#include "InjectUtil.h"

OSType CheckOS()
{
	OSVERSIONINFO os_version;

	os_version.dwOSVersionInfoSize = sizeof(os_version);

	if (GetVersionEx(&os_version))
	{
		if (os_version.dwMajorVersion == 5)
		{

			//wprintf(TEXT("[+] OS version: Windows XP\n"));
			return WindowsXP;
		}
		if (os_version.dwMajorVersion == 6 && os_version.dwMinorVersion == 0)
		{
			//wprintf(TEXT("[+] OS version: Windows Vista\n"));
			return WindowsVista;
		}
		if (os_version.dwMajorVersion == 6 && os_version.dwMinorVersion == 1)
		{
			//wprintf(TEXT("[+] OS version: Windows 7\n"));
			return Windows7;
		}
		if (os_version.dwMajorVersion == 6 && os_version.dwMinorVersion == 2)
		{
			//wprintf(TEXT("[+] OS version: Windows 10\n"));
			return Windows10;
		}
	}
	//else
	//printf("[-] OS version detect failed.\n");

	return Unknown;

}

//DWORD InjectDll4Win7(DWORD dwProcessId, PCWSTR szDllPath)
//{
//	HANDLE hRemoteThread = NULL;
//	NtCreateThreadExBuffer ntbuffer;
//	LARGE_INTEGER dwTmp1 = { 0 };
//	LARGE_INTEGER dwTmp2 = { 0 };
//	char cTmp[100] = { 0x00 };
//	memset(&ntbuffer, 0, sizeof(NtCreateThreadExBuffer));
//
//	DWORD dwSize = (lstrlenW(szDllPath) + 1) * sizeof(wchar_t);
//
//	HANDLE hProcess = OpenProcess(
//		PROCESS_QUERY_INFORMATION |
//		PROCESS_CREATE_THREAD |
//		PROCESS_VM_OPERATION |
//		PROCESS_VM_WRITE,
//		FALSE, dwProcessId);
//
//	if (hProcess == NULL)
//	{
//		//sprintf_s(cTmp, "%s  OpenProcess errcode[%d]", GetCurTime().c_str(), GetLastError());
//		//fprintf(g_plog, "%s\n", cTmp);
//		return(1);
//	}
//
//	LPVOID pszLibFileRemote = (PWSTR)VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
//	if (pszLibFileRemote == NULL)
//	{
//		//sprintf_s(cTmp, "%s  VirtualAllocEx errcode[%d]", GetCurTime().c_str(), GetLastError());
//		//fprintf(g_plog, "%s\n", cTmp);
//		return(1);
//	}
//
//	int n = WriteProcessMemory(hProcess, pszLibFileRemote, (LPVOID)szDllPath, dwSize, NULL);
//	if (n == 0)
//	{
//		//sprintf_s(cTmp, "%s  WriteProcessMemory errcode[%d]", GetCurTime().c_str(), GetLastError());
//		//fprintf(g_plog, "%s\n", cTmp);
//		return(1);
//	}
//
//	PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
//	if (pfnThreadRtn == NULL)
//	{
//		//sprintf_s(cTmp, "%s  GetProcAddress errcode[%d]", GetCurTime().c_str(), GetLastError());
//		//fprintf(g_plog, "%s\n", cTmp);
//		return(1);
//	}
//
//	PTHREAD_START_ROUTINE ntCreateThreadExAddr = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("ntdll.dll")), "NtCreateThreadEx");
//	if (pfnThreadRtn == NULL)
//	{
//		//sprintf_s(cTmp, "%s  ntCreateThreadExAddr errcode[%d]", GetCurTime().c_str(), GetLastError());
//		//fprintf(g_plog, "%s\n", cTmp);
//		return(1);
//	}
//
//	if (ntCreateThreadExAddr)
//	{
//		ntbuffer.Size = sizeof(struct NtCreateThreadExBuffer);
//		ntbuffer.Unknown1 = 0x10003;
//		ntbuffer.Unknown2 = 0x8;
//		ntbuffer.Unknown3 = (DWORD*)&dwTmp2;
//		ntbuffer.Unknown4 = 0;
//		ntbuffer.Unknown5 = 0x10004;
//		ntbuffer.Unknown6 = 4;
//		ntbuffer.Unknown7 = (DWORD*)&dwTmp1;
//		ntbuffer.Unknown8 = 0;
//
//		FARPROC
//		LPFUN_NtCreateThreadEx funNtCreateThreadEx = (LPFUN_NtCreateThreadEx)ntCreateThreadExAddr;
//
//		NTSTATUS status = funNtCreateThreadEx(
//			&hRemoteThread,
//			0x1FFFFF,
//			NULL,
//			hProcess,
//			pfnThreadRtn,
//			(LPVOID)pszLibFileRemote,
//			FALSE,
//			NULL,
//			NULL,
//			NULL,
//			NULL
//			);
//
//#ifdef _DEBUG
//		//wprintf(TEXT("[+] Status: %s\n"), status);
//#endif
//		if (status != NULL)		// FIXME: always returns NULL even when it suceeds. Go figure.
//		{
//			//sprintf_s(cTmp, "%s  funNtCreateThreadEx errcode[%d]", GetCurTime().c_str(), GetLastError());
//			//fprintf(g_plog, "%s\n", cTmp);
//			return(1);
//		}
//		else
//		{
//			//sprintf_s(cTmp, "%s  funNtCreateThreadEx errcode[%d]", GetCurTime().c_str(), GetLastError());
//			//fprintf(g_plog, "%s\n", cTmp);
//			WaitForSingleObject(hRemoteThread, INFINITE);
//		}
//	}
//
//	if (pszLibFileRemote != NULL)
//		VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_RELEASE);
//
//	if (hRemoteThread != NULL)
//		CloseHandle(hRemoteThread);
//
//	if (hProcess != NULL)
//		CloseHandle(hProcess);
//
//	return(0);
//
//}


BOOL InjectDll4Win10(DWORD dwProcessId, LPCTSTR szDllPath) 
{
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	HMODULE hMod = NULL;
	LPVOID pRemoteBuf = NULL;  // 存储在目标进程申请的内存地址  
	DWORD dwBufSize = (DWORD)(_tcslen(szDllPath) + 1) * sizeof(TCHAR);  // 存储DLL文件路径所需的内存空间大小  
	LPTHREAD_START_ROUTINE pThreadProc;
	char cTmp[100] = { 0x00 };

	if (!(hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId))) {
		//sprintf_s(cTmp, "%s  OpenProcess errcode[%d]", GetCurTime().c_str(), GetLastError());
		//fprintf(g_plog, "%s\n", cTmp);
		return FALSE;
	}

	pRemoteBuf = VirtualAllocEx(hProcess, NULL, dwBufSize, MEM_COMMIT, PAGE_READWRITE);  // 在目标进程空间中申请内存  
	if (pRemoteBuf == NULL)
	{
		return FALSE;
		//sprintf_s(cTmp, "%s  VirtualAllocEx errcode[%d]", GetCurTime().c_str(), GetLastError());
		//fprintf(g_plog, "%s\n", cTmp);
	}
	if (!WriteProcessMemory(hProcess, pRemoteBuf, (LPVOID)szDllPath, dwBufSize, NULL))  // 向在目标进程申请的内存空间中写入DLL文件的路径  
	{
		//sprintf_s(cTmp, "%s  WriteProcessMemory errcode[%d]", GetCurTime().c_str(), GetLastError());
		//fprintf(g_plog, "%s\n", cTmp);
		return FALSE;
	}

	hMod = GetModuleHandle(L"kernel32.dll");
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, "LoadLibraryW");  // 获得LoadLibraryW()函数的地址  
	if (pThreadProc == NULL)
	{
		//sprintf_s(cTmp, "%s  GetProcAddress errcode[%d]", GetCurTime().c_str(), GetLastError());
		//fprintf(g_plog, "%s\n", cTmp);
		return FALSE;
	}

	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, pRemoteBuf, 0, NULL);
	if (hThread == NULL)
	{
		//sprintf_s(cTmp, "%s  CreateRemoteThread errcode[%d]", GetCurTime().c_str(), GetLastError());
		//fprintf(g_plog, "%s\n", cTmp);
		return FALSE;
	}
	WaitForSingleObject(hThread, INFINITE);

	//if (pRemoteBuf != NULL)
	//	VirtualFreeEx(hProcess, pRemoteBuf, 0, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(hProcess);

	return TRUE;

}