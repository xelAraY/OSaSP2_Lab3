#include <windows.h>
#include <string.h>
#include <stdio.h>

#define DLL_NAME L"virt.dll"

char* srcStr = "SomeStr";
char* resStr = "StrSome";

typedef struct _Data {
    char* srcStr;
    char* resStr;
} Data;

int aes = 4096;

PVOID AllocExample() {
    PVOID ptrMem = VirtualAlloc(NULL, aes, MEM_COMMIT, PAGE_READWRITE);
    memcpy(ptrMem, srcStr, strlen(srcStr) + 1);
    printf("Start: %s\n", ptrMem);
    return ptrMem;
}

int main()
{
    PVOID exPtr = AllocExample();

    int pid = 0;

    pid = getpid();

    HANDLE procñHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    LPVOID loadLibraryPtr = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");

    LPVOID allocForStrings = VirtualAllocEx(procñHandle, NULL, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    WriteProcessMemory(procñHandle, allocForStrings, DLL_NAME, strlen(DLL_NAME) + 1, NULL);

    LPVOID paramPtr = VirtualAllocEx(procñHandle, NULL, strlen(DLL_NAME) + 1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    WriteProcessMemory(procñHandle, paramPtr, &allocForStrings, sizeof(LPVOID), NULL);

    HANDLE threadID = CreateRemoteThread(procñHandle, NULL, 0, loadLibraryPtr, paramPtr, 0, NULL);
    WaitForSingleObject(threadID, INFINITE);
    CloseHandle(threadID);

    LPVOID funcPtr = GetProcAddress(LoadLibrary(DLL_NAME), "ReplaceString");

    int paramSize = strlen(srcStr) + strlen(resStr) + 2;

    char* buff = malloc(paramSize);
    memcpy(buff, srcStr, strlen(srcStr) + 1);
    memcpy(buff + strlen(srcStr) + 1, resStr, strlen(resStr) + 1);

    WriteProcessMemory(procñHandle, allocForStrings, buff, paramSize, NULL);

    Data params;

    params.srcStr = (unsigned long long int)allocForStrings;
    params.resStr = (unsigned long long int)allocForStrings + strlen(srcStr) + 1;

    LPVOID paramsPtr = VirtualAllocEx(procñHandle, NULL, sizeof(&params), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    WriteProcessMemory(procñHandle, paramsPtr, &params, sizeof(params), NULL);


    threadID = CreateRemoteThread(procñHandle, NULL, 0, funcPtr, paramsPtr, 0, NULL);

    WaitForSingleObject(threadID, INFINITE);
    CloseHandle(threadID);
    CloseHandle(procñHandle);

    free(buff);

    Sleep(2000);

    return 0;
}