#include "admin.h"
#include <cstdio>
#include <cstdlib>

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
  char exePath[MAX_PATH] = { 0 };
  if (!GetModuleFileNameA(nullptr, exePath, MAX_PATH)) {
    exit(EXIT_FAILURE);
  }

  char workingDir[MAX_PATH] = { 0 };
  if (!GetCurrentDirectoryA(MAX_PATH, workingDir)) {
    exit(EXIT_FAILURE);
  }

  SHELLEXECUTEINFOA sei = { };
  sei.cbSize = sizeof(sei);
  sei.fMask = SEE_MASK_DEFAULT;
  sei.lpVerb = "runas";
  sei.lpFile = exePath;
  sei.lpDirectory = workingDir;
  sei.nShow = SW_SHOWNORMAL;

  if (ShellExecuteExA(&sei)) {
    exit(EXIT_SUCCESS);
  }
#endif

  exit(EXIT_FAILURE);
}


void EnsureRunningAsAdmin() {
#ifdef _WIN32
  if (!IsRunningAsAdmin()) {
    printf("Not running as admin, relaunching with admin privs...\n");
    RelaunchAsAdmin();
  }
#endif
}


} // namespace admin
