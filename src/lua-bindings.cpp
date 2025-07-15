#include <unordered_map>
#include "lua-bindings.h"
#include "cpu.h"
#include "games.h"
#include "desktop.h"
#include "scheduler.h"
#include "display.h"
#include "network.h"
#include "tools.h"
#include "tray.h"
#include "window.h"
#include "messagebox.h"
#include "admin.h"
#include "main.h"

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

  int result = scheduler::BindProcessToThreads(pid, threads);
  lua_pushinteger(L, result);
  return 1;
}

static int GetProcessThreads(lua_State* L) {
  int pid = luaL_checkinteger(L, 1);
  scheduler::GetThreadsResult res = scheduler::GetProcessThreads(pid);

  lua_newtable(L);
  lua_pushstring(L, "code");
  lua_pushinteger(L, static_cast<int>(res.code));
  lua_settable(L, -3);

  lua_pushstring(L, "threads");
  lua_newtable(L);

  int index = 1;
  for (int t : res.threads) {
    lua_pushinteger(L, t);
    lua_rawseti(L, -2, index++);
  }

  lua_settable(L, -3);
  return 1;
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

static int RunDetached(lua_State* L) {
  const char* exe = luaL_checkstring(L, 1);
  const char* args = luaL_optstring(L, 2, "");
  tools::RunDetached(exe, args);
  return 0;
}

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

// Tray

static int TrayInit(lua_State* L) {
  const char* tooltip = luaL_checkstring(L, 1);
  tray::Init(tooltip);
  return 0;
}

static int TrayDestroy(lua_State*) {
  tray::Destroy();
  return 0;
}

static int TrayAddMenuItem(lua_State* L) {
  const char* text = luaL_checkstring(L, 1);
  int id = luaL_checkinteger(L, 2);
  tray::AddMenuItem(text, id);
  return 0;
}

static int TrayAddCheckableMenuItem(lua_State* L) {
  const char* text = luaL_checkstring(L, 1);
  int id = luaL_checkinteger(L, 2);
  bool checked = lua_toboolean(L, 3);
  tray::AddCheckableMenuItem(text, id, checked);
  return 0;
}

static int TraySetMenuChecked(lua_State* L) {
  int id = luaL_checkinteger(L, 1);
  bool checked = lua_toboolean(L, 2);
  tray::SetMenuItemChecked(id, checked);
  return 0;
}

static int TrayIsMenuChecked(lua_State* L) {
  int id = luaL_checkinteger(L, 1);
  lua_pushboolean(L, tray::IsMenuItemChecked(id));
  return 1;
}

static int TrayCreateSubMenu(lua_State* L) {
  HMENU submenu = tray::CreateSubMenu();
  lua_pushlightuserdata(L, submenu);
  return 1;
}

static int TrayAddMenuItemToSubMenu(lua_State* L) {
  HMENU submenu = (HMENU)lua_touserdata(L, 1);
  const char* text = luaL_checkstring(L, 2);
  int id = luaL_checkinteger(L, 3);
  tray::AddMenuItemToSubMenu(submenu, text, id);
  return 0;
}

static int TrayAddSubMenu(lua_State* L) {
  const char* text = luaL_checkstring(L, 1);
  HMENU submenu = (HMENU)lua_touserdata(L, 2);
  tray::AddSubMenu(text, submenu);
  return 0;
}

// Window

static std::unordered_map<void*, window::WindowPtr> windowRegistry;

static int WindowCreate(lua_State* L) {
  luaL_checktype(L, 1, LUA_TTABLE);
  window::WindowConfig config;

  lua_getfield(L, 1, "title");
  config.title = luaL_optstring(L, -1, "Window");
  lua_pop(L, 1);

  lua_getfield(L, 1, "width");
  config.width = (int)luaL_optinteger(L, -1, 800);
  lua_pop(L, 1);

  lua_getfield(L, 1, "height");
  config.height = (int)luaL_optinteger(L, -1, 600);
  lua_pop(L, 1);

  lua_getfield(L, 1, "resizable");
  config.resizable = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : true;
  lua_pop(L, 1);

  lua_getfield(L, 1, "center");
  if (lua_isboolean(L, -1) && lua_toboolean(L, -1)) {
    config.position = window::WindowPosition::Center;
  }
  lua_pop(L, 1);

  lua_getfield(L, 1, "scrollable");
  config.scrollableContent = lua_isboolean(L, -1) && lua_toboolean(L, -1);
  lua_pop(L, 1);

  auto ptr = window::Create(config);
  void* key = ptr.get();
  windowRegistry[key] = ptr;

  lua_pushlightuserdata(L, key);
  return 1;
}

static window::WindowPtr GetWindow(lua_State* L, int index) {
  void* key = lua_touserdata(L, index);
  auto it = windowRegistry.find(key);
  if (it != windowRegistry.end()) {
    return it->second;
  }
  return nullptr;
}

static int WindowDestroy(lua_State* L) {
  void* key = lua_touserdata(L, 1);
  auto it = windowRegistry.find(key);
  if (it != windowRegistry.end()) {
    window::Destroy(it->second);
    windowRegistry.erase(it);
  }
  return 0;
}

static int WindowShow(lua_State* L) {
  auto win = GetWindow(L, 1);
  if (win) {
    window::Show(win);
  }
  return 0;
}

static int WindowAddStatic(lua_State* L) {
  auto win = GetWindow(L, 1);
  int x = luaL_checkinteger(L, 2);
  int y = luaL_checkinteger(L, 3);
  int w = luaL_checkinteger(L, 4);
  int h = luaL_checkinteger(L, 5);
  const char* text = luaL_checkstring(L, 6);
  lua_pushinteger(L, window::AddStatic(win, x, y, w, h, text));
  return 1;
}

static int WindowAddGroupBox(lua_State* L) {
  auto win = GetWindow(L, 1);
  int x = luaL_checkinteger(L, 2);
  int y = luaL_checkinteger(L, 3);
  int w = luaL_checkinteger(L, 4);
  int h = luaL_checkinteger(L, 5);
  const char* label = luaL_checkstring(L, 6);
  lua_pushinteger(L, window::AddGroupBox(win, x, y, w, h, label));
  return 1;
}

static int WindowAddComboBox(lua_State* L) {
  auto win = GetWindow(L, 1);
  int x = luaL_checkinteger(L, 2);
  int y = luaL_checkinteger(L, 3);
  int w = luaL_checkinteger(L, 4);
  int h = luaL_checkinteger(L, 5);
  luaL_checktype(L, 6, LUA_TTABLE);

  std::vector<std::string> items;
  lua_pushnil(L);
  while (lua_next(L, 6)) {
    if (lua_isstring(L, -1)) {
      items.emplace_back(lua_tostring(L, -1));
    }
    lua_pop(L, 1);
  }

  lua_pushinteger(L, window::AddComboBox(win, x, y, w, h, items));
  return 1;
}

static int WindowAddCheckBox(lua_State* L) {
  auto win = GetWindow(L, 1);
  int x = luaL_checkinteger(L, 2);
  int y = luaL_checkinteger(L, 3);
  int w = luaL_checkinteger(L, 4);
  int h = luaL_checkinteger(L, 5);
  const char* label = luaL_checkstring(L, 6);
  lua_pushinteger(L, window::AddCheckBox(win, x, y, w, h, label));
  return 1;
}

static int WindowSetCheckBoxChecked(lua_State* L) {
  auto win = GetWindow(L, 1);
  int id = luaL_checkinteger(L, 2);
  bool value = lua_toboolean(L, 3);
  window::SetCheckBoxChecked(win, id, value);
  return 0;
}

static int WindowGetCheckBoxChecked(lua_State* L) {
  auto win = GetWindow(L, 1);
  int id = luaL_checkinteger(L, 2);
  lua_pushboolean(L, window::GetCheckBoxChecked(win, id));
  return 1;
}

static int WindowSetComboBoxSelectedIndex(lua_State* L) {
  auto win = GetWindow(L, 1);
  int id = luaL_checkinteger(L, 2);
  int index = luaL_checkinteger(L, 3);
  window::SetComboBoxSelectedIndex(win, id, index);
  return 0;
}

static int WindowGetComboBoxSelectedIndex(lua_State* L) {
  auto win = GetWindow(L, 1);
  int id = luaL_checkinteger(L, 2);
  int index = window::GetComboBoxSelectedIndex(win, id);
  lua_pushinteger(L, index);
  return 1;
}

static int WindowAddEditBox(lua_State* L) {
  auto win = GetWindow(L, 1);
  int x = luaL_checkinteger(L, 2);
  int y = luaL_checkinteger(L, 3);
  int w = luaL_checkinteger(L, 4);
  int h = luaL_checkinteger(L, 5);
  const char* text = luaL_optstring(L, 6, "");
  bool disabled = false;
  if (lua_gettop(L) >= 7) {
    disabled = lua_toboolean(L, 7);
  }
  lua_pushinteger(L, window::AddEditBox(win, x, y, w, h, text, disabled));
  return 1;
}

static int WindowGetEditBoxText(lua_State* L) {
  auto win = GetWindow(L, 1);
  int id = luaL_checkinteger(L, 2);
  lua_pushstring(L, window::GetEditBoxText(win, id).c_str());
  return 1;
}

static int WindowAddButton(lua_State* L) {
  auto win = GetWindow(L, 1);
  int x = luaL_checkinteger(L, 2);
  int y = luaL_checkinteger(L, 3);
  int w = luaL_checkinteger(L, 4);
  int h = luaL_checkinteger(L, 5);
  const char* label = luaL_checkstring(L, 6);
  lua_pushinteger(L, window::AddButton(win, x, y, w, h, label));
  return 1;
}

// Messagebox

static int ShowMessageBox(lua_State* L) {
  const char* title = luaL_checkstring(L, 1);
  const char* text = luaL_checkstring(L, 2);
  messagebox::Show(title, text);
  return 0;
}

static int ShowYesNoBox(lua_State* L) {
  const char* title = luaL_checkstring(L, 1);
  const char* text = luaL_checkstring(L, 2);
  bool result = messagebox::ConfirmYesNo(title, text);
  lua_pushboolean(L, result);
  return 1;
}

// Admin

static int IsRunningAsAdmin(lua_State* L) {
  lua_pushboolean(L, admin::IsRunningAsAdmin());
  return 1;
}

// Main

static int SetShutdown(lua_State*) {
  shutdownRequest = true;
  return 0;
}

static int SetRestart(lua_State*) {
  restartRequest = true;
  return 0;
}

static int SetRestartAsAdmin(lua_State*) {
  restartAsAdminRequest = true;
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
  lua_pushinteger(L, scheduler::BIND_SUCCESS);
  lua_setfield(L, -2, "PROCESS_BIND_SUCCESS");

  lua_pushinteger(L, scheduler::BIND_INVALID_THREAD_INDEX);
  lua_setfield(L, -2, "PROCESS_BIND_INVALID_THREAD_INDEX");

  lua_pushinteger(L, scheduler::BIND_OPEN_PROCESS_FAILED);
  lua_setfield(L, -2, "PROCESS_BIND_OPEN_PROCESS_FAILED");

  lua_pushinteger(L, scheduler::BIND_PERMISSION_DENIED);
  lua_setfield(L, -2, "PROCESS_BIND_PERMISSION_DENIED");

  lua_pushinteger(L, scheduler::BIND_SETAFFINITY_FAILED);
  lua_setfield(L, -2, "PROCESS_BIND_SETAFFINITY_FAILED");

  lua_pushcfunction(L, BindProcessToThreads);
  lua_setfield(L, -2, "bindProcessToThreads");

  lua_pushinteger(L, scheduler::GET_THREADS_SUCCESS);
  lua_setfield(L, -2, "PROCESS_GET_THREADS_SUCCESS");

  lua_pushinteger(L, scheduler::GET_THREADS_OPEN_PROCESS_FAILED);
  lua_setfield(L, -2, "PROCESS_GET_THREADS_OPEN_PROCESS_FAILED");

  lua_pushinteger(L, scheduler::GET_THREADS_QUERY_FAILED);
  lua_setfield(L, -2, "PROCESS_GET_THREADS_QUERY_FAILED");

  lua_pushinteger(L, scheduler::GET_THREADS_PERMISSION_DENIED);
  lua_setfield(L, -2, "PROCESS_GET_THREADS_PERMISSION_DENIED");

  lua_pushcfunction(L, GetProcessThreads);
  lua_setfield(L, -2, "getProcessThreads");

  // Display
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
  lua_pushcfunction(L, RunDetached);
  lua_setfield(L, -2, "runDetached");

  lua_pushcfunction(L, GetFileTimestamp);
  lua_setfield(L, -2, "getFileTimestamp");

  lua_pushcfunction(L, SleepMs);
  lua_setfield(L, -2, "sleepMs");

  // Tray
  lua_newtable(L);

  lua_pushcfunction(L, TrayInit);
  lua_setfield(L, -2, "init");

  lua_pushcfunction(L, TrayDestroy);
  lua_setfield(L, -2, "destroy");

  lua_pushcfunction(L, TrayAddMenuItem);
  lua_setfield(L, -2, "addMenuItem");

  lua_pushcfunction(L, TrayAddCheckableMenuItem);
  lua_setfield(L, -2, "addCheckableMenuItem");

  lua_pushcfunction(L, TraySetMenuChecked);
  lua_setfield(L, -2, "setMenuChecked");

  lua_pushcfunction(L, TrayIsMenuChecked);
  lua_setfield(L, -2, "isMenuChecked");

  lua_pushcfunction(L, TrayCreateSubMenu);
  lua_setfield(L, -2, "createSubMenu");

  lua_pushcfunction(L, TrayAddMenuItemToSubMenu);
  lua_setfield(L, -2, "addMenuItemToSubMenu");

  lua_pushcfunction(L, TrayAddSubMenu);
  lua_setfield(L, -2, "addSubMenu");

  lua_setfield(L, -2, "tray");

  // Window
  lua_newtable(L);

  lua_pushcfunction(L, WindowCreate);
  lua_setfield(L, -2, "create");

  lua_pushcfunction(L, WindowDestroy);
  lua_setfield(L, -2, "destroy");

  lua_pushcfunction(L, WindowShow);
  lua_setfield(L, -2, "show");

  lua_pushcfunction(L, WindowAddStatic);
  lua_setfield(L, -2, "addStatic");

  lua_pushcfunction(L, WindowAddGroupBox);
  lua_setfield(L, -2, "addGroupBox");

  lua_pushcfunction(L, WindowAddComboBox);
  lua_setfield(L, -2, "addComboBox");

  lua_pushcfunction(L, WindowAddCheckBox);
  lua_setfield(L, -2, "addCheckBox");

  lua_pushcfunction(L, WindowSetCheckBoxChecked);
  lua_setfield(L, -2, "setCheckBoxChecked");

  lua_pushcfunction(L, WindowGetCheckBoxChecked);
  lua_setfield(L, -2, "getCheckBoxChecked");

  lua_pushcfunction(L, WindowSetComboBoxSelectedIndex);
  lua_setfield(L, -2, "setComboBoxSelectedIndex");

  lua_pushcfunction(L, WindowGetComboBoxSelectedIndex);
  lua_setfield(L, -2, "getComboBoxSelectedIndex");

  lua_pushcfunction(L, WindowAddEditBox);
  lua_setfield(L, -2, "addEditBox");

  lua_pushcfunction(L, WindowGetEditBoxText);
  lua_setfield(L, -2, "getEditBoxText");

  lua_pushcfunction(L, WindowAddButton);
  lua_setfield(L, -2, "addButton");

  lua_setfield(L, -2, "window");

  // Messagebox

  lua_pushcfunction(L, ShowMessageBox);
  lua_setfield(L, -2, "showMessageBox");

  lua_pushcfunction(L, ShowYesNoBox);
  lua_setfield(L, -2, "showYesNoBox");

  // Admin

  lua_pushcfunction(L, IsRunningAsAdmin);
  lua_setfield(L, -2, "isRunningAsAdmin");

  // Main

  lua_pushcfunction(L, SetShutdown);
  lua_setfield(L, -2, "shutdown");

  lua_pushcfunction(L, SetRestart);
  lua_setfield(L, -2, "restart");

  lua_pushcfunction(L, SetRestartAsAdmin);
  lua_setfield(L, -2, "restartAsAdmin");

  lua_setglobal(L, "gcb");
}

} // namespace bindings
} // namespace lua
