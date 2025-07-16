// game-watcher.cpp
//
// Detects running games by scanning the process list (Windows & Linux).
// Tracks all matching processes (supports multiple instances of the same game).
// Triggers Lua events individually for each process:
// - Game start when a matching process is found
// - Game stop when a process terminates
//
// Additionally on Windows:
// - Detects if any tracked game window is in the foreground
// - Alternatively considers any fullscreen window as foreground
// - Foreground/background transitions trigger Lua events

#include "game-watcher.h"
#include "games.h"
#include "lua.h"
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#include <cstring>
#else
#include <dirent.h>
#include <unistd.h>
#include <fstream>
#endif

namespace gamewatcher {

struct ProcessInfo {
  int pid;
  const games::Game* game;
};

static std::vector<ProcessInfo> tracked;
static bool isForeground = false;

#ifdef _WIN32
static bool IsAnyFullscreen() {
  HWND hwnd = GetForegroundWindow();
  if (!hwnd) return false;

  RECT windowRect;
  if (!GetWindowRect(hwnd, &windowRect)) return false;
  HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
  MONITORINFO mi;
  std::memset(&mi, 0, sizeof(mi));
  mi.cbSize = sizeof(mi);
  if (!GetMonitorInfo(monitor, &mi)) return false;

  return windowRect.left <= mi.rcMonitor.left &&
         windowRect.top <= mi.rcMonitor.top &&
         windowRect.right >= mi.rcMonitor.right &&
         windowRect.bottom >= mi.rcMonitor.bottom;
}
#endif

void ResetState() {
  for (const auto& proc : tracked) {
    lua::TriggerGameStop(proc.pid, proc.game->name, proc.game->binary);
  }
  tracked.clear();
  isForeground = false;
}

static bool IsAlreadyTracked(int pid) {
  for (const auto& proc : tracked) {
    if (proc.pid == pid) return true;
  }
  return false;
}

void Process() {
  std::vector<ProcessInfo> found;

#ifdef _WIN32
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE) return;

  PROCESSENTRY32 entry = {};
  entry.dwSize = sizeof(PROCESSENTRY32);

  if (Process32First(snapshot, &entry)) {
    do {
      const games::Game* game = games::GetGameByBinary(entry.szExeFile, true);
      if (game) {
        int pid = static_cast<int>(entry.th32ProcessID);
        found.push_back({ pid, game });
        if (!IsAlreadyTracked(pid)) {
          tracked.push_back({ pid, game });
          lua::TriggerGameStart(pid, game->name, game->binary);
        }
      }
    } while (Process32Next(snapshot, &entry));
  }

  CloseHandle(snapshot);

#else
  DIR* dir = opendir("/proc");
  if (!dir) return;

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    if (entry->d_type != DT_DIR) continue;

    std::string pidStr = entry->d_name;
    if (pidStr.find_first_not_of("0123456789") != std::string::npos) continue;

    std::string cmdPath = "/proc/" + pidStr + "/comm";
    std::ifstream cmdFile(cmdPath);
    if (!cmdFile.is_open()) continue;

    std::string exeName;
    std::getline(cmdFile, exeName);
    cmdFile.close();

    const games::Game* game = games::GetGameByBinary(exeName, true);
    if (game) {
      int pid = std::stoi(pidStr);
      found.push_back({ pid, game });
      if (!IsAlreadyTracked(pid)) {
        tracked.push_back({ pid, game });
        lua::TriggerGameStart(pid, game->name, game->binary);
      }
    }
  }

  closedir(dir);
#endif

  for (auto it = tracked.begin(); it != tracked.end();) {
    bool stillRunning = false;
    for (const auto& proc : found) {
      if (proc.pid == it->pid) {
        stillRunning = true;
        break;
      }
    }
    if (!stillRunning) {
      lua::TriggerGameStop(it->pid, it->game->name, it->game->binary);
      it = tracked.erase(it);
    } else {
      ++it;
    }
  }

#ifdef _WIN32
  // Foreground detection
  HWND foreground = GetForegroundWindow();
  if (foreground) {
    DWORD pid = 0;
    GetWindowThreadProcessId(foreground, &pid);

    bool gameWindowActive = false;
    for (const auto& proc : tracked) {
      if (proc.pid == static_cast<int>(pid)) {
        gameWindowActive = true;
        break;
      }
    }

    bool fullscreenActive = IsAnyFullscreen();
    bool nowForeground = gameWindowActive || fullscreenActive;

    if (nowForeground && !isForeground) {
      for (const auto& proc : tracked) {
        lua::TriggerGameForeground(proc.pid, proc.game->name, proc.game->binary);
      }
    }
    if (!nowForeground && isForeground) {
      for (const auto& proc : tracked) {
        lua::TriggerGameBackground(proc.pid, proc.game->name, proc.game->binary);
      }
    }
    isForeground = nowForeground;
  }
#endif
}

} // namespace gamewatcher
