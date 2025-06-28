#include "lua-bindings.h"
#include "cpu.h"
#include "games.h"
#include "desktop.h"
#include "scheduler.h"
#include "console.h"
#include "display.h"
#include "network.h"
#include "tools.h"

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

namespace lua {
namespace bindings {

// CPU

static int GetCPUInfo(lua_State* L) {
  cpu::CPUInfo info = cpu::GetCPUInfo();

  lua_newtable(L);

  lua_pushstring(L, "name");
  lua_pushstring(L, info.name.c_str());
  lua_settable(L, -3);

  lua_pushstring(L, "threads");
  lua_pushinteger(L, info.threads);
  lua_settable(L, -3);

  lua_pushstring(L, "numCcds");
  lua_pushinteger(L, info.numCcds);
  lua_settable(L, -3);

  lua_pushstring(L, "ccds");
  lua_newtable(L);

  for (int i = 0; i < info.numCcds; ++i) {
    const auto& ccd = info.ccds[i];
    lua_newtable(L);

    lua_pushstring(L, "cores");
    lua_pushinteger(L, ccd.cores);
    lua_settable(L, -3);

    lua_pushstring(L, "threads");
    lua_pushinteger(L, ccd.threads);
    lua_settable(L, -3);

    lua_pushstring(L, "isX3D");
    lua_pushboolean(L, ccd.isX3D);
    lua_settable(L, -3);

    lua_pushstring(L, "firstThread");
    lua_pushinteger(L, ccd.firstThreadNum);
    lua_settable(L, -3);

    lua_pushstring(L, "lastThread");
    lua_pushinteger(L, ccd.lastThreadNum);
    lua_settable(L, -3);

    lua_rawseti(L, -2, i + 1);
  }

  lua_settable(L, -3);

  return 1;
}

// Games

static int ClearGameList(lua_State*) {
  games::ClearList();
  return 0;
}

static int AddGame(lua_State* L) {
  const char* name = luaL_checkstring(L, 1);
  const char* binary = luaL_checkstring(L, 2);
  games::AddGame(name, binary);
  return 0;
}

// Desktop

static int DisableDesktopEffects(lua_State*) {
  desktop::DisableEffects();
  return 0;
}

static int EnableDesktopEffects(lua_State*) {
  desktop::EnableEffects();
  return 0;
}

// Scheduler

static int BindProcessToThreads(lua_State* L) {
  int pid = luaL_checkinteger(L, 1);
  if (!lua_istable(L, 2)) {
    return luaL_error(L, "Expected table as second argument");
  }

  std::vector<int> threads;
  lua_pushnil(L);
  while (lua_next(L, 2)) {
    if (lua_isinteger(L, -1)) {
      threads.push_back(static_cast<int>(lua_tointeger(L, -1)));
    }
    lua_pop(L, 1);
  }

  bool result = scheduler::BindProcessToThreads(pid, threads);
  lua_pushboolean(L, result);
  return 1;
}

static int GetProcessThreads(lua_State* L) {
  int pid = luaL_checkinteger(L, 1);
  std::vector<int> threads = scheduler::GetProcessThreads(pid);

  lua_newtable(L);
  int index = 1;
  for (int t : threads) {
    lua_pushinteger(L, t);
    lua_rawseti(L, -2, index++);
  }

  return 1;
}

// Console

static int HideConsole(lua_State*) {
  console::HideWindow();
  return 0;
}

// Displays

static int GetMonitors(lua_State* L) {
  const auto monitors = display::GetMonitors();

  lua_newtable(L);

  for (size_t i = 0; i < monitors.size(); ++i) {
    const auto& m = monitors[i];

    lua_newtable(L);

    lua_pushstring(L, "device");
    lua_pushstring(L, m.deviceName.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "active");
    lua_pushboolean(L, m.isActive);
    lua_settable(L, -3);

    lua_pushstring(L, "primary");
    lua_pushboolean(L, m.isPrimary);
    lua_settable(L, -3);

    lua_pushstring(L, "description");
    lua_pushstring(L, m.deviceString.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "monitorName");
    lua_pushstring(L, m.monitorName.c_str());
    lua_settable(L, -3);

    lua_pushstring(L, "posX");
    lua_pushinteger(L, m.posX);
    lua_settable(L, -3);

    lua_pushstring(L, "posY");
    lua_pushinteger(L, m.posY);
    lua_settable(L, -3);

    lua_pushstring(L, "width");
    lua_pushinteger(L, m.width);
    lua_settable(L, -3);

    lua_pushstring(L, "height");
    lua_pushinteger(L, m.height);
    lua_settable(L, -3);

    lua_pushstring(L, "bitsPerPel");
    lua_pushinteger(L, m.bitsPerPel);
    lua_settable(L, -3);

    lua_pushstring(L, "displayFrequency");
    lua_pushinteger(L, m.displayFrequency);
    lua_settable(L, -3);

    lua_rawseti(L, -2, i + 1);
  }

  return 1;
}

static int DisableMonitor(lua_State* L) {
  const char* device = luaL_checkstring(L, 1);
  bool success = display::DisableMonitor(device);
  lua_pushboolean(L, success);
  return 1;
}

static int EnableMonitor(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);

