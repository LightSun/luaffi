
#include "lua_utils.h"
#include "h_alloctor.h"
#include "atomic.h"

struct HDataWrapper{
    void* data;
    Fun_delete func;
    volatile int ref;
};

int hlua_get_ref(lua_State* L, int tab_id, const char* key, int t){
    int ref_ctx = LUA_NOREF;
    if(lua_getfield(L, tab_id, key) != LUA_TNIL){
        ref_ctx = luaL_ref(L, t);
        lua_pushnil(L);
        lua_setfield(L, HLUA_ADJUST_ID(tab_id), key);
    }
    lua_pop(L, 1);
    return ref_ctx;
}

int hlua_get_int(lua_State* L, int idx, const char* key, int def_val){
    int t = lua_getfield(L, idx, key);
    if(t == LUA_TNUMBER){
        def_val = luaL_checkinteger(L, -1);
        lua_pushnil(L);
        lua_setfield(L, HLUA_ADJUST_ID(idx), key);
    }
    lua_pop(L, 1);
    return def_val;
}

int hlua_get_number(lua_State* L, int idx, const char* key, lua_Number def_val){
    int t = lua_getfield(L, idx, key);
    if(t == LUA_TNUMBER){
        def_val = lua_tonumber(L, -1);
        lua_pushnil(L);
        lua_setfield(L, HLUA_ADJUST_ID(idx), key);
    }
    lua_pop(L, 1);
    return def_val;
}

int hlua_get_boolean(lua_State* L, int idx, const char* key, int def_val){
    int t = lua_getfield(L, idx, key);
    if(t == LUA_TNUMBER){
        def_val = lua_toboolean(L, -1);
        lua_pushnil(L);
        lua_setfield(L, HLUA_ADJUST_ID(idx), key);
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

//infos[4]: continue_mem, share_mem, rows, cols
int hlua_get_ext_info(lua_State* L, int tab_idx, int* infos){
    int continue_mem = 1;
    int share_mem = 1;
    infos[0] = hlua_get_boolean(L, tab_idx, "continue_mem", continue_mem);
    infos[1] = hlua_get_boolean(L, tab_idx, "share_mem", share_mem);
    //infos[1] = hlua_get_boolean(L, tab_idx, "hffi_t", share_mem);

    if(lua_rawlen(L, tab_idx) == 0) {
        return -1;
    }
    int rows = hlua_rawgeti_int(L, tab_idx, 1);
    int cols = 0;
    if(lua_rawlen(L, 1) >= 2){
        cols = hlua_rawgeti_int(L, tab_idx, 2);
    }
    infos[2] = rows;
    infos[3] = cols;
    return 0;
}

void hlua_push_light_uservalue(lua_State* L, int tab_idx, void* ud, Fun_delete delete){
    struct HDataWrapper* w = MALLOC(sizeof (struct HDataWrapper));
    w->data = ud;
    w->func = delete;
    w->ref = 1;
    lua_pushlightuserdata(L, w);
    lua_setuservalue(L, tab_idx);
}
void* hlua_get_light_uservalue(lua_State* L, int tab_index){
    if(lua_getuservalue(L, tab_index) == LUA_TNIL){
        return NULL;
    }
    struct HDataWrapper* w = (struct HDataWrapper*)lua_topointer(L, -1);
    lua_pop(L, 1);
    return w->data;
}
void hlua_delete_light_uservalue(lua_State* L, int tab_index){
    lua_getuservalue(L, tab_index);
    struct HDataWrapper* w = (struct HDataWrapper*)lua_topointer(L, -1);
    lua_pop(L, 1);
    if(atomic_add(&w->ref, -1) == 1){
        w->func(w->data);
        FREE(w);
    }
}

void hlua_share_light_uservalue(lua_State* L, int tab_old, int tab_new){
    lua_getuservalue(L, tab_old);
    struct HDataWrapper* w = (struct HDataWrapper*)lua_topointer(L, -1);
    atomic_add(&w->ref, 1);
    lua_setuservalue(L, tab_new);
}

#define CHECK_NEXT_ASPTR()\
if(lua_rawgeti(L, 1, i + 2) == LUA_TBOOLEAN){\
    asPtr = lua_toboolean(L, -1);\
    /* i + 1 is the flag of ptr. */ \
    i ++; \
}else{\
    asPtr = 0;\
}\
lua_pop(L, 1);

#define CHECK_NEXT_AS_STRING()\
if(lua_rawgeti(L, 1, i + 2) == LUA_TSTRING){\
    array_list_add(sm_names, strdup(lua_tostring(L, -1)));\
    i ++;\
}else{\
    array_list_add(sm_names, NULL);\
}\
lua_pop(L, 1);

int build_smtypes(lua_State* L, array_list* sm_list, array_list* sm_names,
                   Func_get_ptr_struct func_struct, Func_get_ptr_harray func_harray, Func_get_ptr_smtype func_smtype){
    int len = lua_rawlen(L, 1);
    int lua_t, asPtr;
    hffi_smtype* tmp_smtype;
    for(int i = 0 ; i < len; i++){
        lua_t = lua_rawgeti(L, 1, i + 1);
        if(lua_t == LUA_TNUMBER){
            array_list_add(sm_list, hffi_new_smtype(luaL_checkinteger(L, -1)));
            CHECK_NEXT_AS_STRING();
        }else if(luaL_testudata(L, -1, __STR(hffi_struct))){
            //may be ptr
            CHECK_NEXT_ASPTR()
            //check if have name.
            CHECK_NEXT_AS_STRING();
            //create smtype now.
            tmp_smtype = asPtr ? hffi_new_smtype_struct_ptr((hffi_struct*)func_struct(L, -1))
                               : hffi_new_smtype_struct((hffi_struct*)func_struct(L, -1));
            array_list_add(sm_list, tmp_smtype);
        }else if(luaL_testudata(L, -1, __STR(harray))){
            //may be ptr
            CHECK_NEXT_ASPTR()
            CHECK_NEXT_AS_STRING();
            tmp_smtype = asPtr ? hffi_new_smtype_harray_ptr(func_harray(L, -1))
                               : hffi_new_smtype_harray(func_harray(L, -1));
            array_list_add(sm_list, tmp_smtype);
        }else if(luaL_testudata(L, -1, __STR(hffi_smtype))){
            tmp_smtype = func_smtype(L, -1);
            hffi_smtype_ref(tmp_smtype, 1);
            array_list_add(sm_list, tmp_smtype);
            CHECK_NEXT_AS_STRING();
        }
        else{
            return HFFI_STATE_FAILED;
        }
        lua_pop(L, 1);
    }
    return HFFI_STATE_OK;
}

