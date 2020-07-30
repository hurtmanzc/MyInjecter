#ifndef _LOGFILE_H
#define _LOGFILE_H

#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <windows.h>

void w2a(const wchar_t *pwstr, char *pcstr, int len)
{
    if(pcstr == NULL || pcstr == NULL)
        return;
    memset(pcstr, 0, len);
    int nlength = wcslen(pwstr);
    int nbytes = ::WideCharToMultiByte(0,0,pwstr,nlength,NULL,0,NULL,NULL);
    if(nbytes>len)   
        nbytes = len;
    ::WideCharToMultiByte(0,0,pwstr,nlength,pcstr,nbytes,NULL,NULL);
}
/*
CLogFile gLog("My.Log");
gLog.Log("test", 4);//记录日志
gLog.Log("系统启动");
CLogFileEx gLog(".", CLogFileEx::YEAR);//一年生成一个日志文件
CLogFileEx gLog(".\\Log", CLogFileEx::MONTH);//一月生成一个日志文件
CLogFileEx gLog(".\\Log", CLogFileEx::DAY);//一天生成一个日志文件
*/

class CLogFile
{
protected:
	CRITICAL_SECTION _csLock;
	TCHAR * _szFileName;
	HANDLE _hFile;
	bool OpenFile()//打开文件， 指针到文件尾
	{
		if(IsOpen())
			return true;
		if(!_szFileName)
			return false;
		_hFile =  CreateFile(
			_szFileName, 
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL 
			);
		if(!IsOpen() && GetLastError() == 2)//打开不成功， 且因为文件不存在， 创建文件
			_hFile =  CreateFile(
			_szFileName, 
			GENERIC_WRITE,
			FILE_SHARE_READ | FILE_SHARE_WRITE,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL 
			); 
		if(IsOpen())
			SetFilePointer(_hFile, 0, NULL, FILE_END);
		return IsOpen();
	}

	DWORD Write(LPCVOID lpBuffer, DWORD dwLength)
	{
		DWORD dwWriteLength = 0;
		if(IsOpen())
			WriteFile(_hFile, lpBuffer, dwLength, &dwWriteLength, NULL);
		return dwWriteLength;
	}

	virtual void WriteLog( LPCVOID lpBuffer, DWORD dwLength)//写日志, 可以扩展修改
	{
		time_t now;
		char temp[50];
		DWORD dwWriteLength;
        char* pLine = " -> ";
        char* pCR = "\r\n";

		if(IsOpen())
		{
			time(&now);
            memset(temp, 0, 50);
			strftime(temp, 50, "%Y-%m-%d %H:%M:%S", localtime(&now));
            WriteFile(_hFile, temp, strlen(temp), &dwWriteLength, NULL);
            WriteFile(_hFile, pLine, strlen(pLine), &dwWriteLength, NULL);
			WriteFile(_hFile, lpBuffer, dwLength, &dwWriteLength, NULL);
            WriteFile(_hFile, pCR, strlen(pCR), &dwWriteLength, NULL);
			
			FlushFileBuffers(_hFile);
		}
	}

	virtual void WriteHexLog(LPCVOID lpBuffer, DWORD dwLenBytes)
	{
		time_t now;
		char temp[50];
		char hex[4];
		DWORD dwWriteLength;
        char* pLine = " -> ";
        char* pCR = "\r\n";

		if(IsOpen())
		{
			time(&now);
            memset(temp, 0, 50);
			strftime(temp, 50, "%Y-%m-%d %H:%M:%S", localtime(&now));
            WriteFile(_hFile, temp, strlen(temp), &dwWriteLength, NULL);
            WriteFile(_hFile, pLine, strlen(pLine), &dwWriteLength, NULL);
            BYTE* pData = (BYTE*)lpBuffer;
            for(int i=0; i<dwLenBytes; i++)
            {
				sprintf((char*)hex, "%02X ", pData[i]);
				WriteFile(_hFile, hex, 3, &dwWriteLength, NULL);
			}
			WriteFile(_hFile, pCR, strlen(pCR), &dwWriteLength, NULL);
			
			FlushFileBuffers(_hFile);
		}
	}

	void Lock()  { ::EnterCriticalSection(&_csLock); }
	void Unlock() { ::LeaveCriticalSection(&_csLock); }
	
public:
	CLogFile(const TCHAR *szFileName = _T("Log.log"))//设定日志文件名
	{
		_szFileName = NULL;
		_hFile = INVALID_HANDLE_VALUE;
		::InitializeCriticalSection(&_csLock);
		SetFileName(szFileName);
	}

