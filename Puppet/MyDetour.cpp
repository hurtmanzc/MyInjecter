#include "stdafx.h"
#include "Utility.h"
#include "MyDetour.h"
#include "LogFile.h"
#include <WinSock2.h>
#include "../detours/detours.h"

//#pragma comment (lib, "..\\detours\\lib\\detours.lib")
//#pragma comment (lib, "..\\detours\\lib\\syelog.lib")

CLogFile gSendLog("send.log");
CLogFile gRecvLog("recv.log");

BOOL DetourApi(PVOID *ppPointer, PVOID pDetour)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(ppPointer, pDetour);
	DetourTransactionCommit();

	return TRUE;
}

BOOL UnDetourApi(PVOID *ppPointer, PVOID pDetour)
{
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourDetach(ppPointer, pDetour);
	DetourTransactionCommit();

	return TRUE;
}

// Target pointer for the uninstrumented Sleep API.
static void (WINAPI * TrueSleep)(DWORD dwMilliseconds) = Sleep;
static LONG dwSlept = 0;
// Detour function that replaces the Sleep API.
void WINAPI TimedSleep(DWORD dwMilliseconds)
{
	// Save the before and after times around calling the Sleep API.
	DWORD dwBeg = GetTickCount();
	TrueSleep(dwMilliseconds);
	DWORD dwEnd = GetTickCount();
	InterlockedExchangeAdd(&dwSlept, dwEnd - dwBeg);
}

static BOOL (WINAPI * sysIsDebuggerPresent)(VOID) = IsDebuggerPresent;
BOOL WINAPI myIsDebuggerPresent ( VOID )
{
	DWORD dwThreadId = ::GetCurrentThreadId();
	dbgprint("myIsDebuggerPresent dwThreadId=%d", dwThreadId);
    return FALSE;
}

static LRESULT (WINAPI * sysCallNextHookEx)( HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam) = CallNextHookEx;
LRESULT WINAPI myCallNextHookEx(  HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam)
{
	//DWORD dwThreadId = ::GetCurrentThreadId();
	//dbgprint("myCallNextHookEx dwThreadId=%d hhk=%d", dwThreadId, hhk);
	return sysCallNextHookEx( hhk, nCode, wParam, lParam);
}

struct MyHOOK
{
	HHOOK	 hHook;
    HOOKPROC lpfn;
};

static CMap<DWORD,DWORD,MyHOOK,MyHOOK> s_KbdHookMap;
LRESULT CALLBACK myKeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	DWORD dwThreadId = ::GetCurrentThreadId();

	// Are we expected to handle this callback?
	if (nCode == HC_ACTION)
	{
		// Is this keyboard event "real" or "injected"
		// i.e. hardware or software-produced?
		UINT nParam=0;
		if (wParam==WM_KEYDOWN || wParam==WM_SYSKEYDOWN)
			nParam=1;
		else if (wParam == WM_KEYUP || wParam==WM_SYSKEYUP)
			nParam=2;

		KBDLLHOOKSTRUCT *hookStruct = (KBDLLHOOKSTRUCT*)lParam;
		dbgprint("myKeyboardHookProc dwThreadId=%d vkCode=%d", dwThreadId, hookStruct->vkCode);
	}

	//if(s_KbdHookMap[dwThreadId].lpfn != NULL 
	//	&& s_KbdHookMap[dwThreadId].lpfn != myKeyboardHookProc )
	//{
	//	s_KbdHookMap[dwThreadId].lpfn(nCode, wParam, lParam);
	//}

	HHOOK hHook = s_KbdHookMap[dwThreadId].hHook;
	dbgprint("myKeyboardHookProc dwThreadId=%d CallNextHookEx=%d", dwThreadId, (DWORD)hHook);
	return sysCallNextHookEx(hHook, nCode, wParam, lParam);
}

