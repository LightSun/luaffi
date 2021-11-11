
#include "hffi_lua.h"
#include "hffi.h"
#include "dym_loader.h"
#include "h_list.h"
#include "h_alloctor.h"

typedef struct BasePair{
    const char* name;
    sint8 type;
}BasePair;

#define __STR(s) #s

#define REG_CLASS(L, C)                             \
    do {                                            \
        luaL_newmetatable(L, __STR(C));      \
        lua_pushvalue(L, -1);                       \
        lua_setfield(L, -2, "__index");             \
        luaL_setfuncs(L, g_##C##_Methods, 0);        \
        lua_pop(L, 1); /* pop off the meta-table */ \
    } while (0)

#define push_ptr(T) \
void push_ptr_##T(lua_State* L, T* ptr) {\
    *(T**)lua_newuserdata(L, sizeof(T*)) = ptr;\
    luaL_getmetatable(L, __STR(T));\
    lua_setmetatable(L, -2);\
}
#define get_ptr(T)\
T* get_ref_##T(lua_State* L, int index) {\
    return *(T**)luaL_checkudata(L, index, __STR(T));\
}
#define DEF_HFFI_PUSH_GET(t) \
push_ptr(t)\
get_ptr(t)

DEF_HFFI_PUSH_GET(dym_lib)
DEF_HFFI_PUSH_GET(dym_func)
DEF_HFFI_PUSH_GET(hffi_value)
DEF_HFFI_PUSH_GET(hffi_struct)
DEF_HFFI_PUSH_GET(hffi_cif)

static const BasePair _BASE_PAIRS[] = {
    {"sint8", HFFI_TYPE_SINT8},
    {"uint8", HFFI_TYPE_UINT8},
    {"sint16", HFFI_TYPE_SINT16},
    {"uint16", HFFI_TYPE_UINT16},
    {"sint32", HFFI_TYPE_SINT32},
    {"uint32", HFFI_TYPE_UINT32},
    {"sint64", HFFI_TYPE_SINT64},
    {"uint64", HFFI_TYPE_UINT64},

    {"float", HFFI_TYPE_FLOAT},
    {"double", HFFI_TYPE_DOUBLE},
    {"void", HFFI_TYPE_VOID},
    {"pointer", HFFI_TYPE_POINTER},

    {"byte", HFFI_TYPE_SINT8},
    {"bool", HFFI_TYPE_SINT8},
    {"short", HFFI_TYPE_SINT16},
    {"long", HFFI_TYPE_SINT64},
    {"int", HFFI_TYPE_SINT32},
    {"uint", HFFI_TYPE_UINT32},
    {"string", HFFI_TYPE_STRING},
    {NULL, 0},
};
//-------------- share funcs --------------
static void smtype_delete(void* d){
    hffi_delete_smtype((hffi_smtype*)d);
}
static void string_delete(void* d){
    FREE(d);
}
static void value_delete(void* d){
    hffi_delete_value((hffi_value*)d);
}
//----------------- struct -------------
static inline int get_base_type(const char* name){
    const BasePair *lib;
    for (lib = _BASE_PAIRS; lib->name; lib++) {
        if(strcmp(lib->name, name) == 0){
            return lib->type;
        }
    }
    return -1;
}
//SDL_KeyboardEvent = ffi.struct {
//  uint32, "type";
//  uint32, "timestamp";
//  uint32, "windowID";
//  uint8, "state";
//  uint8, "repeat";
//  uint8; uint8;  --padding
//  SDL_Keysym, "keysym";
//}
static int xffi_struct_new(lua_State *L){
    sint8 sm_type;
    luaL_checktype(L, -1, LUA_TTABLE);
    int len = lua_rawlen(L, -1);
    if(len % 2 != 0){
        return luaL_error(L, "table len must be 2n.");
    }
    array_list* sm_list = array_list_new_simple();
    array_list* sm_names = array_list_new_simple();
    for(int i = 0 ; i < len / 2; i++){
        lua_rawgeti(L, -1, i * 2 + 1); //type (-2)
        lua_rawgeti(L, -2, (i + 1)*2); //name (-1)
        luaL_checkstring(L, -1);
        if(lua_type(L, -2) == LUA_TNUMBER){
            sm_type = (sint8)lua_tointeger(L, -2);
            array_list_add(sm_list, hffi_new_smtype_base(sm_type));
        }else{
            if(luaL_testudata(L, -2, __STR(hffi_struct)) != 0){
                array_list_add(sm_list, hffi_new_smtype_struct(get_ref_hffi_struct(L, -2)));
            }else{
                array_list_delete2(sm_list, smtype_delete);
                array_list_delete2(sm_names, string_delete);
                return luaL_error(L, "wrong sm_type = %s", lua_tostring(L, -1));
            }
        }
        array_list_add(sm_names, strdup(lua_tostring(L, -1)));
        lua_pop(L, 2);
    }
    //msg
    char _m[128];
    char* msg[1];
    msg[0] = _m;
    //create struct
    hffi_struct* _struct = hffi_new_struct_from_list(sm_list, -1, msg);
    array_list_delete2(sm_list, smtype_delete);
    push_ptr_hffi_struct(L, _struct);
    lua_pushlightuserdata(L, sm_names);
    lua_setuservalue(L, -2);
    return 1;
}
static int xffi_struct_gc(lua_State *L){
    hffi_struct* _struct = get_ref_hffi_struct(L, -1);
    lua_getuservalue(L, -1);
    array_list* sm_names = (array_list*)lua_topointer(L, -1);
    hffi_delete_struct(_struct);
    array_list_delete2(sm_names, string_delete);
    return 0;
}

