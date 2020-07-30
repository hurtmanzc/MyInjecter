// Utility.cpp: implementation of the CUtility class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utility.h"
#include <Tlhelp32.h>
#include <WinSock2.h>
#include <time.h>
#include <Psapi.h>
#include <locale.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

char *w2c(const wchar_t *pwstr, char *pcstr, int len)
{
	if(pcstr == NULL || pcstr == NULL)
		return NULL;
	memset(pcstr, 0, len);
	int nlength = wcslen(pwstr);
	int nbytes = ::WideCharToMultiByte(0,0,pwstr,nlength,NULL,0,NULL,NULL);
	if(nbytes>len)   
		nbytes = len;
	::WideCharToMultiByte(0,0,pwstr,nlength,pcstr,nbytes,NULL,NULL);
	return pcstr ;
}

void dbgprint(const char *format, ...)
{
	DWORD pid = 0;
	va_list vl;
	char dbgbuf1[2048] = {0};
	char dbgbuf2[2048] = {0};  
	pid = GetCurrentThreadId();
	va_start(vl, format);
	wvsprintfA(dbgbuf1, format, vl);
	wsprintfA(dbgbuf2, "%lu: %s\r\n", pid, dbgbuf1);
	va_end(vl); 
	OutputDebugStringA(dbgbuf2);
}

DWORD GetMainThread(DWORD dwProcessID)
{
	DWORD dwThreadID = 0;
	HANDLE hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
	THREADENTRY32 te32 = {sizeof(te32)};
	if( Thread32First( hThreadSnap, &te32) )
	{
		do
		{
			if( dwProcessID == te32.th32OwnerProcessID )
			{
				dwThreadID = te32.th32ThreadID;
				break;
			}
		}while( Thread32Next( hThreadSnap, &te32) );
	}
	return dwThreadID;
}

HWND GetWindowHandleByPid(DWORD dwProcessID, const char* sTitle)
{
    HWND hWnd = GetTopWindow(NULL);
    while (hWnd)
    {
        DWORD pid = 0;
        DWORD dwTheardId = GetWindowThreadProcessId(hWnd, &pid);		
        if (dwTheardId != 0)
        {
            if (pid == dwProcessID/*your process id*/)
            {
                // here hWnd is the handle to the window
				if(sTitle != NULL)
				{
                    char strTitle[MAX_PATH] = {0};
					::GetWindowTextA(hWnd, strTitle, MAX_PATH);
					if(strcmp(strTitle, sTitle) == 0)
						return hWnd;
				}
				else
				{
					return hWnd;
				}
            }
        }
        
        hWnd = GetNextWindow(hWnd, GW_HWNDNEXT);
    }
	
    return NULL;
}

int GetFileNameByHandle(HANDLE hFile, char* buff, DWORD size)
{
    HANDLE hfilemap = CreateFileMapping(hFile, NULL, PAGE_READWRITE, NULL, NULL, NULL);
    if(NULL == hfilemap || INVALID_HANDLE_VALUE == hfilemap)
    {
        printf("file mapping error");
        return 0;
    }

    LPVOID lpmap = MapViewOfFile(hfilemap, FILE_MAP_READ | FILE_MAP_WRITE, NULL, NULL, 0);
    if(NULL==lpmap)
    {
        printf("MapViewOfFile error:%d", GetLastError());
        return 0;
    }

    DWORD length = GetMappedFileNameA(GetCurrentProcess(), lpmap, buff, size);
    if(0 == length)
    {
        printf("get mapped file name error");
        return 0;
    }

    char DosPath[MAX_PATH];
    char DriverString[MAX_PATH];
    memset(DosPath, 0, MAX_PATH);
    memset(DriverString, 0, MAX_PATH);
    GetLogicalDriveStringsA(MAX_PATH, DriverString);
    char *p = (char*)DriverString;  //p����ָ���̷�
    do
    {
        *(p+2)='\0'; //����QuerDosDevice��һ������������c:�������͵ģ�������\�����Ұ��Ǹ�\��Ĩ����  
        QueryDosDeviceA(p, DosPath, MAX_PATH);
        char *q = strstr(buff, DosPath); //���buff���Ƿ���DosDevice�е�DosPath���еĻ���pָ����Ǹ��ִ�����Ҫ���̷�
        if(q != NULL)
        {
            //�ҵ�֮��Ӧ�ð�buff�����һ������\�ط����ִ����ƹ������̷����·��
            q = strrchr(buff,0x5c);
            //�ٰ�DriverString·���������ַ����㣬ֻ�����ҵ����̷�
            //memset(p+2, 0, MAX_PATH-2);
            strcat(p,q);  //����·��
            strcpy(buff,p);
            return 1;
        }

        p = p+4;  //ָ���ƶ���DriverString����һ���̷���
    }
    while(*p != 0);

    return 0;
}

