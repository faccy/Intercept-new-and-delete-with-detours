//InjectDll.cpp
#include "windows.h"
#include "tchar.h"
#include "tlhelp32.h"
#include <iostream>
using namespace std;

//PCWSTR dllName = L"C:/Users/95195/source/repos/firstDll/x64/Debug/firstDll.dll";
PCWSTR dllName = L"firstDll.dll";

//ÊµÏÖ×¢Èë
BOOL InjectDll(DWORD dwPID) {
	TCHAR lpdllpath[MAX_PATH];
	GetFullPathName(dllName, MAX_PATH, lpdllpath, NULL);
	HANDLE hProcess = NULL;
	HANDLE hThread = NULL;
	HMODULE hMod = NULL;
	LPVOID pMemory = NULL;
	DWORD dwDllSize = (DWORD)(wcslen(lpdllpath) * sizeof(TCHAR));
	LPTHREAD_START_ROUTINE pThreadProc;

	hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, dwPID);
	if (hProcess == NULL) {
		_tprintf(L"[-]OpenProcess(%d) failed!!! [%d]\n", dwPID, GetLastError());
		return FALSE;
	}
	hMod = GetModuleHandle(L"Kernel32.dll");
	pThreadProc = (LPTHREAD_START_ROUTINE)GetProcAddress(hMod, "LoadLibraryW");
	pMemory = VirtualAllocEx(hProcess, NULL, dwDllSize,
		MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(hProcess, pMemory, lpdllpath, dwDllSize, NULL);
	hThread = CreateRemoteThread(hProcess, NULL, 0, pThreadProc, pMemory, 0, NULL);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	CloseHandle(hProcess);
	//VirtualFreeEx(hProcess, pMemory, 0, MEM_RELEASE);
	return TRUE;
}

int _tmain(int argc, TCHAR* argv[]) {
	if (argc != 2) {
		_tprintf(L"[*]Usage : %s <pid>\n", argv[0]);
		return 1;
	}
	if (InjectDll((DWORD)_tstol(argv[1]))) {
		_tprintf(L"[+]Inject DLL success!!!\n");
	}
	else {
		_tprintf(L"[-]Inject DLL failed!!!\n");
	}
	return 0;
}