static CMap<DWORD,DWORD,MyHOOK,MyHOOK> s_KbdLLHookMap;
LRESULT CALLBACK myKeyboardLLHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	DWORD dwThreadId = ::GetCurrentThreadId();

	// Are we expected to handle this callback?
	if (nCode == HC_ACTION)
	{
		// Is this keyboard event "real" or "injected"
		// i.e. hardware or software-produced?
		UINT nParam=0;
		if (wParam==WM_KEYDOWN || wParam==WM_SYSKEYDOWN)
			nParam=1;
		else if (wParam == WM_KEYUP || wParam==WM_SYSKEYUP)
			nParam=2;

		KBDLLHOOKSTRUCT *hookStruct = (KBDLLHOOKSTRUCT*)lParam;
		dbgprint("myKeyboardLLHookProc vkCode=%d", hookStruct->vkCode);
	}

	//if(s_KbdLLHookMap[dwThreadId].lpfn != NULL 
	//	&& s_KbdLLHookMap[dwThreadId].lpfn != myKeyboardLLHookProc)
	//{
	//	s_KbdLLHookMap[dwThreadId].lpfn(nCode, wParam, lParam);
	//}

	HHOOK hHook = s_KbdLLHookMap[dwThreadId].hHook;
	dbgprint("myKeyboardLLHookProc CallNextHookEx=%d", (DWORD)hHook);
	return sysCallNextHookEx(hHook, nCode, wParam, lParam);
}

static CMap<DWORD,DWORD,MyHOOK,MyHOOK> s_MsgFilterHookMap;
LRESULT CALLBACK myMsgFilterHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	DWORD dwThreadId = ::GetCurrentThreadId();
	HHOOK hHook = s_MsgFilterHookMap[dwThreadId].hHook;
	dbgprint("myMsgFilterHookProc dwThreadId=%d", dwThreadId);
	return sysCallNextHookEx(hHook, nCode, wParam, lParam);
}

