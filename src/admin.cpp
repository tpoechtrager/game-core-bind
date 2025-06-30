#include "admin.h"
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#else
#include <unistd.h>
#endif

namespace admin {

bool IsRunningAsAdmin() {
#ifdef _WIN32
  BOOL isAdmin = FALSE;
  HANDLE token = nullptr;
  if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
    TOKEN_ELEVATION elevation;
    DWORD size = sizeof(elevation);
    if (GetTokenInformation(token, TokenElevation, &elevation, sizeof(elevation), &size)) {
      isAdmin = elevation.TokenIsElevated;
    }
    CloseHandle(token);
  }
  return isAdmin;
#else
  return geteuid() == 0;
#endif
}

void RelaunchAsAdmin() {
#ifdef _WIN32
  char exePath[MAX_PATH];
  if (GetModuleFileNameA(nullptr, exePath, MAX_PATH)) {
    SHELLEXECUTEINFOA sei = { };
    sei.cbSize = sizeof(sei);
    sei.fMask = 0;
    sei.lpVerb = "runas";
    sei.lpFile = exePath;
    sei.nShow = SW_SHOWNORMAL;
    if (ShellExecuteExA(&sei)) {
      std::exit(0);
    }
  }
#endif
}

void EnsureRunningAsAdmin() {
#ifdef _WIN32
  if (!IsRunningAsAdmin()) {
    printf("Not running as admin, relaunching with admin privs...\n");
    RelaunchAsAdmin();
    std::exit(0);
  }
#endif
}


} // namespace admin
