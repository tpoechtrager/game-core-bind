#include "lua.h"
#include "lua-bindings.h"
#include "game-watcher.h"
#include "tools.h"
#include "network.h"
#include <thread>
#include <chrono>
#include <filesystem>
#include <vector>
#include <string>
#include <cstdio>

static const std::vector<std::string> luaFiles = { 
  "gcb.lua",
  "config.lua",
  "games.lua",
  "main.lua"
};

static std::vector<std::time_t> timestamps;

static void LoadLua() {
  lua::Init();
  for (const auto& file : luaFiles) {
    lua::ExecuteFile(file.c_str());
  }
  lua::InitTick();
  lua::InitForegroundCallbacks();
}

static void UpdateTimestamps() {
  timestamps.clear();
  for (const auto& file : luaFiles) {
    timestamps.push_back(tools::GetFileTimestamp(file));
  }
}

static void ShutdownLua() {
  lua::Shutdown();
}

static bool LuaFilesChanged() {
  for (size_t i = 0; i < luaFiles.size(); ++i) {
    if (tools::GetFileTimestamp(luaFiles[i]) != timestamps[i]) {
      return true;
    }
  }
  return false;
}

int main() {
  tools::SetWorkingDirToExePath();

  network::Init();
  UpdateTimestamps();
  LoadLua();

  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    gamewatcher::Process();
    lua::TriggerTick();

    if (LuaFilesChanged()) {
      printf("Lua files changed, reloading...\n");
      gamewatcher::ResetState();
      ShutdownLua();
      UpdateTimestamps();
      LoadLua();
    }
  }

  ShutdownLua();
  network::Deinit();
}