HHOOK (WINAPI * sysSetWindowsHookExA)( int idHook, HOOKPROC lpfn,  HINSTANCE hmod, DWORD dwThreadId) = SetWindowsHookExA;
HHOOK WINAPI mySetWindowsHookExA( int idHook, HOOKPROC lpfn,  HINSTANCE hmod, DWORD dwThreadId)
{
	DWORD dwlpfn = (DWORD)lpfn;
	HHOOK hHook = NULL;
	switch(idHook)
	{
	case WH_MSGFILTER:		//-1 线程级; 截获用户与控件交互的消息
		{
			DWORD dwId = ::GetCurrentThreadId();
			hHook = sysSetWindowsHookExA(idHook, myMsgFilterHookProc, hmod, 0/*dwThreadId*/);
			s_MsgFilterHookMap[dwId].hHook = hHook;
			s_MsgFilterHookMap[dwId].lpfn = lpfn;
			dwlpfn = (DWORD)myMsgFilterHookProc;
		}
		break;
	case WH_JOURNALRECORD:	//0 系统级; 记录所有消息队列从消息队列送出的输入消息, 在消息从队列中清除时发生; 可用于宏记录
		hHook = sysSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
		break;
	case WH_JOURNALPLAYBACK://1 系统级; 回放由 WH_JOURNALRECORD 记录的消息, 也就是将这些消息重新送入消息队列
		hHook = sysSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
		break;
	case WH_KEYBOARD:		//2 系统级或线程级; 截获键盘消息
		{
			DWORD dwId = ::GetCurrentThreadId();
			hHook = sysSetWindowsHookExA(idHook, myKeyboardHookProc, hmod, 0/*dwThreadId*/);
			s_KbdHookMap[dwId].hHook = hHook;
			s_KbdHookMap[dwId].lpfn = lpfn;
			dwlpfn = (DWORD)myKeyboardHookProc;
		}
		break;
	case WH_GETMESSAGE:		//3 系统级或线程级; 截获从消息队列送出的消息
		hHook = sysSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
		break;
	case WH_CALLWNDPROC:	//4 系统级或线程级; 截获发送到目标窗口的消息, 在 SendMessage 调用时发生
		hHook = sysSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
		break;
	case WH_CBT:			//5 系统级或线程级; 截获系统基本消息, 譬如: 窗口的创建、激活、关闭、最大最小化、移动等等
		hHook = sysSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
		break;
	case WH_SYSMSGFILTER:	//6 系统级; 截获系统范围内用户与控件交互的消息
		hHook = sysSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
		break;
	case WH_MOUSE:			//7 系统级或线程级; 截获鼠标消息
		hHook = sysSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
		break;
	//case WH_HARDWARE:		//8 系统级或线程级; 截获非标准硬件(非鼠标、键盘)的消息
	//	hHook = sysSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
	//	break;
	case WH_DEBUG:			//9 系统级或线程级; 在其他钩子调用前调用, 用于调试钩子
		hHook = sysSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
		break;
	case WH_SHELL:			//10 系统级或线程级; 截获发向外壳应用程序的消息
		hHook = sysSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
		break;
	case WH_FOREGROUNDIDLE:	//11 系统级或线程级; 在程序前台线程空闲时调用
		hHook = sysSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
		break;
	case WH_CALLWNDPROCRET:	//12 系统级或线程级; 截获目标窗口处理完毕的消息, 在 SendMessage 调用后发生
		hHook = sysSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
		break;
	case WH_KEYBOARD_LL:	//13 系统级或线程级; 截获目标窗口处理完毕的消息, 在 SendMessage 调用后发生
		{
			DWORD dwId = ::GetCurrentThreadId();
			hHook = sysSetWindowsHookExA(idHook, myKeyboardLLHookProc, hmod, 0/*dwThreadId*/);
			s_KbdLLHookMap[dwId].hHook = hHook;
			s_KbdLLHookMap[dwId].lpfn = lpfn;
			dwlpfn = (DWORD)myKeyboardLLHookProc;
		}
		break;
	case WH_MOUSE_LL:		//14 系统级或线程级; 截获目标窗口处理完毕的消息, 在 SendMessage 调用后发生
		hHook = sysSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
		break;
	default:
		break;
	}

	dbgprint("mySetWindowsHookExA idHook=%d dwThreadId=%d hhk=%d lpfn=%d", idHook, dwThreadId, (DWORD)hHook, (DWORD)lpfn);
	return hHook;
}

BOOL (WINAPI * sysUnhookWindowsHookEx)( HHOOK hhk) = UnhookWindowsHookEx;
BOOL WINAPI myUnhookWindowsHookEx( HHOOK hhk)
{
	DWORD dwThreadId = ::GetCurrentThreadId();
	dbgprint("myUnhookWindowsHookEx hhk=%d dwThreadId=%d", hhk, dwThreadId);
	return sysUnhookWindowsHookEx(hhk);
}

int (WSAAPI * sysSend)( IN SOCKET s, IN const char FAR * buf, IN int len, IN int flags ) = send;
int WSAAPI mySend( IN SOCKET s, IN const char FAR * buf, IN int len, IN int flags )
{
	DWORD dwThreadId = ::GetCurrentThreadId();
	dbgprint("Send ThreadId=%d socket=%u length=%d", dwThreadId, s, len);
    if(buf != NULL && len > 0)
    {
        char* pBuf = (char*)malloc(len*3);
        memset(pBuf, 0, len*3);
        HexToStr((BYTE*)buf, len, pBuf);
        dbgprint(pBuf);
        //gSendLog.Log(pBuf);
        free(pBuf);
    }
	return sysSend(s, buf, len, flags);
}

