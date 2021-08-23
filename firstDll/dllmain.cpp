// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <tchar.h>
#include <atlstr.h>
#include <iostream>
#include <Windows.h>
#include "detours.h"
#pragma comment (lib,"detours.lib")

const int MAXSIZE = 20000;
HANDLE hFile = NULL;    //日志文件
LPVOID* allocAddr = new LPVOID[MAXSIZE]; //存储分配的堆内存地址
int* allocSize = new int[MAXSIZE];   //对应分配的堆内存大小
int index = 0;

//写入日志文件
void writeToLog(LPCWSTR msg) {
    DWORD msgByte = wcslen(msg) * sizeof(TCHAR);
    DWORD dwWrittenSize = 0;
    WriteFile(hFile, msg, msgByte, &dwWrittenSize, NULL);
    FlushFileBuffers(hFile);
}

//保存原函数
LPVOID(WINAPI* OldHeapAlloc)(
    HANDLE, DWORD, SIZE_T) = HeapAlloc;
BOOL(WINAPI* OldHeapFree)(
    HANDLE, DWORD, LPVOID) = HeapFree;
//拦截后执行的函数
LPVOID WINAPI NewHeapAlloc(
    HANDLE hHeap,
    DWORD  dwFlags,
    SIZE_T dwBytes) {
    LPVOID record_ptr = OldHeapAlloc(hHeap, dwFlags, dwBytes);
    while (allocSize[index] > 0) {
        index = (index + 1) % MAXSIZE;
    }
    allocAddr[index] = record_ptr;
    allocSize[index] = dwBytes;
    index = (index + 1) % MAXSIZE;
    return record_ptr;
}
BOOL NewHeapFree(
    HANDLE hHeap,
    DWORD dwFlags,
    LPVOID lpMem) {
    for (int i = 0; i < index; i++) {
        if (allocAddr[i] == lpMem) {
            allocAddr[i] = NULL;
            allocSize[i] = 0;
        }
    }
    return OldHeapFree(hHeap, dwFlags, lpMem);
}
//下钩子
void StartHook() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)OldHeapAlloc, NewHeapAlloc);
    DetourAttach(&(PVOID&)OldHeapFree, NewHeapFree);
    DetourTransactionCommit();
}
//撤钩子
void EndHook() {
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourDetach(&(PVOID&)OldHeapAlloc, NewHeapAlloc);
    DetourDetach(&(PVOID&)OldHeapFree, NewHeapFree);
    DetourTransactionCommit();
}

void work() {
    hFile = CreateFile(
        L"workLog.txt",
        GENERIC_WRITE,
        FILE_SHARE_WRITE,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );
    LPCWSTR beginMsg = L"[#]Dll start working.\n";
    writeToLog(beginMsg);
    StartHook();
}

extern "C" __declspec(dllexport) DWORD WINAPI startFunc(LPVOID lParam) {
    int aa = MessageBox(NULL,
        L"Dll线程开始执行",
        L"通知",
        MB_OK);
    work();
    return aa;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    HANDLE hThread = NULL;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        hThread = CreateThread(NULL, 0, startFunc, NULL, 0, NULL);
        CloseHandle(hThread);
        break;
    case DLL_PROCESS_DETACH:
        EndHook();
        writeToLog(L"[#]Dll end working.\n");
        writeToLog(L"[#]Heap not free:\n");
        for (int i = 0; i < MAXSIZE; i++) {
            if (allocSize[i] < 0) {
                break;
            }
            if (allocSize[i] != 0) {
                writeToLog(L"---Addr: ");
                const char* add = reinterpret_cast<const char*>(&allocAddr[i]);
                CString _addr;
                _addr.Format(L"%p", add);
                writeToLog((LPCWSTR)_addr);
                writeToLog(L" Size: ");
                CString _size;
                _size.Format(L"%d", allocSize[i]);
                writeToLog((LPCWSTR)_size);
                writeToLog(L"\n");
            }
        }
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}

