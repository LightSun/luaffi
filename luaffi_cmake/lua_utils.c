#include "hffi_lua.h"

int hlua_get_int(lua_State* L, int idx, const char* key, int def_val){
    int t = lua_getfield(L, idx, key);
    if(t == LUA_TNUMBER){
        def_val = luaL_checkinteger(L, -1);
        lua_pushnil(L);
        lua_setfield(L, idx, key);
    }
    lua_pop(L, 1);
    return def_val;
}

int hlua_get_number(lua_State* L, int idx, const char* key, lua_Number def_val){
    int t = lua_getfield(L, idx, key);
    if(t == LUA_TNUMBER){
        def_val = lua_tonumber(L, -1);
        lua_pushnil(L);
        lua_setfield(L, idx, key);
    }
    lua_pop(L, 1);
    return def_val;
}

int hlua_get_boolean(lua_State* L, int idx, const char* key, int def_val){
    int t = lua_getfield(L, idx, key);
    if(t == LUA_TNUMBER){
        def_val = lua_toboolean(L, -1);
        lua_pushnil(L);
        lua_setfield(L, idx, key);
    }
    lua_pop(L, 1);
    return def_val;
}
int hlua_rawgeti_int(lua_State* L, int tab_idx, int n){
    lua_rawgeti(L, tab_idx, n);
    int v = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    return v;
}