	virtual ~CLogFile()
	{
		::DeleteCriticalSection(&_csLock);
		Close();
		if(_szFileName)
			delete []_szFileName;
	}
	
	const TCHAR * GetFileName()
	{
		return _szFileName;
	}

	void SetFileName(const TCHAR *szName)//修改文件名， 同时关闭上一个日志文件
	{
		assert(szName);
		if(_szFileName)
			delete []_szFileName;
		Close();
		_szFileName = new TCHAR[_tcslen(szName) + 1];
		assert(_szFileName);
		_tcscpy(_szFileName, szName);     
	}

	bool IsOpen()
	{
		return _hFile != INVALID_HANDLE_VALUE;
	}

	void Close()
	{
		if(IsOpen())
		{
			CloseHandle(_hFile);
			_hFile = INVALID_HANDLE_VALUE;
		}
	}

	void Log(LPCVOID lpBuffer, DWORD dwLength)//追加日志内容
	{
		assert(lpBuffer);
		__try
		{
			Lock();
			if(!OpenFile())
				return;
			WriteLog(lpBuffer, dwLength);
		}
		__finally
		{
			Unlock();
		} 
	}

	void LogHex(LPCVOID lpBuffer, DWORD dwLength)//追加日志内容
	{
		assert(lpBuffer);
		__try
		{
			Lock();
			if(!OpenFile())
				return;
			WriteHexLog(lpBuffer, dwLength);
		}
		__finally
		{
			Unlock();
		} 
	}

	void Log(const char *szText)
	{
		Log(szText, strlen(szText));
	}

    void Log(const wchar_t *szText)
    {
        int len = wcslen(szText) * sizeof(wchar_t);
        char* pBuf = (char*)malloc(len);
        memset(pBuf, 0, len);
        w2a(szText, pBuf, len);
        Log(pBuf);
        free(pBuf);
    }

    void LogHex(const TCHAR *szText)
    {
        LogHex(szText, _tcslen(szText) * sizeof(TCHAR));
    }
private://屏蔽函数
	CLogFile(const CLogFile&);
	CLogFile&operator = (const CLogFile&);
};

class CLogFileEx : public CLogFile
{
protected:
	TCHAR *_szPath;
	TCHAR _szLastDate[9];
	int _iType;

	void SetPath(const TCHAR *szPath)
	{
		assert(szPath);
		WIN32_FIND_DATA wfd;
		TCHAR temp[MAX_PATH + 1] = {0};
		if(FindFirstFile(szPath, &wfd) == INVALID_HANDLE_VALUE && CreateDirectory(szPath, NULL) == 0)
		{
			_tcscat(_tcscpy(temp, szPath), _T(" Create Fail. Exit Now! Error ID :"));
			_ltot(GetLastError(), temp + _tcslen(temp), 10);
			MessageBox(NULL, temp, _T("Class CLogFileEx"), MB_OK);
			exit(1);
		}
		else
		{
			GetFullPathName(szPath, MAX_PATH, temp, NULL);
			_szPath = new TCHAR[_tcslen(temp) + 1];
			assert(_szPath);
			_tcscpy(_szPath, temp);
		}
	}
public:
	enum LOG_TYPE{YEAR = 0, MONTH = 1, DAY = 2};
	CLogFileEx(const TCHAR *szPath = _T("."), LOG_TYPE iType = MONTH)
	{
		_szPath = NULL;
		SetPath(szPath);
		_iType = iType;
		memset(_szLastDate, 0, 9);
	}

	~CLogFileEx()
	{
		if(_szPath)
			delete []_szPath;
	}

	const TCHAR * GetPath()
	{
		return _szPath;
	}

	void Log(LPCVOID lpBuffer, DWORD dwLength)
	{
		assert(lpBuffer);
		TCHAR temp[10];
		static const TCHAR format[3][10] = {_T("%Y"), _T("%Y-%m"), _T("%Y%m%d")};
		
		__try
		{
			Lock();
			
			time_t now = time(NULL);
			_tcsftime(temp, 9, format[_iType], localtime(&now));
			if(_tcscmp(_szLastDate, temp) != 0)//更换文件名
			{
				_tcscat(_tcscpy(_szFileName, _szPath), _T("//"));
				_tcscat(_tcscat(_szFileName, temp), _T(".log"));
				_tcscpy(_szLastDate, temp);
				Close();
			}
			if(!OpenFile())
				return;
			
			WriteLog(lpBuffer, dwLength);
		}
		__finally
		{
			Unlock();
		}
	}

private://屏蔽函数
	CLogFileEx(const CLogFileEx&);
	CLogFileEx&operator = (const CLogFileEx&);
};

#endif //_LOGFILE_H
