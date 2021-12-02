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

#define HLUA_ADJUST_ID(id) (id < 0 ? lua_gettop(L) + id + 1 : id)

typedef hffi_struct* (*Func_get_ptr_struct)(lua_State* L, int idx);
typedef harray* (*Func_get_ptr_harray)(lua_State* L, int idx);
typedef hffi_smtype* (*Func_get_ptr_smtype)(lua_State* L, int idx);
typedef hffi_closure* (*Func_get_ptr_closure)(lua_State* L, int idx);
typedef hffi_value* (*Func_get_ptr_value)(lua_State* L, int idx);
typedef void(*Fun_delete)(void* data);

harray* hlua_new_harray_from_table(lua_State* L, int idx, Func_get_ptr_struct func_struct, Func_get_ptr_harray func_harray);
//build smtypes. return HFFI_STATE_OK if success.
int build_smtypes(lua_State* L, array_list* sm_list, array_list* sm_names,
                   Func_get_ptr_struct func_struct, Func_get_ptr_harray func_harray,
                  Func_get_ptr_smtype func_smtype, Func_get_ptr_closure func_clo,
                  Func_get_ptr_value func_val);

int hlua_get_int(lua_State* L, int idx, const char* key, int def_val);

int hlua_get_number(lua_State* L, int idx, const char* key, lua_Number def_val);

int hlua_get_boolean(lua_State* L, int idx, const char* key, int def_val);

int hlua_rawgeti_int(lua_State* L, int tab_idx, int n);

//get ext info from a table: continue_mem, share_mem, rows, cols
int hlua_get_ext_info(lua_State* L, int tab_idx, int* infos);

int hlua_get_ref(lua_State* L, int tab_id, const char* key, int t);

void hlua_push_light_uservalue(lua_State* L, int tab_idx, void* ud, Fun_delete delete);
void hlua_share_light_uservalue(lua_State* L, int tab_old, int tab_new);
void* hlua_get_light_uservalue(lua_State* L, int tab_index);
void hlua_delete_light_uservalue(lua_State* L, int tab_index);

int hlua_get_struct_member_index(lua_State* L, int tab_index, int count, const char* in_name);

LUALIB_API int luaB_dumpStack(lua_State* L);

#endif // LUA_UTIL_H
