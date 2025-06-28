#include "tools.h"
#include <filesystem>
#include <iostream>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <limits.h>
#include <cerrno>
#include <cstring>
#endif

namespace fs = std::filesystem;

namespace tools {

std::time_t GetFileTimestamp(const std::string& path) {
  try {
    auto ftime = fs::last_write_time(path);
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
      ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
    return std::chrono::system_clock::to_time_t(sctp);
  } catch (...) {
    return 0;
  }
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
