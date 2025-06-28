#pragma once

extern "C" {
#include <lua.h>
}

namespace lua {
namespace bindings {

void Register(lua_State* L);

} // namespace bindings
} // namespace lua
