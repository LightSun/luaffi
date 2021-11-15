#ifndef LUA_UTIL_H
#define LUA_UTIL_H

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "hffi.h"
#include "h_array.h"
#include "h_list.h"

#ifndef __STR
#define __STR(x) #x
#endif

typedef hffi_struct* (*Func_get_ptr_struct)(lua_State* L, int idx);
typedef harray* (*Func_get_ptr_harray)(lua_State* L, int idx);
typedef hffi_smtype* (*Func_get_ptr_smtype)(lua_State* L, int idx);

//build smtypes. return HFFI_STATE_OK if success.
int build_smtypes(lua_State* L, array_list* sm_list, array_list* sm_names,
                   Func_get_ptr_struct func_struct, Func_get_ptr_harray func_harray, Func_get_ptr_smtype func_smtype);

int hlua_get_int(lua_State* L, int idx, const char* key, int def_val);

int hlua_get_number(lua_State* L, int idx, const char* key, lua_Number def_val);

int hlua_get_boolean(lua_State* L, int idx, const char* key, int def_val);

int hlua_rawgeti_int(lua_State* L, int tab_idx, int n);

#endif // LUA_UTIL_H
