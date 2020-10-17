#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

inline DWORD GetProcessIdByName(const char* name) {
    PVOID snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 process;
    process.dwSize = sizeof(process);

    DWORD pid = 0;
    while (Process32Next(snapshot, &process)) {
        if (strcmp(process.szExeFile, name) == 0) {
            pid = process.th32ProcessID;
            break;
        }
    }

    CloseHandle(snapshot);
    return pid;
}

inline bool GetFileExtFromDir(const fs::path& dir, const fs::path ext, fs::path& file)
{
    for (const auto& entry : fs::directory_iterator(dir))
    {
        if (!entry.is_regular_file()) continue;
        const fs::path currentFile = entry.path();
        if (currentFile.extension() == ext.extension()) 
        {
            file = currentFile;
            return true;
        }
    }
    return false;
}

inline bool RemoteInject(const HANDLE& process, const std::wstring& mod)
{
    auto len = mod.length() * sizeof(wchar_t);
    LPVOID alloc = VirtualAllocEx(process, 0, len, MEM_COMMIT, PAGE_READWRITE);
    if (!alloc) return false;
    if (!WriteProcessMemory(process, alloc, mod.data(), len, 0)) return false;
    auto thread = CreateRemoteThread(process, 0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryW), alloc, 0, 0);
    if (!thread) return false;
    WaitForSingleObject(thread, INFINITE);
    VirtualFreeEx(process, alloc, len, MEM_RESERVE);
    return true;
}

int main(int argc, const char* argv[]) {
    const DWORD pid = GetProcessIdByName("SoTGame.exe");
    if (!pid)
    {
        printf("Game process not found\n");
        return 1;
    }
    const HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
    if (process == INVALID_HANDLE_VALUE)
    {
        printf("Couldn't get process handle\n");
        return 1;
    }
    const fs::path workingDir = fs::path(argv[0]).remove_filename();
    fs::path library;
    GetFileExtFromDir(workingDir, "filename.dll", library);
    if (!RemoteInject(process, library.wstring()))
    {
        printf("Couldn't inject module\n");
        return 1;
    };
    printf("Successfully injected!\n");

    return 0;
  
}