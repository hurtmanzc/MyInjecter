// Utility.h: interface for the CUtility class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UTILITY_H__EC8E9F8B_368A_4981_8C39_B221C60C20BE__INCLUDED_)
#define AFX_UTILITY_H__EC8E9F8B_368A_4981_8C39_B221C60C20BE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

char *w2c(const wchar_t *pwstr, char *pcstr, int len);

void dbgprint(const char *format, ...);

int GetFileNameByHandle(HANDLE hFile,LPTSTR buff,DWORD size);

#ifdef OUTPUT_LOG
#define DBGPRINT dbgprint
#else
#define DBGPRINT /##/
#endif

DWORD GetMainThread(DWORD dwProcessID);

HWND GetWindowHandleByPid(DWORD dwProcessID, const char* sTitle = NULL);

char* WideStringToAnsiString(const wchar_t* strWide);

wchar_t* AnsiStringToWideString(const char* strAnsi);

void StrToHex(const char *pSrc, BYTE *pDest, int& nLen);

void HexToStr(BYTE *pSrc, int nLen, char *pDest, BOOL bWithSpace = TRUE);

// HWND GetHwndByProcessId(DWORD dwProcessId);

// BOOL GetSysTime(SYSTEMTIME& sysTime);

#endif // !defined(AFX_UTILITY_H__EC8E9F8B_368A_4981_8C39_B221C60C20BE__INCLUDED_)