static const luaL_Reg g_hffi_struct_Methods[] = {
    {"__gc", xffi_struct_gc},
    {NULL, NULL}
};
//------------------ hffi_value ------------
#define VALUE_SET_IMPL(ffi_t, t)\
case ffi_t:{\
    t val = (t)lua_tointeger(L, idx);\
    t* s = hval->ptr;\
    *s = val;\
}break;
#define VALUE_SET_IMPL_F(ffi_t, t)\
case ffi_t:{\
    t val = (t)lua_tonumber(L, idx);\
    t* s = hval->ptr;\
    *s = val;\
}break;
static inline int __hffi_value_set(hffi_value* hval, lua_State *L, int idx){
    switch (hval->type->base_ffi_type) {
    VALUE_SET_IMPL(HFFI_TYPE_SINT8, sint8)
    VALUE_SET_IMPL(HFFI_TYPE_UINT8, uint8)
    VALUE_SET_IMPL(HFFI_TYPE_SINT16, sint16)
    VALUE_SET_IMPL(HFFI_TYPE_UINT16, sint16)
    VALUE_SET_IMPL(HFFI_TYPE_SINT32, sint32)
    VALUE_SET_IMPL(HFFI_TYPE_UINT32, sint32)
    VALUE_SET_IMPL(HFFI_TYPE_SINT64, sint64)
    VALUE_SET_IMPL(HFFI_TYPE_UINT64, uint64)

    VALUE_SET_IMPL(HFFI_TYPE_INT, sint32)
    VALUE_SET_IMPL_F(HFFI_TYPE_FLOAT, float)
    VALUE_SET_IMPL_F(HFFI_TYPE_DOUBLE, double)
    case HFFI_TYPE_VOID: break;
    case HFFI_TYPE_POINTER:{
        //can set a value for ptr[0] . or set array for ptr[n]
        luaL_checktype(L, idx, LUA_TNUMBER);
        sint8 type = (sint8)lua_tointeger(L, idx);
        hffi_value_set_ptr_base_type(hval, type);
        //TODO
    } break;
    }
    //
    return 1;
}
static int xffi_value_new(lua_State *L){
    // base_type[, val]
    // pointer, ptr_base_type[, val]
    // struct[, is_pointer]
    // string/string_array
    int c = lua_gettop(L);
    hffi_value* val;
    if(lua_type(L, 1) == LUA_TNUMBER){
        sint8 type = (sint8)lua_tointeger(L, 1);
        //pointer,struct
        if(type == HFFI_TYPE_POINTER && c >= 2 && luaL_testudata(L, 2, __STR(hffi_struct))){
            val = hffi_new_value_struct_ptr(get_ref_hffi_struct(L, 2));
        }else{
            val = hffi_new_value_raw_type(type);
            if(c >= 2 && __hffi_value_set(val, L, 2) == 0){
                return 0;
            }
        }
        push_ptr_hffi_value(L, val);
        return 1;
    }else if(luaL_testudata(L, 1, __STR(hffi_struct))){
       //struct[, is_pointer]
       hffi_struct* stru = get_ref_hffi_struct(L, 1);
       if(c >= 2 && lua_toboolean(L, 2)){
            val = hffi_new_value_struct_ptr(stru);
       }else{
            val = hffi_new_value_struct(stru);
       }
       push_ptr_hffi_value(L, val);
       return 1;
    }
    return luaL_error(L, "new value only support: 'base_type[, val]' or 'struct[, is_pointer]'");
}
static int xffi_value_gc(lua_State *L){
    hffi_value* val = get_ref_hffi_value(L, 1);
    hffi_delete_value(val);
    return 0;
}

