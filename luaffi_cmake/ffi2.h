#ifndef FFI2_H
#define FFI2_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

LUAMOD_API
int luaopen_ffi(lua_State *L);

#endif // FFI2_H
