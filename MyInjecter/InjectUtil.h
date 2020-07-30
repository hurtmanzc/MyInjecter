#pragma once

struct NtCreateThreadExBuffer 
{
	ULONG Size;
	ULONG Unknown1;
	ULONG Unknown2;
	PULONG Unknown3;
	ULONG Unknown4;
	ULONG Unknown5;
	ULONG Unknown6;
	PULONG Unknown7;
	ULONG Unknown8;
};

typedef DWORD (WINAPI *PTHREAD_START_ROUTINE)(
	LPVOID lpThreadParameter
	);

enum OSType
{
	Unknown = 0,
	WindowsXP = 50,
	WindowsVista = 60,
	Windows7 = 61,
	Windows10 = 62
};

OSType CheckOS();
BOOL InjectDll4Win10(DWORD dwProcessId, LPCTSTR szDllPath);