static const luaL_Reg g_hffi_value_Methods[] = {
    {"__gc", xffi_value_gc},
    {NULL, NULL}
};
//------------------- dym_func ------------
static int xffi_dym_func_gc(lua_State *L){
    dym_func* func = get_ref_dym_func(L, -1);
    dym_delete_func(func);
    return 0;
}
#define VAL_TO_LUA_RESULT(ft, t)\
case ft:{\
    t val;\
    hffi_value_get_##t(cif->out, &val);\
    lua_pushinteger(L, val);\
    return 1;\
}break;

#define VAL_TO_LUA_RESULT_F(ft, t)\
case ft:{\
    t val;\
    hffi_value_get_##t(cif->out, &val);\
    lua_pushnumber(L, val);\
    return 1;\
}break;

static int xffi_dym_func_call(lua_State *L){
    //func, cif
    dym_func* func = get_ref_dym_func(L, 1);
    hffi_cif* cif = get_ref_hffi_cif(L, -1);
    hffi_cif_call(cif, func->func_ptr);
    hffi_struct* stru = hffi_value_get_struct(cif->out);
    if(stru != NULL){
        hffi_struct_ref(stru, 1);
        push_ptr_hffi_struct(L, stru);
        return 1;
    }
    //cast to lua
    switch (cif->out->type->base_ffi_type) {
    case HFFI_TYPE_VOID:{return 0;}break;
//    case HFFI_TYPE_SINT8:{
//        sint8 val;
//        hffi_value_get_sint8(cif->out, &val);
//        lua_pushinteger(L, val);
//        return 1;
//    }break;
VAL_TO_LUA_RESULT(HFFI_TYPE_SINT8, sint8)
VAL_TO_LUA_RESULT(HFFI_TYPE_UINT8, uint8)
VAL_TO_LUA_RESULT(HFFI_TYPE_SINT16, sint16)
VAL_TO_LUA_RESULT(HFFI_TYPE_UINT16, uint16)
VAL_TO_LUA_RESULT(HFFI_TYPE_SINT32, sint32)
VAL_TO_LUA_RESULT(HFFI_TYPE_INT, sint32)
VAL_TO_LUA_RESULT(HFFI_TYPE_UINT32, uint32)
VAL_TO_LUA_RESULT(HFFI_TYPE_SINT64, sint64)
VAL_TO_LUA_RESULT(HFFI_TYPE_UINT64, uint64)

VAL_TO_LUA_RESULT_F(HFFI_TYPE_FLOAT, float)
VAL_TO_LUA_RESULT_F(HFFI_TYPE_DOUBLE, double)
    }
    //default push value
    hffi_value_ref(cif->out, 1);
    push_ptr_hffi_value(L, cif->out);
    return 1;
}
static const luaL_Reg g_dym_func_Methods[] = {
    {"__gc", xffi_dym_func_gc},
    {"call", xffi_dym_func_call},
    {NULL, NULL}
};
//------------------ dym_lib ----------------------------

static int xffi_dym_lib_gc(lua_State *L){
    dym_lib* lib = get_ref_dym_lib(L, -1);
    dym_delete_lib(lib);
    return 0;
}
static int xffi_dym_lib_index(lua_State *L){
    // lib , name
    dym_lib* lib = get_ref_dym_lib(L, 1);
    dym_func* func = dym_lib_get_function(lib, luaL_checkstring(L, 2));
    if(func == NULL){
        lua_getuservalue(L, 1);
        return luaL_error(L, "can't find function(%s) for lib(%s)", lua_tostring(L, 2), lua_tostring(L, -1));
    }
    dym_func_ref(func, 1);
    push_ptr_dym_func(L, func);
    return 1;
}
static int xffi_dym_lib_toString(lua_State *L){
     //dym_lib* lib = get_ref_dym_lib(L, 1);
     lua_getuservalue(L, 1);
     return 1;
}

static const luaL_Reg g_dym_lib_Methods[] = {
    {"__gc", xffi_dym_lib_gc},
    {"__index", xffi_dym_lib_index},
    {"__tostring", xffi_dym_lib_toString},
    {NULL, NULL}
};

static int xffi_dym_lib_new(lua_State *L){
    luaL_checktype(L, -1, LUA_TSTRING);
    dym_lib* lib = dym_new_lib(lua_tostring(L, -1));
    push_ptr_dym_lib(L, lib);
    lua_pushvalue(L, -2);      // str, lib, str
    lua_setuservalue(L, -2);   // str, lib
    return 1;
}
//--------------------- cif -------------------

