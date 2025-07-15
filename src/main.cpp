#include "lua.h"
#include "lua-bindings.h"
#include "game-watcher.h"
#include "tools.h"
#include "network.h"
#include "admin.h"
#include <thread>
#include <chrono>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdio>
#include "tray.h"
#include "window.h"

bool shutdownRequest = false;
bool restartRequest = false;
bool restartAsAdminRequest = false;

struct LuaFileEntry {
  std::string path;
  bool watchChanges;
};

static const std::vector<LuaFileEntry> luaFiles = {
  { "gcb.lua", true },
  { "config.lua", false }, // config.lua is written automatically. Don't monitor it.
  { "games-config.lua", false },  // games-config.lua is written automatically. Don't monitor it.
  { "games.lua", true },
  { "main.lua", true },
  { "tray.lua", true },
  { "window.lua", true },
  { "games-gui.lua", true}
};

static std::vector<std::time_t> timestamps;

static void LoadLua() {
  lua::Init();
  for (const auto& file : luaFiles) {
    lua::ExecuteFile(file.path.c_str());
  }
  lua::InitTick();
  lua::InitForegroundCallbacks();
  lua::InitTrayCallback();
  lua::InitWindowCallback();
  lua::InitWindowCloseCallback();
}

static void UpdateTimestamps() {
  timestamps.clear();
  for (const auto& file : luaFiles) {
    if (file.watchChanges) {
      timestamps.push_back(tools::GetFileTimestamp(file.path));
    } else {
      timestamps.push_back(0); // Dummy-Wert
    }
  }
}

static void ShutdownLua() {
  lua::Shutdown();
}

static bool LuaFilesChanged() {
  for (size_t i = 0; i < luaFiles.size(); ++i) {
    if (!luaFiles[i].watchChanges) continue;
    if (tools::GetFileTimestamp(luaFiles[i].path) != timestamps[i]) {
      return true;
    }
  }
  return false;
}

int main() {
  tools::SetWorkingDirToExePath();

  init:;

  network::Init();
  UpdateTimestamps();
  LoadLua();

  int counter = 0;

  while (!shutdownRequest && !restartRequest && !restartAsAdminRequest) {
    tray::PollTrayMessages();
    window::PollEvents();

    if (counter % 100 == 0) { // Approx every second
      gamewatcher::Process();
      lua::TriggerTick();

      if (LuaFilesChanged()) {
        printf("Lua files changed, reloading...\n");
        gamewatcher::ResetState();
        ShutdownLua();
        window::DestroyAllWindows();
        UpdateTimestamps();
        LoadLua();
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    counter++;
  }

  gamewatcher::ResetState();
  ShutdownLua();
  window::DestroyAllWindows();
  network::Deinit();

  if (restartAsAdminRequest) {
    restartAsAdminRequest = false;
    admin::RelaunchAsAdmin();
  }

  if (restartRequest) {
    restartRequest = false;
    goto init;
  }

  return 0;
}
