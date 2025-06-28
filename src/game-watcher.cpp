// game-watcher.cpp
//
// Detects running games by scanning the process list (Windows & Linux).
// Triggers Lua events when a known game starts or stops.
// Additionally detects:
// - If the recognized game is in the foreground (active window or any fullscreen application active)
// - Foreground/background transitions trigger Lua events.
//
// Only one game is monitored at a time. Once detected, its state remains tracked until the game stops.

#include "game-watcher.h"
#include "games.h"
#include "lua.h"
#include <string>

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <fstream>
#endif

namespace gamewatcher {

bool GameRunning = false;
static bool wasRunning = false;
static const games::Game* activeGame = nullptr;
static int activePid = 0;
static bool isForeground = false;

void ResetState() {
  if (GameRunning && activeGame) {
    lua::TriggerGameStop(activePid, activeGame->name, activeGame->binary);
  }
  GameRunning = false;
  activeGame = nullptr;
  activePid = 0;
  wasRunning = false;
  isForeground = false;
}

static bool CheckProcess(const std::string& exeName, int pid) {
  const games::Game* game = games::GetGameByBinary(exeName);
  if (game && !GameRunning) {
    GameRunning = true;
    activeGame = game;
    activePid = pid;
    return true;
  }
  return false;
}

#ifdef _WIN32
static bool IsAnyFullscreen() {
  HWND hwnd = GetForegroundWindow();
  if (!hwnd) return false;

  RECT windowRect;
  if (!GetWindowRect(hwnd, &windowRect)) return false;

  HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);
  MONITORINFO mi = { sizeof(mi) };
  if (!GetMonitorInfo(monitor, &mi)) return false;

  return windowRect.left <= mi.rcMonitor.left &&
         windowRect.top <= mi.rcMonitor.top &&
         windowRect.right >= mi.rcMonitor.right &&
         windowRect.bottom >= mi.rcMonitor.bottom;
}
#endif

void Process() {
  bool runningBefore = GameRunning;
  const games::Game* gameBefore = activeGame;
  int pidBefore = activePid;

  GameRunning = false;
  activeGame = nullptr;
  activePid = 0;

#ifdef _WIN32
  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE) {
    return;
  }

  PROCESSENTRY32 entry = { };
  entry.dwSize = sizeof(PROCESSENTRY32);

  if (Process32First(snapshot, &entry)) {
    do {
      if (CheckProcess(entry.szExeFile, static_cast<int>(entry.th32ProcessID))) {
        break;
      }
    } while (Process32Next(snapshot, &entry));
  }

  CloseHandle(snapshot);

#else
  DIR* dir = opendir("/proc");
  if (!dir) {
    return;
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr) {
    if (entry->d_type != DT_DIR) {
      continue;
    }

    std::string pidStr = entry->d_name;
    if (pidStr.find_first_not_of("0123456789") != std::string::npos) {
      continue;
    }

    std::string cmdPath = "/proc/" + pidStr + "/comm";
    std::ifstream cmdFile(cmdPath);
    if (!cmdFile.is_open()) {
      continue;
    }

    std::string exeName;
    std::getline(cmdFile, exeName);
    cmdFile.close();

    if (CheckProcess(exeName, std::stoi(pidStr))) {
      closedir(dir);
      break;
    }
  }

  closedir(dir);
#endif

  if (GameRunning && !runningBefore) {
    if (activeGame) {
      lua::TriggerGameStart(activePid, activeGame->name, activeGame->binary);

#ifdef _WIN32
      HWND foreground = GetForegroundWindow();
      if (foreground) {
        DWORD pid = 0;
        GetWindowThreadProcessId(foreground, &pid);
        isForeground = (static_cast<int>(pid) == activePid) || IsAnyFullscreen();
        if (isForeground) {
          lua::TriggerGameForeground(activePid, activeGame->name, activeGame->binary);
        }
      }
#endif
    }
  }

  if (!GameRunning && runningBefore) {
    if (gameBefore) {
      lua::TriggerGameStop(pidBefore, gameBefore->name, gameBefore->binary);
    }
  }

#ifdef _WIN32
  if (GameRunning && activePid) {
    HWND foreground = GetForegroundWindow();
    if (foreground) {
      DWORD pid = 0;
      GetWindowThreadProcessId(foreground, &pid);

      bool gameWindowActive = (static_cast<int>(pid) == activePid);
      bool fullscreenActive = IsAnyFullscreen();
      bool nowForeground = gameWindowActive || fullscreenActive;

      if (nowForeground && !isForeground) {
        lua::TriggerGameForeground(activePid, activeGame->name, activeGame->binary);
      }
      if (!nowForeground && isForeground) {
        lua::TriggerGameBackground(activePid, activeGame->name, activeGame->binary);
      }
      isForeground = nowForeground;
    }
  }
#endif

  wasRunning = GameRunning;
}

} // namespace gamewatcher