// Wide String -> Ansi String
// ��������Ҫfree���ص��ַ���ָ��
char* WideStringToAnsiString(const wchar_t* strWide)
{
    if(!strWide || wcslen(strWide) == 0)
        return NULL;

    char* pOut = NULL;
    DWORD dwNum = ::WideCharToMultiByte(CP_ACP, NULL, strWide, -1, NULL, 0, NULL, FALSE);
    pOut = (char*)malloc(dwNum * sizeof(char));
    memset(pOut, 0, dwNum * sizeof(char));
    ::WideCharToMultiByte (CP_ACP, NULL, strWide, -1, pOut, dwNum, NULL, FALSE);

    return pOut;
}

// Ansi String -> Wide String
// ��������Ҫfree���ص��ַ���ָ��
wchar_t* AnsiStringToWideString(const char* strAnsi)
{
    if(!strAnsi || strlen(strAnsi) == 0)
        return NULL;

    wchar_t* pOut = NULL;
    setlocale(LC_ALL, "");//ʹ���Ŀ��Դ�ӡ��ʾ
    DWORD dwNum = ::MultiByteToWideChar (CP_ACP, 0, strAnsi, -1, NULL, 0);
    pOut = (wchar_t*)malloc(dwNum * sizeof(wchar_t));
    memset(pOut, 0, dwNum * sizeof(wchar_t));
    ::MultiByteToWideChar (CP_ACP, 0, strAnsi, -1, pOut, dwNum+10);

    return pOut;
}

// C prototype : void StrToHex(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - ���������
//	[IN] pSrc - �ַ���
//	[IN] nLen - 16���������ֽ���(�ַ����ĳ���/2)
// return value: 
// remarks : ���ַ���ת��Ϊ16������
void StrToHex(const char *pSrc, BYTE *pDest, int& nLen)
{
    char h1, h2;
    BYTE s1, s2;
    nLen = 0;
    int len = strlen(pSrc);
    for (int i=0; i<len; i++)
    {
        if(pSrc[i] == ' ') 
            continue;

        h1 = pSrc[i];
        h2 = pSrc[i+1];
        i++;

        s1 = toupper(h1) - 0x30;
        if (s1 > 9) 
            s1 -= 7;

        s2 = toupper(h2) - 0x30;
        if (s2 > 9) 
            s2 -= 7;

        pDest[len] = s1*16 + s2;
        nLen++;
    }
}

// C prototype : void HexToStr(BYTE *pbDest, BYTE *pbSrc, int nLen)
// parameter(s): [OUT] pbDest - ���Ŀ���ַ���
//	[IN] pbSrc - ����16����������ʼ��ַ
//	[IN] nLen - 16���������ֽ���
// return value: 
// remarks : ��16������ת��Ϊ�ַ���
void HexToStr(BYTE *pSrc, int nLen, char *pDest, BOOL bWithSpace/* = TRUE*/)
{
    if(bWithSpace)
    {
        //��� 5C 60 EB 89 3D 00 ���ո���ʽ 
        char ddl,ddh;
        for(int i=0; i<nLen; i++)
        {
            ddh = 48 + pSrc[i] / 16;
            ddl = 48 + pSrc[i] % 16;
            if (ddh > 57) ddh = ddh + 7;
            if (ddl > 57) ddl = ddl + 7;
            pDest[i*2+i] = ddh;
            pDest[i*2+i+1] = ddl;
            pDest[i*2+i+2] = ' ';
        }
        pDest[nLen*2+nLen-1] = '\0';
    }
    else
    {
        //��� 5C60EB893D00 �����ո���ʽ 
        char ddl,ddh;
        for(int i=0; i<nLen; i++)
        {
            ddh = 48 + pSrc[i] / 16;
            ddl = 48 + pSrc[i] % 16;
            if (ddh > 57) ddh = ddh + 7;
            if (ddl > 57) ddl = ddl + 7;
            pDest[i*2] = ddh;
            pDest[i*2+1] = ddl;
        }
        pDest[nLen*2] = '\0';
    }
}

// typedef struct
// {
// 	HWND hWnd;
// 	DWORD dwPid;
// } WNDINFO;
// 
// BOOL CALLBACK EnumWndProc(HWND hWnd, LPARAM lParam)
// {
// 	WNDINFO* pInfo = (WNDINFO*)lParam;
// 	DWORD dwProcessId = 0;
// 	GetWindowThreadProcessId(hWnd, &dwProcessId);
// 	
// 	if(dwProcessId == pInfo->dwPid)
// 	{
// 		pInfo->hWnd = hWnd;
// 		return FALSE;
// 	}
// 	return TRUE;
// }
// 
// HWND GetHwndByProcessId(DWORD dwProcessId)
// {
// 	WNDINFO info = {0};
// 	info.hWnd = NULL;
// 	info.dwPid = dwProcessId;
// 	EnumWindows(EnumWndProc, (LPARAM)&info);
// 	return info.hWnd;
// }

