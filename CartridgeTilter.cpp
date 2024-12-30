#include "CartridgeTilter.h"
// Define a typedef for the original CreateFile function
typedef HANDLE(WINAPI* CreateFile_t)(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);

typedef HANDLE(WINAPI* CreateFileW_t)(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile);

// Pointer to the original CreateFile function
CreateFile_t OriginalCreateFile = nullptr;

std::vector<std::string> regexes = {
    ".*"
};

bool loadingMap = false;
// Hook function
HANDLE WINAPI HookedCreateFile(
    LPCSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile)
{
    // Log the intercepted call
    std::string str = std::string(lpFileName);
    for (const auto& reg : regexes) {
        if (regexMatch(str, reg)) {
            std::cerr << "CreateFileA - " << lpFileName << std::endl;
            lpFileName = "D:\\eadBeef";
            break;
        }
    }
    return OriginalCreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

CreateFileW_t OriginalCreateFileW = nullptr;
HANDLE WINAPI HookedCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile)
{
    // Log the intercepted call
    std::string str = std::string(CW2A(lpFileName));
    for (const auto& reg : regexes) {
        if (regexMatch(str, reg)) {
            std::cerr << "CreateFileW - " << CW2A(lpFileName) << std::endl;
            lpFileName = L"D:\\eadBeef";
            break;
        }
    }
    return OriginalCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    DisableThreadLibraryCalls(hinstDLL);
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        // Initialize MinHook
        if (MH_Initialize() != MH_OK) {
            std::cerr << "Failed to initialize MinHook!" << std::endl;
            return 1;
        }
        std::cerr << "Init minhook" << std::endl;

        // Create a hook for CreateFile
        if (MH_CreateHook(&CreateFileA, &HookedCreateFile, reinterpret_cast<LPVOID*>(&OriginalCreateFile)) != MH_OK) {
            std::cerr << "Failed to create hook!" << std::endl;
            MH_Uninitialize();
            return 1;
        }
        if (MH_CreateHook(&CreateFileW, &HookedCreateFileW, reinterpret_cast<LPVOID*>(&OriginalCreateFileW)) != MH_OK) {
            std::cerr << "Failed to create hook! W" << std::endl;
            MH_Uninitialize();
            return 1;
        }
        std::cerr << "Init hook" << std::endl;

        // Enable the hook
        if (MH_EnableHook(&CreateFileA) != MH_OK) {
            std::cerr << "Failed to enable hook!" << std::endl;
            MH_Uninitialize();
            return 1;
        }
        if (MH_EnableHook(&CreateFileW) != MH_OK) {
            std::cerr << "Failed to enable hook! W" << std::endl;
            MH_Uninitialize();
            return 1;
        }
        std::cerr << "Hook enabled" << std::endl;
    }

    return TRUE;
}

//int main() {
//    getchar();
//
//    // Disable the hook and cleanup
//    MH_DisableHook(&CreateFileA);
//    MH_Uninitialize();
//
//    return 0;
//}