static int xffi_cif_gc(lua_State *L){
    hffi_cif* cif = get_ref_hffi_cif(L, 1);
    hffi_delete_cif(cif);
    return 0;
}

static const luaL_Reg g_hffi_cif_Methods[] = {
    {"__gc", xffi_cif_gc},
    {NULL, NULL}
};

static inline hffi_value* __get_value(lua_State *L, int idx){
    if(lua_type(L, idx) == LUA_TNUMBER){
        return hffi_new_value_raw_type(lua_tointeger(L, idx));
    }else if(luaL_testudata(L, idx, __STR(hffi_struct)) != 0){
        hffi_struct* _struct = get_ref_hffi_struct(L, idx);
        return hffi_new_value_struct(_struct);
        //how to support struct pointer? recommend use value.
    }else if(luaL_testudata(L, idx, __STR(hffi_value)) != 0){
        hffi_value* val = get_ref_hffi_value(L, idx);
        hffi_value_ref(val, 1);
        return val;
    }else{
        return NULL;
    }
}
//cif { ret = pointer; pointer, sint, uint32 }
static int xffi_cif(lua_State *L){
    //ret_type, tab_param_types.
    luaL_checktype(L, 1, LUA_TTABLE);
    int len = lua_rawlen(L, 1);
    //abi
    int abi = FFI_DEFAULT_ABI;
    if(lua_getfield(L, 1, "abi") != LUA_TNIL){
        abi = lua_tointeger(L, -1);
    }
    //ret
    hffi_value* ret_val;
    if(lua_getfield(L, 1, "ret") != LUA_TNIL){
        ret_val = __get_value(L, -1);
        if(ret_val == NULL){
             return luaL_error(L, "unsupport cif type. type must be (sint8,hffi_struct,hffi_value)");
        }
    }else{
        ret_val = hffi_new_value_raw_type(HFFI_TYPE_VOID);
    }
    lua_pop(L, 2);//pop abi and ret.
    //params
    hffi_value* val;
    array_list* params = array_list_new(12, 0.75f);
    for(int i = 0 ;i < len ; i ++){
        lua_rawgeti(L, 1, i+1);
        //type: int, struct, value?
        val = __get_value(L, -1);
        if(val != NULL){
            array_list_add(params, val);
        }else{
            array_list_delete2(params, value_delete);
            hffi_delete_value(ret_val);
            return luaL_error(L, "unsupport cif type. type must be (sint8,hffi_struct,hffi_value)");
        }
        lua_pop(L, 1);
    }
    //build cif
    char _m[128];
    char* msg[1];
    msg[0] = _m;
    hffi_cif* cif = hffi_new_cif(abi, params, ret_val, msg);
    if(cif == NULL){
        array_list_delete2(params, value_delete);
        hffi_delete_value(ret_val);
        return luaL_error(L, "%s", msg);
    }
    push_ptr_hffi_cif(L, cif);
    return 1;
}
//---------------------------- ffi -----------------------
static int xffi_defines(lua_State *L){
#define reg_t(name, type)\
    lua_pushinteger(L, type);\
    lua_setglobal(L, name);

    const BasePair *lib;
    for (lib = _BASE_PAIRS; lib->name; lib++) {
        reg_t(lib->name, lib->type);
    }
#undef reg_t
    return 0;
}

static int xffi_undefines(lua_State *L){
#define unreg_t(name)\
    lua_pushnil(L);\
    lua_setglobal(L, name);

    const BasePair *lib;
    for (lib = _BASE_PAIRS; lib->name; lib++) {
        unreg_t(lib->name);
    }
#undef unreg_t
    return 0;
}

//----------------------------------------
static inline void setfield_function(lua_State* L,
                              const char key[], lua_CFunction value) {
    lua_pushcfunction(L, value);
    lua_setfield(L, -2, key);
}
LUAMOD_API void register_ffi(lua_State *L){
    REG_CLASS(L, dym_lib);
    REG_CLASS(L, dym_func);
    REG_CLASS(L, hffi_value);
    REG_CLASS(L, hffi_struct);
    REG_CLASS(L, hffi_cif);

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "ffi");
    // the ffi table is still on top
    setfield_function(L, "defines", xffi_defines);
    setfield_function(L, "undefines", xffi_undefines);

    setfield_function(L, "loadLib", xffi_dym_lib_new);
    setfield_function(L, "struct", xffi_struct_new);   
    setfield_function(L, "cif", xffi_cif);
    setfield_function(L, "newValue", xffi_value_new);
    lua_pop(L, 1);
}
