#include "lua.h"
#include "lua-bindings.h"
#include <cstdio>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

namespace lua {

static lua_State* L = nullptr;
static int tickFuncRef = LUA_REFNIL;
static int gameStartFuncRef = LUA_REFNIL;
static int gameStopFuncRef = LUA_REFNIL;
static int gameForegroundRef = LUA_REFNIL;
static int gameBackgroundRef = LUA_REFNIL;

void Init() {
  L = luaL_newstate();
  luaL_openlibs(L);
  bindings::Register(L);
}

void Execute(const char* code) {
  if (luaL_dostring(L, code)) {
    printf("Lua error: %s\n", lua_tostring(L, -1));
  }
}

void ExecuteFile(const char* filename) {
  if (luaL_dofile(L, filename)) {
    printf("Lua error: %s\n", lua_tostring(L, -1));
  }
}

// Tick

void InitTick() {
  tickFuncRef = LUA_REFNIL;
  gameStartFuncRef = LUA_REFNIL;
  gameStopFuncRef = LUA_REFNIL;

  lua_getglobal(L, "gcb");
  if (lua_istable(L, -1)) {

    lua_getfield(L, -1, "onTick");
    if (lua_isfunction(L, -1)) {
      tickFuncRef = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
      lua_pop(L, 1);
    }

    lua_getfield(L, -1, "onGameStart");
    if (lua_isfunction(L, -1)) {
      gameStartFuncRef = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
      lua_pop(L, 1);
    }

    lua_getfield(L, -1, "onGameStop");
    if (lua_isfunction(L, -1)) {
      gameStopFuncRef = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
      lua_pop(L, 1);
    }
  }
  lua_pop(L, 1);
}

void TriggerTick() {
  if (tickFuncRef == LUA_REFNIL) {
    return;
  }

  lua_rawgeti(L, LUA_REGISTRYINDEX, tickFuncRef);
  if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
    printf("Lua error: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
  }
}

// Game start/stop events

void TriggerGameStart(int pid, const std::string& name, const std::string& binary) {
  if (gameStartFuncRef == LUA_REFNIL) {
    return;
  }

  lua_rawgeti(L, LUA_REGISTRYINDEX, gameStartFuncRef);
  lua_pushinteger(L, pid);
  lua_pushstring(L, name.c_str());
  lua_pushstring(L, binary.c_str());

  if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
    printf("Lua error: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
  }
}

void TriggerGameStop(int pid, const std::string& name, const std::string& binary) {
  if (gameStopFuncRef == LUA_REFNIL) {
    return;
  }

  lua_rawgeti(L, LUA_REGISTRYINDEX, gameStopFuncRef);
  lua_pushinteger(L, pid);
  lua_pushstring(L, name.c_str());
  lua_pushstring(L, binary.c_str());

  if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
    printf("Lua error: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
  }
}

// Game foreground/background events

void InitForegroundCallbacks() {
  gameForegroundRef = LUA_REFNIL;
  gameBackgroundRef = LUA_REFNIL;

  lua_getglobal(L, "gcb");
  if (lua_istable(L, -1)) {
    lua_getfield(L, -1, "onGameForeground");
    if (lua_isfunction(L, -1)) {
      gameForegroundRef = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
      lua_pop(L, 1);
    }

    lua_getfield(L, -1, "onGameBackground");
    if (lua_isfunction(L, -1)) {
      gameBackgroundRef = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
      lua_pop(L, 1);
    }
  }
  lua_pop(L, 1);
}

void TriggerGameForeground(int pid, const std::string& name, const std::string& binary) {
  if (gameForegroundRef == LUA_REFNIL) return;

  lua_rawgeti(L, LUA_REGISTRYINDEX, gameForegroundRef);
  lua_pushinteger(L, pid);
  lua_pushstring(L, name.c_str());
  lua_pushstring(L, binary.c_str());
  if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
    printf("Lua error: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
  }
}

void TriggerGameBackground(int pid, const std::string& name, const std::string& binary) {
  if (gameBackgroundRef == LUA_REFNIL) return;

  lua_rawgeti(L, LUA_REGISTRYINDEX, gameBackgroundRef);
  lua_pushinteger(L, pid);
  lua_pushstring(L, name.c_str());
  lua_pushstring(L, binary.c_str());
  if (lua_pcall(L, 3, 0, 0) != LUA_OK) {
    printf("Lua error: %s\n", lua_tostring(L, -1));
    lua_pop(L, 1);
  }
}

void Shutdown() {
  if (L) {
    tickFuncRef = LUA_REFNIL;
    gameStartFuncRef = LUA_REFNIL;
    gameStopFuncRef = LUA_REFNIL;
    gameForegroundRef = LUA_REFNIL;
    gameBackgroundRef = LUA_REFNIL;

    lua_close(L);
    L = nullptr;
  }
}

} // namespace lua