  display::MonitorInfo info;

  lua_getfield(L, 1, "device");
  info.deviceName = luaL_checkstring(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, 1, "posX");
  info.posX = (int)luaL_checkinteger(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, 1, "posY");
  info.posY = (int)luaL_checkinteger(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, 1, "width");
  info.width = (int)luaL_checkinteger(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, 1, "height");
  info.height = (int)luaL_checkinteger(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, 1, "bitsPerPel");
  info.bitsPerPel = (int)luaL_checkinteger(L, -1);
  lua_pop(L, 1);

  lua_getfield(L, 1, "displayFrequency");
  info.displayFrequency = (int)luaL_checkinteger(L, -1);
  lua_pop(L, 1);

  bool success = display::EnableMonitor(info);
  lua_pushboolean(L, success);
  return 1;
}

// Network

static int SendUdp(lua_State* L) {
  const char* host = luaL_checkstring(L, 1);
  int port = luaL_checkinteger(L, 2);
  const char* message = luaL_checkstring(L, 3);

  bool success = network::SendUdpMessage(host, port, message);
  lua_pushboolean(L, success);
  return 1;
}

// Tools

static int GetFileTimestamp(lua_State* L) {
  const char* path = luaL_checkstring(L, 1);
  std::time_t ts = tools::GetFileTimestamp(path);
  lua_pushinteger(L, static_cast<lua_Integer>(ts));
  return 1;
}

static int SleepMs(lua_State* L) {
  int ms = luaL_checkinteger(L, 1);
  tools::SleepMs(ms);
  return 0;
}

void Register(lua_State* L) {
  lua_getglobal(L, "gcb");

  if (lua_isnil(L, -1)) {
    lua_pop(L, 1);
    lua_newtable(L);
  }

  // CPU
  lua_pushcfunction(L, GetCPUInfo);
  lua_setfield(L, -2, "getCPUInfo");

  // Games
  lua_pushcfunction(L, ClearGameList);
  lua_setfield(L, -2, "clearGameList");

  lua_pushcfunction(L, AddGame);
  lua_setfield(L, -2, "addGame");

  // Desktop
  lua_pushcfunction(L, DisableDesktopEffects);
  lua_setfield(L, -2, "disableDesktopEffects");

  lua_pushcfunction(L, EnableDesktopEffects);
  lua_setfield(L, -2, "enableDesktopEffects");

  // Scheduler
  lua_pushcfunction(L, BindProcessToThreads);
  lua_setfield(L, -2, "bindProcessToThreads");

  lua_pushcfunction(L, GetProcessThreads);
  lua_setfield(L, -2, "getProcessThreads");

  // Console
  lua_pushcfunction(L, HideConsole);
  lua_setfield(L, -2, "hideConsole");

  // Displays
  lua_pushcfunction(L, GetMonitors);
  lua_setfield(L, -2, "getMonitors");

  lua_pushcfunction(L, DisableMonitor);
  lua_setfield(L, -2, "disableMonitor");

  lua_pushcfunction(L, EnableMonitor);
  lua_setfield(L, -2, "enableMonitor");

  // Network
  lua_pushcfunction(L, SendUdp);
  lua_setfield(L, -2, "sendUdp");

  // Tools
  lua_pushcfunction(L, GetFileTimestamp);
  lua_setfield(L, -2, "getFileTimestamp");

  lua_pushcfunction(L, SleepMs);
  lua_setfield(L, -2, "sleepMs");

  lua_setglobal(L, "gcb");
}

} // namespace bindings
} // namespace lua
