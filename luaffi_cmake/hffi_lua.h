#ifndef HFFI_LUA_H
#define HFFI_LUA_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

LUAMOD_API void register_ffi(lua_State *L);

#endif // HFFI_LUA_H