int (WSAAPI * sysRecv)( IN SOCKET s, OUT char FAR * buf, IN int len, IN int flags ) = recv;
int WSAAPI myRecv( IN SOCKET s, OUT char FAR * buf, IN int len, IN int flags )
{
	DWORD dwThreadId = ::GetCurrentThreadId();
	int rtn = sysRecv(s, buf, len, flags);
	dbgprint("Recv ThreadId=%d socket=%u length=%d", dwThreadId, s, rtn);
    if(buf != NULL && rtn > 0)
    {
        char* pBuf = (char*)malloc(rtn*3);
        memset(pBuf, 0, rtn*3);
        HexToStr((BYTE*)buf, rtn, pBuf);
        dbgprint(pBuf);
        //gRecvLog.Log(pBuf);
        free(pBuf);
    }

	return rtn;
}

int (WSAAPI * sysSendTo)( IN SOCKET s, IN const char FAR * buf, IN int len, IN int flags, IN const struct sockaddr FAR * to, IN int tolen ) = sendto;
int WSAAPI mySendTo( IN SOCKET s, IN const char FAR * buf, IN int len, IN int flags, IN const struct sockaddr FAR * to, IN int tolen )
{
	DWORD dwThreadId = ::GetCurrentThreadId();
	dbgprint("SendTo dwThreadId=%d length=%d", dwThreadId, len);
	dbgprint("data:%s", buf);
	return sysSendTo(s, buf, len, flags, to, tolen);
}

int (WSAAPI * sysRecvFrom)( IN SOCKET s, OUT char FAR * buf, IN int len, IN int flags, OUT struct sockaddr FAR * from, IN OUT int FAR * fromlen ) = recvfrom;
int WSAAPI myRecvFrom( IN SOCKET s, OUT char FAR * buf, IN int len, IN int flags, OUT struct sockaddr FAR * from, IN OUT int FAR * fromlen )
{
	DWORD dwThreadId = ::GetCurrentThreadId();
	int rtn = sysRecvFrom(s, buf, len, flags, from, fromlen);
	dbgprint("RecvFrom dwThreadId=%d length=%d", dwThreadId, rtn);
	dbgprint("data:%s", buf);

	return rtn;
}

int (WSAAPI * sysWSASend)( SOCKET s, 
						   LPWSABUF lpBuffers, 
						   DWORD dwBufferCount, 
						   LPDWORD lpNumberOfBytesSent, 
						   DWORD dwFlags, 
						   LPWSAOVERLAPPED lpOverlapped, 
						   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine 
						  ) = WSASend;
int WSAAPI myWSASend( SOCKET s, 
					  LPWSABUF lpBuffers, 
					  DWORD dwBufferCount, 
					  LPDWORD lpNumberOfBytesSent, 
					  DWORD dwFlags, 
					  LPWSAOVERLAPPED lpOverlapped, 
					  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
					  )
{
	DWORD dwPid = ::GetCurrentProcessId();//::GetCurrentThreadId();
	dbgprint("WSASend pid=%d length=%d", dwPid, dwBufferCount);
    if(lpBuffers != NULL && dwBufferCount > 0)
    {
        char* pBuf = (char*)malloc(dwBufferCount*3);
        memset(pBuf, 0, dwBufferCount*3);
        HexToStr((BYTE*)lpBuffers, dwBufferCount, pBuf);
        dbgprint(pBuf);
        //gSendLog.Log(pBuf);
        free(pBuf);
    }

	int rtn = sysWSASend( s, 
						lpBuffers, 
						dwBufferCount, 
						lpNumberOfBytesSent, 
						dwFlags, 
						lpOverlapped, 
					    lpCompletionRoutine);
	return rtn;
}

int (WSAAPI * sysWSARecv)( SOCKET s, 
						  LPWSABUF lpBuffers, 
						  DWORD dwBufferCount, 
						  LPDWORD lpNumberOfBytesRecvd, 
						  LPDWORD lpFlags, 
						  LPWSAOVERLAPPED lpOverlapped, 
						  LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine 
						  ) = WSARecv;