// struct   NTP_Packet  
// {  
//     int      Control_Word;     
//     int      root_delay;     
//     int      root_dispersion;     
//     int      reference_identifier;     
//     __int64 reference_timestamp;     
//     __int64 originate_timestamp;     
//     __int64 receive_timestamp;     
//     int      transmit_timestamp_seconds;     
//     int      transmit_timestamp_fractions;     
// }; 
// 
// /************************************************************************/  
// /* ����˵��:�Զ���ʱ�������ͬ������ 
// /* ����˵��:�� 
// /* �� �� ֵ:�ɹ�����TRUE��ʧ�ܷ���FALSE 
// /************************************************************************/  
// BOOL GetSysTime(SYSTEMTIME& sysTime)  
// {
//     WORD    wVersionRequested;  
//     WSADATA wsaData;
//     int nRtn = 0;
// 	
//     // ��ʼ���汾   
//     wVersionRequested = MAKEWORD(1, 1);  
//     if (0 != WSAStartup(wVersionRequested, &wsaData))   
//     {  
//         WSACleanup();  
//         return FALSE;  
//     } 
// 	
//     if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)   
//     {  
//         WSACleanup( );  
//         return FALSE;   
//     }  
// 	
//     // ���IP���й���½ʱ��ͬ����������ַ���������޸�   
//     SOCKET sock = socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);  
//     struct sockaddr_in addrSrv;  
//     addrSrv.sin_addr.S_un.S_addr = inet_addr("210.72.145.44");  
//     addrSrv.sin_family = AF_INET;  
//     addrSrv.sin_port = htons(123);  
// 	
//     NTP_Packet NTP_Send,NTP_Recv;   
//     NTP_Send.Control_Word = htonl(0x0B000000);   
//     NTP_Send.root_delay = 0;   
//     NTP_Send.root_dispersion = 0;     
//     NTP_Send.reference_identifier = 0;     
//     NTP_Send.reference_timestamp = 0;     
//     NTP_Send.originate_timestamp = 0;     
//     NTP_Send.receive_timestamp = 0;     
//     NTP_Send.transmit_timestamp_seconds = 0;     
//     NTP_Send.transmit_timestamp_fractions = 0;   
//     
//     nRtn = sendto(sock, (const char*)&NTP_Send, sizeof(NTP_Send),  
//         0, (struct sockaddr*)&addrSrv, sizeof(addrSrv));
//     if(SOCKET_ERROR == nRtn)  
//     {  
//         closesocket(sock);  
//         return FALSE;  
//     }  
//     int sockaddr_Size = sizeof(addrSrv);  
//     nRtn = recvfrom(sock,(char*)&NTP_Recv,sizeof(NTP_Recv),  
//         0,(struct sockaddr*)&addrSrv,&sockaddr_Size);
//     if(SOCKET_ERROR == nRtn)  
//     {  
//         closesocket(sock);  
//         return FALSE;  
//     }  
//     closesocket(sock);  
//     WSACleanup();  
// 	
//     //SYSTEMTIME  sysTime;  
//     float       Splitseconds;  
//     struct      tm  *lpLocalTime;  
//     time_t      ntp_time;  
// 	
//     // ��ȡʱ���������ʱ��   
//     ntp_time    = ntohl(NTP_Recv.transmit_timestamp_seconds)-2208988800;  
//     lpLocalTime = localtime(&ntp_time);  
//     if(lpLocalTime == NULL)  
//     {  
//         return FALSE;  
//     }  
// 	
//     // ��ȡ�µ�ʱ��   
//     sysTime.wYear      = lpLocalTime->tm_year+1900;  
//     sysTime.wMonth     = lpLocalTime->tm_mon+1;  
//     sysTime.wDayOfWeek = lpLocalTime->tm_wday;  
//     sysTime.wDay       = lpLocalTime->tm_mday;  
//     sysTime.wHour      = lpLocalTime->tm_hour;  
//     sysTime.wMinute    = lpLocalTime->tm_min;  
//     sysTime.wSecond    = lpLocalTime->tm_sec;  
// 	
//     // ����ʱ�侫��   
//     Splitseconds = (float)ntohl(NTP_Recv.transmit_timestamp_fractions);  
//     Splitseconds = (float)0.000000000200 * Splitseconds;  
//     Splitseconds = (float)1000.0 * Splitseconds;  
//     sysTime.wMilliseconds = (unsigned short)Splitseconds;  
// 	
//     // �޸ı���ϵͳʱ��   
//     //SetLocalTime(&newtime);  
//     return TRUE;  
// } 