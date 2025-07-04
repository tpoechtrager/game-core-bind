#include "tools.h"
#include <filesystem>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#include <cerrno>
#include <cstring>
#endif
namespace tools {

void RunDetached(const std::string& exe, const std::string& args) {
#ifdef _WIN32
  std::string cmd = "\"" + exe + "\" " + args;

  STARTUPINFOA si = { sizeof(si) };
  PROCESS_INFORMATION pi = { 0 };

  if (CreateProcessA(nullptr, (LPSTR)cmd.c_str(), nullptr, nullptr, FALSE,
                     CREATE_NO_WINDOW | DETACHED_PROCESS, nullptr, nullptr, &si, &pi)) {
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
  }
#else
  std::string fullCmd = exe + " " + args + " &";
  system(fullCmd.c_str());
#endif
}

std::time_t GetFileTimestamp(const std::string& path) {
  std::error_code ec;
  auto ftime = std::filesystem::last_write_time(path, ec);
  if (ec) {
    return 0;
  }

  auto s = std::chrono::time_point_cast<std::chrono::seconds>(ftime).time_since_epoch();
  return s.count();
}

void SetWorkingDirToExePath() {
#ifdef _WIN32
  char path[MAX_PATH];
  if (GetModuleFileNameA(nullptr, path, MAX_PATH) > 0) {
    std::string exePath(path);
    auto pos = exePath.find_last_of("\\/");
    if (pos != std::string::npos) {
      exePath = exePath.substr(0, pos);
      if (!SetCurrentDirectoryA(exePath.c_str())) {
        std::cerr << "Failed to set working directory to: " << exePath << "\n";
      }
    } else {
      std::cerr << "Failed to parse executable path.\n";
    }
  } else {
    std::cerr << "GetModuleFileNameA failed.\n";
  }
#else
  char path[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", path, sizeof(path));
  if (count != -1) {
    std::string exePath(path, count);
    auto pos = exePath.find_last_of("/");
    if (pos != std::string::npos) {
      exePath = exePath.substr(0, pos);
      if (chdir(exePath.c_str()) != 0) {
        std::cerr << "Failed to set working directory to: " << exePath
                  << " (" << std::strerror(errno) << ")\n";
      }
    } else {
      std::cerr << "Failed to parse executable path.\n";
    }
  } else {
    std::cerr << "readlink failed (" << std::strerror(errno) << ")\n";
  }
#endif
}

void SleepMs(int ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

} // namespace tools