int WSAAPI myWSARecv( SOCKET s, 
					 LPWSABUF lpBuffers, 
					 DWORD dwBufferCount, 
					 LPDWORD lpNumberOfBytesRecvd, 
					 LPDWORD lpFlags, 
					 LPWSAOVERLAPPED lpOverlapped, 
					 LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
					 )
{
	DWORD dwPid = ::GetCurrentProcessId();//::GetCurrentThreadId();
	int rtn = sysWSARecv(  s, 
		lpBuffers, 
		dwBufferCount, 
		lpNumberOfBytesRecvd, 
		lpFlags, 
		lpOverlapped, 
		lpCompletionRoutine);
	dbgprint("WSARecv pid=%d length=%d", dwPid, *lpNumberOfBytesRecvd);
    if(lpBuffers != NULL && *lpNumberOfBytesRecvd > 0)
    {
        char* pBuf = (char*)malloc(*lpNumberOfBytesRecvd*3);
        memset(pBuf, 0, *lpNumberOfBytesRecvd*3);
        HexToStr((BYTE*)lpBuffers, *lpNumberOfBytesRecvd, pBuf);
        dbgprint(pBuf);
        //gRecvLog.Log(pBuf);
        free(pBuf);
    }

	return rtn;
}


BOOL (WINAPI * sysTerminateProcess)( HANDLE hProcess, UINT uExitCode ) = TerminateProcess;
BOOL WINAPI myTerminateProcess( HANDLE hProcess, UINT uExitCode )
{
	DWORD dwThreadId = ::GetCurrentThreadId();
	dbgprint("myTerminateProcessdwThreadId=%d", dwThreadId);
	//return TRUE;
	return sysTerminateProcess(hProcess, uExitCode);
}

BOOL (WINAPI * sysWriteFile)(
	__in        HANDLE hFile,
	__in_bcount_opt(nNumberOfBytesToWrite) LPCVOID lpBuffer,
	__in        DWORD nNumberOfBytesToWrite,
	__out_opt   LPDWORD lpNumberOfBytesWritten,
	__inout_opt LPOVERLAPPED lpOverlapped
	) = WriteFile;

BOOL WINAPI myWriteFile(
	__in        HANDLE hFile,
	__in_bcount_opt(nNumberOfBytesToWrite) LPCVOID lpBuffer,
	__in        DWORD nNumberOfBytesToWrite,
	__out_opt   LPDWORD lpNumberOfBytesWritten,
	__inout_opt LPOVERLAPPED lpOverlapped
	)
{
	DWORD dwThreadId = ::GetCurrentThreadId();
    //char szFileName[MAX_PATH];
    //memset(szFileName, 0, MAX_PATH);
    //GetFileNameByHandle(hFile, szFileName, MAX_PATH);
    dbgprint("WriteFile ThreadId=%d FileHandle=%u Length=%d", dwThreadId, hFile, nNumberOfBytesToWrite);
    if(lpBuffer != NULL && nNumberOfBytesToWrite > 0)
    {
        char* pBuf = (char*)malloc(nNumberOfBytesToWrite*3);
        memset(pBuf, 0, nNumberOfBytesToWrite*3);
        HexToStr((BYTE*)lpBuffer, nNumberOfBytesToWrite, pBuf);
        dbgprint(pBuf);
        // 注意这里不能写入日志文件，写入时调用WriteFile，形成递归死循环！！！
        //gSendLog.Log(pBuf);
        free(pBuf);
    }
	return sysWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

BOOL (WINAPI * sysReadFile)(
	__in        HANDLE hFile,
	__out_bcount_part_opt(nNumberOfBytesToRead, *lpNumberOfBytesRead) __out_data_source(FILE) LPVOID lpBuffer,
	__in        DWORD nNumberOfBytesToRead,
	__out_opt   LPDWORD lpNumberOfBytesRead,
	__inout_opt LPOVERLAPPED lpOverlapped
	) = ReadFile;
BOOL WINAPI myReadFile(
	__in        HANDLE hFile,
	__out_bcount_part_opt(nNumberOfBytesToRead, *lpNumberOfBytesRead) __out_data_source(FILE) LPVOID lpBuffer,
	__in        DWORD nNumberOfBytesToRead,
	__out_opt   LPDWORD lpNumberOfBytesRead,
	__inout_opt LPOVERLAPPED lpOverlapped
	)
{
    BOOL bOK = sysReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
	DWORD dwThreadId = ::GetCurrentThreadId();
    //char szFileName[MAX_PATH];
    //memset(szFileName, 0, MAX_PATH);
    //GetFileNameByHandle(hFile, szFileName, MAX_PATH);
    dbgprint("ReadFile ThreadId=%d FileHandle=%u Length=%d", dwThreadId, hFile, *lpNumberOfBytesRead);
    if(lpBuffer != NULL && *lpNumberOfBytesRead > 0)
    {
        char* pBuf = (char*)malloc(*lpNumberOfBytesRead*3);
        memset(pBuf, 0, *lpNumberOfBytesRead*3);
        HexToStr((BYTE*)lpBuffer, *lpNumberOfBytesRead, pBuf);
        dbgprint(pBuf);
        //gRecvLog.Log(pBuf);
        free(pBuf);
    }
	return bOK;
}

BOOL DoDetour()
{
    //DetourApi(&(PVOID&)TrueSleep, TimedSleep);
    //DetourApi(&(PVOID&)sysIsDebuggerPresent, myIsDebuggerPresent);
    //DetourApi(&(PVOID&)sysSetWindowsHookExA, mySetWindowsHookExA);
    //DetourApi(&(PVOID&)sysUnhookWindowsHookEx, myUnhookWindowsHookEx);
    //DetourApi(&(PVOID&)sysCallNextHookEx, myCallNextHookEx);
    //DetourApi(&(PVOID&)sysTerminateProcess, myTerminateProcess);
    DetourApi(&(PVOID&)sysSend, mySend);
    DetourApi(&(PVOID&)sysRecv, myRecv);
    //DetourApi(&(PVOID&)sysSendTo, mySendTo);
    //DetourApi(&(PVOID&)sysRecvFrom, myRecvFrom);
    //DetourApi(&(PVOID&)sysWSASend, myWSASend);
    //DetourApi(&(PVOID&)sysWSARecv, myWSARecv);
	//DetourApi(&(PVOID&)sysWriteFile, myWriteFile);
	//DetourApi(&(PVOID&)sysReadFile, myReadFile);

	return TRUE;
}

BOOL DoUndetour()
{
    //UnDetourApi(&(PVOID&)TrueSleep, TimedSleep);
    //UnDetourApi(&(PVOID&)sysIsDebuggerPresent, myIsDebuggerPresent);
    //UnDetourApi(&(PVOID&)sysSetWindowsHookExA, mySetWindowsHookExA);
    //UnDetourApi(&(PVOID&)sysUnhookWindowsHookEx, myUnhookWindowsHookEx);
    //UnDetourApi(&(PVOID&)sysCallNextHookEx, myCallNextHookEx);
    //UnDetourApi(&(PVOID&)sysTerminateProcess, myTerminateProcess);
    UnDetourApi(&(PVOID&)sysSend, mySend);
    UnDetourApi(&(PVOID&)sysRecv, myRecv);
    //UnDetourApi(&(PVOID&)sysSendTo, mySendTo);
    //UnDetourApi(&(PVOID&)sysRecvFrom, myRecvFrom);
    //UnDetourApi(&(PVOID&)sysWSASend, myWSASend);
    //UnDetourApi(&(PVOID&)sysWSARecv, myWSARecv);
    //UnDetourApi(&(PVOID&)sysWriteFile, myWriteFile);
    //UnDetourApi(&(PVOID&)sysReadFile, myReadFile);

	return TRUE;
}