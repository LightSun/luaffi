
#include <errno.h>
#include "hffi_lua.h"
#include "hffi.h"
#include "dym_loader.h"
#include "h_list.h"
#include "h_alloctor.h"
#include "h_array.h"
#include "h_string.h"

#include "lua_utils.h"
#include "h_float_bits.h"

#define __N_VAL INT_MIN

#define REG_CLASS(L, C)                             \
    do {                                            \
        luaL_newmetatable(L, __STR(C));      \
        lua_pushvalue(L, -1);                       \
        lua_setfield(L, -2, "__index");             \
        luaL_setfuncs(L, g_##C##_Methods, 0);        \
        lua_pop(L, 1); /* pop off the meta-table */ \
    } while (0)

#define push_ptr(T) \
int push_ptr_##T(lua_State* L, T* ptr) {\
    *(T**)lua_newuserdata(L, sizeof(T*)) = ptr;\
    luaL_getmetatable(L, __STR(T));\
    lua_setmetatable(L, -2);\
    return 1;\
}
#define get_ptr(T)\
T* get_ptr_##T(lua_State* L, int index) {\
    return *(T**)luaL_checkudata(L, index, __STR(T));\
}

#define DEF_HFFI_PUSH_GET(t) \
push_ptr(t)\
get_ptr(t)

DEF_HFFI_PUSH_GET(dym_lib)
DEF_HFFI_PUSH_GET(dym_func)
DEF_HFFI_PUSH_GET(hffi_value)
DEF_HFFI_PUSH_GET(hffi_struct)
DEF_HFFI_PUSH_GET(hffi_smtype)
DEF_HFFI_PUSH_GET(harray)
DEF_HFFI_PUSH_GET(hffi_closure)

typedef struct BasePair{
    const char* name;
    sint8 type;
}BasePair;
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
    {"int", HFFI_TYPE_INT},
    //
    {"uint", HFFI_TYPE_UINT32},
    {"size_t", HFFI_TYPE_UINT32},
    //----- internal types -------
    {"array", HFFI_TYPE_HARRAY},
    {"array_ptr", HFFI_TYPE_HARRAY_PTR},
    {"struct", HFFI_TYPE_STRUCT},
    {"struct_ptr", HFFI_TYPE_STRUCT_PTR},
    {NULL, 0},
};

#define __INDEX_METHOD(name, func_lua)\
if(strcmp(fun_name, name) == 0){\
    lua_pushvalue(L, 1);\
    lua_pushcclosure(L, func_lua, 1);\
    return 1;\
}

#define __STRUCT_GET_MEMBER_INDEX(tab_idx, count, fun_name)\
int index = -1;\
{array_list* sm_names = (array_list*)hlua_get_light_uservalue(L, tab_idx);\
char* tmp_name;\
for(int i = count - 1 ; i >= 0 ; i --){\
    tmp_name = array_list_get(sm_names, i);\
    if(tmp_name != NULL && strcmp((char*)tmp_name, fun_name) == 0){\
        index = i;\
        break;\
    }\
}}

/**
---------- harray->hffi_smtype->hffi_value-> hffi_struct-> hffi_cif
---------- dym_lib->dym_func -------
  */
//-------------- share funcs --------------
static void string_delete(void* d){
    if(d) FREE(d);
}
static hffi_value* __get_value(lua_State *L, int idx);
//------------------- harray --------------------

#define _harray_index_impl_int(hffi_t, type)\
case hffi_t:{\
    union harray_ele ele;\
    if(harray_geti(arr, index, &ele)== HFFI_STATE_OK){\
        lua_pushinteger(L, ele._##type);\
        return 1;\
    }\
    return 0;\
}break;

#define _harray_index_impl_f(hffi_t, type)\
case hffi_t:{\
    union harray_ele ele;\
    if(harray_geti(arr, index, &ele)== HFFI_STATE_OK){\
        lua_pushnumber(L, ele._##type);\
        return 1;\
    }\
    return 0;\
}break;

#define _harray_newindex_impl_int(hffi_t, type)\
case hffi_t:{\
    union harray_ele ele;\
    ele._##type = (type)lua_tointeger(L, -1);\
    if(harray_seti(arr, index, &ele) == HFFI_STATE_OK){\
       return 0;\
    }\
    return luaL_error(L, "wrong index.");\
}break;

#define _harray_newindex_impl_f(hffi_t, type)\
case hffi_t:{\
    union harray_ele ele;\
    ele._##type = (type)lua_tonumber(L, -1);\
    if(harray_seti(arr, index, &ele) == HFFI_STATE_OK){\
       return 0;\
    }\
    return luaL_error(L, "wrong index.");\
}break;

//-1, is value
static inline int __harray_set_vals(lua_State* L, harray* arr, int index){
    //int ot float
    switch (arr->hffi_t) {
    _harray_newindex_impl_int(HFFI_TYPE_SINT8, sint8)
    _harray_newindex_impl_int(HFFI_TYPE_UINT8, uint8)
    _harray_newindex_impl_int(HFFI_TYPE_SINT16, sint16)
    _harray_newindex_impl_int(HFFI_TYPE_UINT16, uint16)
    _harray_newindex_impl_int(HFFI_TYPE_SINT32, sint32)
    _harray_newindex_impl_int(HFFI_TYPE_UINT32, uint32)
    _harray_newindex_impl_int(HFFI_TYPE_SINT64, sint64)
    _harray_newindex_impl_int(HFFI_TYPE_UINT64, uint64)
    _harray_newindex_impl_int(HFFI_TYPE_INT, sint32)

    _harray_newindex_impl_f(HFFI_TYPE_FLOAT, float)
    _harray_newindex_impl_f(HFFI_TYPE_DOUBLE, double)

    case HFFI_TYPE_HARRAY:
    case HFFI_TYPE_HARRAY_PTR:{
        union harray_ele ele;
        ele._extra = luaL_checkudata(L, -1, __STR(harray));
        if(harray_seti(arr, index, &ele) == HFFI_STATE_OK){
           return 0;
        }
        return luaL_error(L, "wrong index or data.");
    }break;
    case HFFI_TYPE_STRUCT_PTR:
    case HFFI_TYPE_STRUCT:{
        union harray_ele ele;
        ele._extra = luaL_checkudata(L, -1, __STR(hffi_struct));
        if(harray_seti(arr, index, &ele) == HFFI_STATE_OK){
           return 0;
        }
        return luaL_error(L, "wrong index or data.");
    }break;
    }
    return 0;
}

static int xffi_harray_gc(lua_State* L){
    harray* arr = get_ptr_harray(L, 1);
    hlua_delete_light_uservalue(L, -1);
    harray_delete(arr);
    return 0;
}
static int xffi_harray_newindex(lua_State* L){
    //tab, i, val(single or table)
    harray* arr = get_ptr_harray(L, -3);
    int index = luaL_checkinteger(L, -2);
    //check if is N. set all element to it.
    if(index == __N_VAL){
        //-1 is val
        for(int i = 0 ; i < arr->ele_count ; i ++){
            __harray_set_vals(L, arr, i);
        }
        HFFI_SYNC_PARENT_I(arr)
        return 0;
    }
    if(index < 0) index = arr->ele_count + index;
    if(lua_type(L, -1) == LUA_TTABLE){
        int c = lua_rawlen(L, -1);
        if(c > arr->ele_count - index){
            c = arr->ele_count - index;
        }
        for(int i = 0 ; i < c ; i ++){
            //element of table
            lua_rawgeti(L, -1, i + 1);
            __harray_set_vals(L, arr, i + index); //data index in c
            lua_pop(L, 1);
        }
        HFFI_SYNC_PARENT_I(arr)
        return 0;
    }else{
        //int ot float
        __harray_set_vals(L, arr, index);
        HFFI_SYNC_PARENT_I(arr)
        return 0;
    }
    return luaL_error(L, "wrong element type for __newindex(harray)!");
}
//when access harray from struct, we may need sync data to struct.
static int __harray_func_set(lua_State* L){
    //index, data
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_insert(L, 1);
    //ud, index, data
    xffi_harray_newindex(L);
    return 0;
}
static int __harray_func_copy(lua_State* L){
    harray* arr = get_ptr_harray(L, lua_upvalueindex(1));
    return push_ptr_harray(L, harray_copy(arr));
}
static int __harray_func_eletype(lua_State* L){
    harray* arr = get_ptr_harray(L, lua_upvalueindex(1));
    lua_pushnumber(L, arr->hffi_t);
    return 1;
}
static int __harray_func_elesize(lua_State* L){
    harray* arr = get_ptr_harray(L, lua_upvalueindex(1));
    lua_pushnumber(L, arr->data_size/ arr->ele_count);
    return 1;
}
static int __harray_func_addr(lua_State* L){
    harray* arr = get_ptr_harray(L, lua_upvalueindex(1));
    lua_pushfstring(L, "%p", arr);
    return 1;
}
static int __harray_func_hasData(lua_State* L){
    harray* arr = get_ptr_harray(L, lua_upvalueindex(1));
    lua_pushboolean(L, arr->data != NULL);
    return 1;
}

static const luaL_Reg g_harray_str_Methods[] = {
    {"set", __harray_func_set},
    {"hasData", __harray_func_hasData},
    {"addr", __harray_func_addr},
    {"copy", __harray_func_copy},
    {"eletype", __harray_func_eletype},
    {"elesize", __harray_func_elesize},
    {NULL, NULL}
};
static int xffi_harray_index(lua_State* L){
    //may be eg: arr.set(1, {5, 6})
    if(lua_type(L, 2) == LUA_TSTRING){
        const char* fun_name = lua_tostring(L, 2);
        //array, str
        const luaL_Reg *lib;
        for (lib = g_harray_str_Methods; lib->name; lib++) {
            if(strcmp(lib->name, fun_name) == 0){
                lua_pushvalue(L, 1);
                lua_pushcclosure(L, lib->func, 1);
                return 1;
            }
        }
        return luaL_error(L, "unsupport method('%s') for harray.", fun_name);
    }
    //tab, i
    harray* arr = get_ptr_harray(L, 1);

    int index = luaL_checkinteger(L, 2);
    if(index < 0) index = arr->ele_count + index;
    switch (arr->hffi_t) {
     _harray_index_impl_int(HFFI_TYPE_SINT8, sint8)
     _harray_index_impl_int(HFFI_TYPE_UINT8, uint8)
     _harray_index_impl_int(HFFI_TYPE_SINT16, sint16)
     _harray_index_impl_int(HFFI_TYPE_UINT16, uint16)
     _harray_index_impl_int(HFFI_TYPE_SINT32, sint32)
     _harray_index_impl_int(HFFI_TYPE_UINT32, uint32)
     _harray_index_impl_int(HFFI_TYPE_SINT64, sint64)
     _harray_index_impl_int(HFFI_TYPE_UINT64, uint64)
     _harray_index_impl_int(HFFI_TYPE_INT, sint32)

     _harray_index_impl_f(HFFI_TYPE_FLOAT, float)
     _harray_index_impl_f(HFFI_TYPE_DOUBLE, double)

     case HFFI_TYPE_HARRAY:
     case HFFI_TYPE_HARRAY_PTR:{
         union harray_ele ele;
         if(harray_geti(arr, index, &ele) == HFFI_STATE_OK){
             harray_ref((harray*)ele._extra, 1);
            return push_ptr_harray(L, (harray*)ele._extra);
         }
         return 0;
     }break;
     case HFFI_TYPE_STRUCT_PTR:
     case HFFI_TYPE_STRUCT:{
         union harray_ele ele;
         if(harray_geti(arr, index, &ele) == HFFI_STATE_OK){
             hffi_struct_ref((hffi_struct*)ele._extra, 1);
            return push_ptr_hffi_struct(L, (hffi_struct*)ele._extra);
         }
         return 0;
     }break;

    }
    return luaL_error(L, "unsupport data type for get index data from harray.");
}

static int xffi_harray_len(lua_State* L){
    harray* arr = get_ptr_harray(L, 1);
    lua_pushinteger(L, arr->ele_count);
    return 1;
}
static int xffi_harray_tostring(lua_State* L){
    harray* arr = get_ptr_harray(L, 1);
    hstring* hs = hstring_new();
    harray_dump(arr, hs);
    lua_pushstring(L, hstring_tostring(hs));
    hstring_delete(hs);
    return 1;
}
static int xffi_harray_eq(lua_State* L){
    harray* arr = get_ptr_harray(L, 1);
    harray* arr2 = get_ptr_harray(L, 2);
    int ok = harray_eq(arr, arr2) == HFFI_STATE_OK;
    lua_pushboolean(L, ok);
    return 1;
}

static int xffi_harray_new(lua_State* L){
    if(lua_gettop(L) < 1){
        return luaL_error(L, "create harray need [type & count]/[harray...]/[struct...].");
    }
    switch (lua_type(L, 1)) {
    case LUA_TNUMBER:{
        if(lua_gettop(L) < 2){
            return luaL_error(L, "create harray need [type & count]/[harray...]/[struct...].");
        }
        int type = (sint8)luaL_checkinteger(L, 1);
        harray* arr;
        if(lua_type(L, 2) == LUA_TNUMBER){
            //judge if has no_data flag.
            int has_data = 1;
            if(lua_type(L,3) == LUA_TBOOLEAN){
                has_data = lua_toboolean(L, 3);
                lua_pop(L, 1);
            }
            //type, count
            int c = luaL_checkinteger(L, 2);
            arr = has_data ? harray_new(type, c) : harray_new_nodata(type, c);
        }else{
            //type , tab(base/harray/struct)
            luaL_checktype(L, 2, LUA_TTABLE);
            int c = lua_rawlen(L, 2);
            arr = harray_new(type, c);
            if(arr != NULL){
                for(int i = 0 ; i < c; i ++){
                    lua_rawgeti(L, 2, i + 1); //type, tab, ele
                    __harray_set_vals(L, arr, i);
                    lua_pop(L, 1);
                }
            }
        }
        if(arr == NULL){
            return luaL_error(L, "create array failed by wrong type = %d", type);
        }
        return push_ptr_harray(L, arr);
    }break;
    case LUA_TTABLE:{
        harray* arr = hlua_new_harray_from_table(L, 1, get_ptr_hffi_struct, get_ptr_harray);
        if(arr){
            return push_ptr_harray(L, arr);
        }
    }break;
    case LUA_TSTRING:{
        //string as char array
        const char* str = lua_tostring(L, 1);      
        return push_ptr_harray(L, harray_new_chars(str));
    }break;
    }

    return luaL_error(L, "unsupport args for create harray. need [type & count]/[harray...]/[struct...].");
}

static const luaL_Reg g_harray_Methods[] = {
    {"__gc", xffi_harray_gc},
    {"__index", xffi_harray_index},
    {"__newindex", xffi_harray_newindex},
    {"__len", xffi_harray_len},
    {"__eq", xffi_harray_eq},
    {"__tostring", xffi_harray_tostring},
    {NULL, NULL}
};

//------------------ smtype ----------------------

static int xffi_smtype_gc(lua_State* L){
    hffi_smtype* hs = get_ptr_hffi_smtype(L, 1);
    hffi_delete_smtype(hs);
    return 0;
}
static int __smtype_cppy(lua_State* L){
    hffi_smtype* hs = get_ptr_hffi_smtype(L, lua_upvalueindex(1));
    return push_ptr_hffi_smtype(L, hffi_smtype_cpoy(hs));
}

static int xffi_smtype_index(lua_State* L){
    //current support copy
    if(lua_type(L, 2) == LUA_TSTRING){
        const char* fun_name = lua_tostring(L, 2);
        __INDEX_METHOD("copy", __smtype_cppy)
        return luaL_error(L, "unsupport method('%s') for smtype.", fun_name);
    }
    return luaL_error(L, "smtype doesn't support int index.");
}

static const luaL_Reg g_hffi_smtype_Methods[] = {
    {"__gc", xffi_smtype_gc},
    {"__index", xffi_smtype_index},
    {NULL, NULL}
};
static int xffi_smtype_new(lua_State* L){
    //type/smtypes
    //harray/hffi_struct [, bool(ptr or not)]
    //value
    switch (lua_type(L, 1)) {
    case LUA_TNUMBER:{
        return push_ptr_hffi_smtype(L, hffi_new_smtype((sint8)luaL_checkinteger(L, 1)));
    }break;
    case LUA_TTABLE:{
        int c = lua_rawlen(L, 1);
        array_list* list = array_list_new2(8);
        for(int i = 0 ; i < c ; i ++){
            lua_rawgeti(L, 1, i + 1);
            if(luaL_testudata(L, -1, __STR(hffi_smtype))){
                array_list_add(list, get_ptr_hffi_smtype(L, -1));
            }else{
                array_list_delete2(list, NULL);
                return luaL_error(L, "for create smtype. table element must be smtype.");
            }
            lua_pop(L, 1);
        }
        hffi_smtype* smtype = hffi_new_smtype_members(list);
        array_list_delete2(list, NULL);
        return push_ptr_hffi_smtype(L, smtype);
    }break;
    case LUA_TUSERDATA:{
        int asPtr = 0;
        if(lua_gettop(L) >= 2){
            asPtr = lua_toboolean(L, 2);
        }
        if(luaL_testudata(L, 1, __STR(hffi_struct))){
            hffi_struct* hs = get_ptr_hffi_struct(L, 1);
            hffi_smtype* smtype = asPtr ? hffi_new_smtype_struct_ptr(hs) : hffi_new_smtype_struct(hs);
            return push_ptr_hffi_smtype(L, smtype);
        }else if(luaL_testudata(L, 1, __STR(harray))){
            harray* hs = get_ptr_harray(L, 1);
            hffi_smtype* smtype = asPtr ? hffi_new_smtype_harray_ptr(hs) : hffi_new_smtype_harray(hs);
            return push_ptr_hffi_smtype(L, smtype);
        }else if(luaL_testudata(L, 1, __STR(hffi_value))){
            hffi_value* hs = get_ptr_hffi_value(L, 1);
            return push_ptr_hffi_smtype(L, hffi_value_to_smtype(hs));
        }
    }break;
    }
    return luaL_error(L, "create smtype support 3 formats: \n1, base_type(int). "
                         "2, smtypes(table). 3, userdata(struct/array) + bool(as ptr or not).");
}
//----------------- struct ---------------
static void __delete_smnames(void* d){
    array_list* list = d;
    array_list_delete2(list, string_delete);
}
//static inline int get_base_type(const char* name){
//    const BasePair *lib;
//    for (lib = _BASE_PAIRS; lib->name; lib++) {
//        if(strcmp(lib->name, name) == 0){
//            return lib->type;
//        }
//    }
//    return -1;
//}
//SDL_KeyboardEvent = ffi.struct {
//  uint32, "type";
//  uint32, "timestamp";
//  uint32, "windowID";
//  uint8, "state";
//  sint8, sint8; //just for padding.
//  uint8, "repeat";
//  uint8; uint8;  --padding
//  SDL_Keysym, "keysym";
//  SDL_Keysym, true, "keysym2";
//}
static int xffi_struct_new(lua_State *L){
    luaL_checktype(L, 1, LUA_TTABLE);
    if(lua_rawlen(L, 1) == 0) return 0;
    //options
    int nodata = 0;
    int freeData = 1;
    int abi = FFI_DEFAULT_ABI;
    nodata = hlua_get_boolean(L, 1, "no_data", nodata);
    abi = hlua_get_int(L, 1, "abi", abi);
    freeData = hlua_get_boolean(L, 1, "free_data", freeData);
    //start build param
    array_list* sm_list = array_list_new_simple();
    array_list* sm_names = array_list_new_simple();

    if(build_smtypes(L, sm_list, sm_names, get_ptr_hffi_struct, get_ptr_harray,
                     get_ptr_hffi_smtype, get_ptr_hffi_closure, get_ptr_hffi_value) == HFFI_STATE_FAILED){
        array_list_delete2(sm_list, list_travel_smtype_delete);
        array_list_delete2(sm_names, string_delete);
        return luaL_error(L, "build struct met unsupport data type.");
    }

    //msg
    char _m[128];
    char* msg[1];
    msg[0] = _m;
    //create struct
    hffi_struct* _struct;
    if(nodata){
        _struct = hffi_new_struct_from_list_nodata(abi, sm_list, msg);
    }else{
        _struct = hffi_new_struct_from_list2(abi, sm_list, msg);
    }
    if(_struct == NULL){
        array_list_delete2(sm_list, list_travel_smtype_delete);
        array_list_delete2(sm_names, string_delete);
        return luaL_error(L, "create struct failed by '%s'", msg[0]);
    }
    _struct->should_free_data = freeData;
    array_list_delete2(sm_list, list_travel_smtype_delete);
    push_ptr_hffi_struct(L, _struct);
    hlua_push_light_uservalue(L, -1, sm_names, __delete_smnames);
    return 1;
}
static int xffi_struct_gc(lua_State *L){
    hffi_struct* _struct = get_ptr_hffi_struct(L, -1);
    hlua_delete_light_uservalue(L, -1);
    hffi_delete_struct(_struct);
    return 0;
}
static int __struct_copy(lua_State *L){
    hffi_struct* hs = get_ptr_hffi_struct(L, lua_upvalueindex(1));
    lua_pushvalue(L, lua_upvalueindex(1));
    push_ptr_hffi_struct(L, hffi_struct_copy(hs));
    hlua_share_light_uservalue(L, 1, -1);
    return 1;
}

static int __struct_getOffsets(lua_State *L){
    hffi_struct* hs = get_ptr_hffi_struct(L, lua_upvalueindex(1));
    size_t* offsets = HFFI_STRUCT_OFFSETS(hs->type, hs->count);
    lua_newtable(L);
    for(int i = 0 ; i < hs->count ; i++){
        lua_pushinteger(L, offsets[i]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}
static int __struct_typeSize(lua_State *L){
    hffi_struct* hs = get_ptr_hffi_struct(L, lua_upvalueindex(1));
    lua_pushinteger(L, hs->type->size);
    return 1;
}
static int __struct_typeAlignment(lua_State *L){
    hffi_struct* hs = get_ptr_hffi_struct(L, lua_upvalueindex(1));
    lua_pushinteger(L, hs->type->alignment);
    return 1;
}
static int __struct_memberType(lua_State *L){
    hffi_struct* hs = get_ptr_hffi_struct(L, lua_upvalueindex(1));
    int index = luaL_checkinteger(L, 1);
    if(index >= hs->count) return 0;
    return hs->hffi_types[index];
}
static int __struct_hasData(lua_State *L){
    hffi_struct* hs = get_ptr_hffi_struct(L, lua_upvalueindex(1));
    lua_pushboolean(L, hs->data != NULL);
    return 1;
}
static int __struct_ptrToNull(lua_State *L){
    hffi_struct* hs = get_ptr_hffi_struct(L, lua_upvalueindex(1));
    hffi_struct_set_all(hs, NULL);
    return 0;
}
static int __struct_ptrValue(lua_State *L){
    lua_pushvalue(L, lua_upvalueindex(1));
    hffi_struct* hs = get_ptr_hffi_struct(L, -1);
    lua_insert(L, 1);
    //struct, index, ffi_t
    if(lua_type(L, 2) == LUA_TSTRING){
         const char* fun_name = lua_tostring(L, 2);
        __STRUCT_GET_MEMBER_INDEX(1, hs->count, fun_name);
        if(index >= 0){
            int target_type = luaL_checkinteger(L, 3);
            hffi_value* val = hffi_struct_to_ptr_value(hs, index, target_type);
            if(val == NULL){
                return luaL_error(L, "struct member type mismatch for 'ptrValue(...)'.");
            }
            return push_ptr_hffi_value(L, val);
        }
        return luaL_error(L, "wrong field name of struct for 'ptrValue(...)'.");
    }
    int index = luaL_checkinteger(L, 2);
    int target_type = luaL_checkinteger(L, 3);
    hffi_value* val = hffi_struct_to_ptr_value(hs, index, target_type);
    if(val == NULL){
        return luaL_error(L, "struct member type mismatch for 'ptrValue(...)'.");
    }
    return push_ptr_hffi_value(L, val);
}
static int xffi_struct_index(lua_State *L){
    hffi_struct* hs = get_ptr_hffi_struct(L, 1);
    if(lua_type(L, 2) == LUA_TSTRING){
        const char* fun_name = lua_tostring(L, 2);
        __INDEX_METHOD("copy", __struct_copy)
        __INDEX_METHOD("hasData", __struct_hasData)
        __INDEX_METHOD("getMembertype", __struct_memberType)
        __INDEX_METHOD("getOffsets", __struct_getOffsets)
        __INDEX_METHOD("getTypeSize", __struct_typeSize)
        __INDEX_METHOD("getTypeAlignment", __struct_typeAlignment)
        __INDEX_METHOD("ptrToNull", __struct_ptrToNull)
        __INDEX_METHOD("ptrValue", __struct_ptrValue)
        //check member name as method
        __STRUCT_GET_MEMBER_INDEX(1, hs->count, fun_name);
        //int index = hlua_get_struct_member_index(L, 1, hs->count, fun_name);
        if(index >= 0){
            //make index replace name
            lua_pushinteger(L, index);
            lua_replace(L, 2);
            return xffi_struct_index(L);
        }
        return luaL_error(L, "unsupport method('%s') for struct.", fun_name);
    }
    int index = luaL_checkinteger(L, 2);
    if(index < 0) index = hs->count + index;
    if(index >= hs->count){
        return 0;
    }
    sint8 ffi_t = hs->hffi_types[index];
    switch (ffi_t) {
    case HFFI_TYPE_POINTER:{
        //unknown. reguard as a simple ptr value.
        hffi_value* val = hffi_new_value_ptr_nodata(HFFI_TYPE_VOID);
        val->ptr = hffi_struct_get_pointer(hs, index);
        val->should_free_ptr = 0;
        return push_ptr_hffi_value(L, val);
    }break;

    case HFFI_TYPE_FLOAT:{
        float num = 0;
        hffi_struct_get_base(hs, index, HFFI_TYPE_FLOAT, &num);
        lua_pushnumber(L, num);
        return 1;
    }break;

    case HFFI_TYPE_DOUBLE:{
        double num = 0;
        hffi_struct_get_base(hs, index, HFFI_TYPE_DOUBLE, &num);
        lua_pushnumber(L, num);
        return 1;
    }break;

    case HFFI_TYPE_STRUCT:
    case HFFI_TYPE_STRUCT_PTR:{
        hffi_struct* hstr = hffi_struct_get_struct(hs, index);
        if(hstr != NULL){
             hffi_struct_ref(hstr, 1);
             push_ptr_hffi_struct(L, hstr);
             return 1;
        }
    }break;

    case HFFI_TYPE_HARRAY:
    case HFFI_TYPE_HARRAY_PTR: {
        harray* arr = hffi_struct_get_harray(hs, index);
        if(arr != NULL){
             harray_ref(arr, 1);
             push_ptr_harray(L, arr);
             return 1;
        }
    }break;

    default:{
        lua_Integer num = 0; //must assign 0, or else cause bug
        if(hffi_struct_get_base(hs, index, HFFI_TYPE_INT, &num) == HFFI_STATE_OK){
            lua_pushinteger(L, num);
            return 1;
        }
    }break;
    }
    return 0;
}
static int xffi_struct_newindex(lua_State *L){
    //tab, index/string, val
    hffi_struct* hs = get_ptr_hffi_struct(L, 1);
    if(lua_type(L, 2) == LUA_TSTRING){
        const char* fun_name = lua_tostring(L, 2);
        __STRUCT_GET_MEMBER_INDEX(1, hs->count, fun_name);
        if(index >= 0){
            //make index replace name
            lua_pushinteger(L, index);
            lua_replace(L, 2);
            return xffi_struct_newindex(L);
        }
        return luaL_error(L, "only support struct member name as struct newindex method.");
    }
    int index = luaL_checkinteger(L, 2);
    if(index >= hs->count) return luaL_error(L, "index out of range. max index is %d, bit is %d",
                                             hs->count - 1, index);
    switch (hs->hffi_types[index]) {
    case HFFI_TYPE_POINTER:{
        return luaL_error(L, "struct doesn't support index pointer-type.");
    }break;

    case HFFI_TYPE_INT:
    case HFFI_TYPE_SINT8:
    case HFFI_TYPE_UINT8:
    case HFFI_TYPE_SINT16:
    case HFFI_TYPE_UINT16:
    case HFFI_TYPE_SINT32:
    case HFFI_TYPE_UINT32:
    case HFFI_TYPE_SINT64:
    case HFFI_TYPE_UINT64:{
        lua_Integer num = luaL_checkinteger(L, 3);
        hffi_struct_set_base(hs, index, HFFI_TYPE_INT, &num);
        break;
    }break;

    case  HFFI_TYPE_FLOAT:{
        float num = (float)luaL_checknumber(L, 3);
        hffi_struct_set_base(hs, index, HFFI_TYPE_FLOAT, &num);
        break;
    }break;
    case  HFFI_TYPE_DOUBLE:{
        double num = (double)luaL_checknumber(L, 3);
        hffi_struct_set_base(hs, index, HFFI_TYPE_DOUBLE, &num);
        break;
    }break;

    case HFFI_TYPE_HARRAY:
    case HFFI_TYPE_HARRAY_PTR:{
        harray* arr = get_ptr_harray(L, 3);
        if(hffi_struct_set_harray(hs, index, arr) == HFFI_STATE_OK){
            break;
        }
        return luaL_error(L, "set array(member) for struct failed.");
    }break;

    case HFFI_TYPE_STRUCT:
    case HFFI_TYPE_STRUCT_PTR:{
        hffi_struct* new_hs = get_ptr_hffi_struct(L, 3);
        if(hffi_struct_set_struct(hs, index, new_hs) == HFFI_STATE_OK){
            break;
        }
        return luaL_error(L, "set struct(member) for struct failed.");
    }break;

    default:
        return luaL_error(L, "wrong struct type = %d", hs->hffi_types[index]);
    }
    HFFI_SYNC_PARENT_I(hs)
    return 0;
}

static int xffi_struct_count(lua_State* L){
    hffi_struct* _struct = get_ptr_hffi_struct(L, -1);
    lua_pushinteger(L, _struct->count);
    return 1;
}
static int xffi_struct_tostring(lua_State* L){
    hffi_struct* hstr = get_ptr_hffi_struct(L, -1);
    hstring* hs = hstring_new();
    hffi_struct_dump(hstr, hs);
    lua_pushstring(L, hstring_tostring(hs));
    hstring_delete(hs);
    return 1;
}
static int xffi_struct_eq(lua_State* L){
     lua_pushboolean(L, hffi_struct_eq(get_ptr_hffi_struct(L, 1), get_ptr_hffi_struct(L, 2)) == HFFI_TRUE);
     return 1;
}

static const luaL_Reg g_hffi_struct_Methods[] = {
    {"__gc", xffi_struct_gc},
    {"__index", xffi_struct_index},
    {"__newindex", xffi_struct_newindex},
    {"__eq", xffi_struct_eq},
    {"__len", xffi_struct_count},
    {"__tostring", xffi_struct_tostring},
    {NULL, NULL}
};
//------------------ hffi_value ------------
static int xffi_value_ptr_new(lua_State *L){
    hffi_value* val;
    switch (lua_type(L, 1)) {
    case LUA_TNUMBER:{
        sint8 type = luaL_checkinteger(L, 1);
        int nodata = 1;
        if(lua_gettop(L) >= 2){
            nodata = lua_toboolean(L, 2);
        }
        val = nodata ? hffi_new_value_ptr_nodata(type) : hffi_new_value_ptr(type);
    }break;

    case LUA_TSTRING:{
        const char* str = lua_tostring(L, 1);
        harray* arr = harray_new_chars(str);
        harray_ref(arr, -1);
        val = hffi_new_value_harray_ptr(arr); //char* a
    }break;

    case LUA_TUSERDATA:{
        if(luaL_testudata(L, 1, __STR(hffi_struct))){
            val = hffi_new_value_struct_ptr(get_ptr_hffi_struct(L, 1));
        }else if(luaL_testudata(L, 1, __STR(harray))){
            val = hffi_new_value_harray_ptr(get_ptr_harray(L, 1));
        }else if(luaL_testudata(L, 1, __STR(hffi_closure))){
            val = hffi_new_value_closure(get_ptr_hffi_closure(L, 1));
        }else{
            return luaL_error(L, "unsupport userdata type for valuePtr(...)");
        }
    }break;
    default:
        return luaL_error(L, "unsupport userdata type for valuePtr(...)");
    }
    return push_ptr_hffi_value(L, val);
}

static int xffi_value_new(lua_State *L){
    // base_type[, val]
    // pointer, ptr_base_type, [val]
    // struct/harray [, is_pointer]
    // string[, is_pointer]
    hffi_value* val = NULL;
    switch (lua_type(L, 1)) {
    case LUA_TNUMBER:{
        sint8 type = (sint8)luaL_checkinteger(L, 1);       
        //ptr type
        if(type == HFFI_TYPE_POINTER){
            type = (sint8)luaL_checkinteger(L, 2);
            if(lua_gettop(L) >= 3){
                if(lua_type(L, 3) == LUA_TBOOLEAN){
                    //bool: indicate alloc val.ptr or not
                    if(lua_toboolean(L, 3)){
                        val = hffi_new_value_ptr_nodata(type);
                    }else{
                        val = hffi_new_value_ptr(type);
                    }
                }else{
                    //has val
                    if(type == HFFI_TYPE_FLOAT || type == HFFI_TYPE_DOUBLE){
                        lua_Number num = lua_tonumber(L, 3);
                        val = hffi_new_value_ptr2(type, &num);
                    }else{
                        lua_Integer num = lua_tointeger(L, 3);
                        val = hffi_new_value_ptr2(type, &num);
                    }
                }
            }else{
                //only pointer and type
                val = hffi_new_value_ptr(type);
            }
        }else{
            if(lua_gettop(L) >= 2){ //hasVal = 1;
                if(type == HFFI_TYPE_FLOAT || type == HFFI_TYPE_DOUBLE){
                    lua_Number num = lua_tonumber(L, 2);
                    val = hffi_new_value_raw_type2(type, &num);
                }else{
                    lua_Integer num = lua_tointeger(L, 2);
                    val = hffi_new_value_raw_type2(type, &num);
                }
            }else{
                val = hffi_new_value_raw_type(type);
            }
        }
        return push_ptr_hffi_value(L, val);
    }break;
    case LUA_TUSERDATA:{
        if(luaL_testudata(L, 1, __STR(hffi_struct))){
            if(lua_gettop(L) >= 2 && lua_toboolean(L, 2)){
                val = hffi_new_value_struct_ptr(get_ptr_hffi_struct(L, 1));
            }else{
                val = hffi_new_value_struct(get_ptr_hffi_struct(L, 1));
            }
            return push_ptr_hffi_value(L, val);
        }else if(luaL_testudata(L, 1, __STR(harray))){
            if(lua_gettop(L) >= 2 && lua_toboolean(L, 2)){
                val = hffi_new_value_harray_ptr(get_ptr_harray(L, 1));
            }else{
                val = hffi_new_value_harray(get_ptr_harray(L, 1));
            }
            return push_ptr_hffi_value(L, val);
        }else if(luaL_testudata(L, 1, __STR(hffi_closure))){
            val = hffi_new_value_closure(get_ptr_hffi_closure(L, 1));
            return push_ptr_hffi_value(L, val);
        }
    }break;
    case LUA_TSTRING:{
        const char* str = lua_tostring(L, 1);
        harray* arr = harray_new_chars(str);
        harray_ref(arr, -1);
        if(lua_gettop(L) >= 2 && lua_toboolean(L, 2)){
            val = hffi_new_value_harray_ptr(arr); //char* a
        }else{
            val = hffi_new_value_harray(arr); //char a[len]
        }
        return push_ptr_hffi_value(L, val);
    }break;  
    }
    return luaL_error(L, "new value only support: 'base_type[, val]', 'pointer,base_type[, val]', "
                         "'struct/harray[, is_pointer]', string[, is_pointer]");
}
static int xffi_value_gc(lua_State *L){
    hffi_value* val = get_ptr_hffi_value(L, 1);
    hffi_delete_value(val);
    return 0;
}
static int __hiff_value_get(lua_State *L){
    hffi_value* val = get_ptr_hffi_value(L, lua_upvalueindex(1));
    if(val->base_ffi_type == HFFI_TYPE_VOID){
        return 0;
    }
    hffi_struct* hs = hffi_value_get_struct(val);
    if(hs != NULL){
        hffi_struct_ref(hs, 1);
        return push_ptr_hffi_struct(L, hs);
    }
    harray* hy = hffi_value_get_harray(val);
    if(hy != NULL){
        harray_ref(hy, 1);
        return push_ptr_harray(L, hy);
    }
    //base type or base's one-level-ptr.
    int ffi_t = val->base_ffi_type;
    if(ffi_t == HFFI_TYPE_POINTER){
        ffi_t = val->pointer_base_type;
        if(ffi_t == HFFI_TYPE_VOID){
            return 0;
        }
    }
    if(ffi_t == HFFI_TYPE_FLOAT){
        float num = 0;
        hffi_value_get_base(val, &num);
        lua_pushnumber(L, num);
        return 1;
    }
    if(ffi_t == HFFI_TYPE_DOUBLE){
        lua_Number num = 0;
        hffi_value_get_base(val, &num);
        lua_pushnumber(L, num);
        return 1;
    }else{
        lua_Integer num = 0;
        hffi_value_get_base(val, &num);
        lua_pushinteger(L, num);
        return 1;
    }
    return 0;
}
static int __hiff_value_copy(lua_State *L){
     hffi_value* val = get_ptr_hffi_value(L, lua_upvalueindex(1));
     return push_ptr_hffi_value(L, hffi_value_copy(val));
}
static int __hiff_value_addr(lua_State *L){
     hffi_value* val = get_ptr_hffi_value(L, lua_upvalueindex(1));
     lua_pushfstring(L, "%p", (void*)val);
     return 1;
}
static int __hiff_value_hasData(lua_State *L){
     hffi_value* val = get_ptr_hffi_value(L, lua_upvalueindex(1));
     lua_pushboolean(L, hffi_value_hasData(val));
     return 1;
}
static int __hiff_value_as(lua_State *L){
     hffi_value* val = get_ptr_hffi_value(L, lua_upvalueindex(1));
     if(val->base_ffi_type != HFFI_TYPE_POINTER || !val->ptr){
         return luaL_error(L, "value.as(...) only used for pointer which has data!");
     }
     //harray, struct, closure.
     if(luaL_testudata(L, 1, __STR(hffi_struct))){
        hffi_struct* hs = get_ptr_hffi_struct(L, 1);
        hffi_struct_set_all(hs, val->ptr);
     }else if(luaL_testudata(L, 1, __STR(harray))){
        harray* arr = get_ptr_harray(L, 1);
        harray_set_all(arr, val->ptr);
     }else if(luaL_testudata(L, 1, __STR(hffi_closure))){
         hffi_closure* arr = get_ptr_hffi_closure(L, 1);
         hffi_closure_set_func_ptr(arr, val->ptr);
     }else{
         return luaL_error(L, "unsupport data-type of value.as(...)!");
     }
     return 0;
}
static int __hiff_value_add(lua_State *L){
    hffi_value* val = get_ptr_hffi_value(L, lua_upvalueindex(1));
    if(!hffi_is_base_type(val->base_ffi_type)){
        return luaL_error(L, "unsupport value type for add(...)");
    }
    if(lua_type(L, 1)== LUA_TNUMBER){
#define __XFFI_VALUE_ADD_NUMBER(ffi_t, type)\
case ffi_t:{\
    type num = (type)lua_tointeger(L, 1);\
    if(hffi_value_add(val, &num)){\
        hffi_value_ref(val, 1);\
        return push_ptr_hffi_value(L, val);\
    }\
}break;
    DEF_HFFI_BASE_SWITCH_INT(__XFFI_VALUE_ADD_NUMBER, val->base_ffi_type)
    if(val->base_ffi_type == HFFI_TYPE_FLOAT){
        float num = (float)lua_tonumber(L, 1);
        if(hffi_value_add(val, &num)){
            hffi_value_ref(val, 1);
            return push_ptr_hffi_value(L, val);
        }
    }else if(val->base_ffi_type == HFFI_TYPE_DOUBLE){
        double num = (double)lua_tonumber(L, 1);
        if(hffi_value_add(val, &num)){
            hffi_value_ref(val, 1);
            return push_ptr_hffi_value(L, val);
        }
    }
    }else if(luaL_testudata(L, 1, __STR(hffi_value))){
        hffi_value* val2 = get_ptr_hffi_value(L, 1);
        if(!hffi_is_base_type(val2->base_ffi_type)){
            return luaL_error(L, "unsupport value type for add(...)");
        }
#define __XFFI_VALUE_ADD_VALUE(ffi_t, type)\
case ffi_t:{\
    if(hffi_value_add(val, val2->ptr)){\
        hffi_value_ref(val, 1);\
        return push_ptr_hffi_value(L, val);\
    }\
}break;
       DEF_HFFI_BASE_SWITCH(__XFFI_VALUE_ADD_VALUE, val->base_ffi_type)
    }
    return luaL_error(L, "value add op only support number with base value type.");
}
static int __hiff_value_ptr_null(lua_State *L){
    hffi_value* val = get_ptr_hffi_value(L, lua_upvalueindex(1));
    val->ptr = NULL;
    return 0;
}
static int __hiff_value_ptr_value(lua_State *L){
    hffi_value* val = get_ptr_hffi_value(L, lua_upvalueindex(1));
    hffi_value* newVal = hffi_new_value_ptr_nodata(HFFI_TYPE_VOID);
    newVal->ptr = &val->ptr;
    //as this ptr is not alloc by self value.
    newVal->should_free_ptr = 0;
    return push_ptr_hffi_value(L, newVal);
}
static int __hiff_value_setPtr(lua_State *L){
    //array, offset
    hffi_value* val = get_ptr_hffi_value(L, lua_upvalueindex(1));
    if(val->ptr && val->should_free_ptr){
        return luaL_error(L, "valid value data can't call 'ptrOffset(...)'.");
    }
    if(val->base_ffi_type != HFFI_TYPE_POINTER){
        return luaL_error(L, "'value.setPtr(...)' can only used for pointer type.");
    }
    harray* arr = get_ptr_harray(L, 1);
    int offset = luaL_checkinteger(L, 2);
    val->ptr = arr->data + offset;
    return 0;
}

static int xffi_value_index(lua_State *L){
    if(lua_type(L, 2) == LUA_TSTRING){
        const char* fun_name = lua_tostring(L, 2);
        __INDEX_METHOD("copy", __hiff_value_copy);
        __INDEX_METHOD("get", __hiff_value_get);
        __INDEX_METHOD("addr", __hiff_value_addr);
        __INDEX_METHOD("hasData", __hiff_value_hasData);
        __INDEX_METHOD("as", __hiff_value_as);
        __INDEX_METHOD("add", __hiff_value_add);
         //make ptr to null. this is used when ptr is released by extra function call(dym_lib).
        __INDEX_METHOD("ptrToNull", __hiff_value_ptr_null);
        //the pointer to current value.
        __INDEX_METHOD("ptrValue", __hiff_value_ptr_value);
        __INDEX_METHOD("setPtr", __hiff_value_setPtr);
    }
    return 0;
}
static int xffi_value_tostring(lua_State *L){
    hffi_value* val = get_ptr_hffi_value(L, 1);
    hstring* hs = hstring_new();
    hffi_value_dump(val, hs);
    lua_pushstring(L, hstring_tostring(hs));
    hstring_delete(hs);
    return 1;
}
static int xffi_value_eq(lua_State *L){
    lua_pushboolean(L, hffi_value_eq(get_ptr_hffi_value(L, 1), get_ptr_hffi_value(L, 2)) == HFFI_TRUE);
    return 1;
}

static const luaL_Reg g_hffi_value_Methods[] = {
    {"__gc", xffi_value_gc},
    {"__index", xffi_value_index},
    {"__eq", xffi_value_eq},
    {"__tostring", xffi_value_tostring},
    {NULL, NULL}
};
//------------------- dym_func ------------
static int xffi_dym_func_gc(lua_State *L){
    dym_func* func = get_ptr_dym_func(L, -1);
    hlua_delete_light_uservalue(L, -1);
    dym_delete_func(func);
    return 0;
}

/** as ffi can't use like this:
1, create a cif to hold some cif info. and latter to call.
*/
static int xffi_dym_func_call(lua_State* L){
    dym_func* func;
    int tab_idx;
    if(lua_type(L, 1) == LUA_TTABLE){
        tab_idx = 1;
        func = get_ptr_dym_func(L, lua_upvalueindex(1));
    }else{
        //1 is dym_func. 2 is table
        func = get_ptr_dym_func(L, 1);
        tab_idx = 2;
    }
    //ret_type, tab_param_types.
    luaL_checktype(L, tab_idx, LUA_TTABLE);
    int len = lua_rawlen(L, tab_idx);
    //abi
    int abi = FFI_DEFAULT_ABI;
    abi = hlua_get_int(L, tab_idx, "abi", abi);
    //var count
    int var_count = 0;
    var_count = hlua_get_int(L, tab_idx, "var_count", var_count);
    if(var_count > len){
        return luaL_error(L, "var count can't bigger than total parameter count.");
    }
    //ret
    hffi_value* ret_val;
    if(lua_getfield(L, tab_idx, "ret") != LUA_TNIL){
        ret_val = __get_value(L, -1);
        if(ret_val == NULL){
             return luaL_error(L, "ret: unsupport cif type. type must be (sint8,hffi_struct,hffi_value)");
        }
        lua_pushnil(L);
        lua_setfield(L, tab_idx, "ret");
    }else{
        ret_val = hffi_new_value_raw_type(HFFI_TYPE_VOID);
    }
    lua_pop(L, 1);//pop ret.
    //params
    hffi_value* val;
    array_list* params = array_list_new_max(len > 2 ? len : 2);
    for(int i = 0 ;i < len ; i ++){
        lua_rawgeti(L, tab_idx, i+1);
        //type: int, struct, value?
        val = __get_value(L, -1);
        if(val != NULL){
            array_list_add(params, val);
        }else{
            array_list_delete2(params, list_travel_value_delete);
            hffi_delete_value(ret_val);
            return luaL_error(L, "params(index = %d): unsupport type. type must be (sint8,hffi_struct,hffi_value)", i);
        }
        lua_pop(L, 1);
    }
    //prepare cif and call
    char _m[128];
    char* msg[1];
    msg[0] = _m;
    if(hffi_call_from_list(abi, func->func_ptr, params, var_count, ret_val, msg) == HFFI_STATE_FAILED){
        array_list_delete2(params, list_travel_value_delete);
        hffi_delete_value(ret_val);
        return luaL_error(L, "%s", msg[0]);
    }
    array_list_delete2(params, list_travel_value_delete);
   // hffi_value_ref(ret_val, 1); //already add ref.
    push_ptr_hffi_value(L, ret_val);
    return 1;
}
int xffi_dym_func_index(lua_State* L){
    if(lua_type(L, 2) == LUA_TSTRING){
        const char* fun_name = lua_tostring(L, 2);
        __INDEX_METHOD("call", xffi_dym_func_call)
        return luaL_error(L, "unsupport method('%s') for smtype.", fun_name);
    }
    return 0;
}
static const luaL_Reg g_dym_func_Methods[] = {
    {"__gc", xffi_dym_func_gc},
    {"__index", xffi_dym_func_index},
    {NULL, NULL}
};
//------------------ dym_lib ----------------------------

static int __dym_lib_func_call(lua_State *L){
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_insert(L, 1);
    return xffi_dym_func_call(L);
}
static int xffi_dym_lib_gc(lua_State *L){
    dym_lib* lib = get_ptr_dym_lib(L, -1);
    dym_delete_lib(lib);
    return 0;
}
static int xffi_dym_lib_index(lua_State *L){
    // lib , name
    dym_lib* lib = get_ptr_dym_lib(L, 1);
    dym_func* func = dym_lib_get_function(lib, luaL_checkstring(L, 2), 1);
    if(func == NULL){
        lua_getuservalue(L, 1);
        return luaL_error(L, "can't find function(%s) for lib(%s)", lua_tostring(L, 2), lua_tostring(L, -1));
    }
    push_ptr_dym_func(L, func);
    lua_pushcclosure(L, __dym_lib_func_call, 1);
    return 1;
}
static int xffi_dym_lib_toString(lua_State *L){
     //dym_lib* lib = get_ptr_dym_lib(L, 1);
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
    dym_lib* lib;
    if(lua_gettop(L) == 2){
        lib = dym_new_lib2(luaL_checkstring(L, 1), luaL_checkstring(L, 2));
        if(lib == NULL){
            return luaL_error(L, "load lib(%s, %s) failed.", lua_tostring(L, 1), lua_tostring(L, 2));
        }
    }else{
        lib = dym_new_lib(luaL_checkstring(L, 1));
        if(lib == NULL){
            return luaL_error(L, "load lib(%s) failed.", luaL_checkstring(L, -1));
        }
    }
    push_ptr_dym_lib(L, lib);
    lua_pushvalue(L, -2);      // str, lib, str
    lua_setuservalue(L, -2);   // str, lib
    return 1;
}
//---------------------  -------------------
static inline hffi_value* __get_value(lua_State *L, int idx){
    if(lua_type(L, idx) == LUA_TNUMBER){
        return hffi_new_value_raw_type((sint8)luaL_checkinteger(L, idx));
    }else if(luaL_testudata(L, idx, __STR(hffi_struct)) != 0){
        hffi_struct* _struct = get_ptr_hffi_struct(L, idx);
        return hffi_new_value_struct(_struct);
        //how to support struct pointer? recommend use value.
    }else if(luaL_testudata(L, idx, __STR(hffi_value)) != 0){
        hffi_value* val = get_ptr_hffi_value(L, idx);
        hffi_value_ref(val, 1);
        return val;
    }else if(luaL_testudata(L, idx, __STR(harray)) != 0){
        harray* val = get_ptr_harray(L, idx);
        //for support harray ptr. must create hffi_value
        return hffi_new_value_harray(val);
    }
    else{
        return NULL;
    }
}
//------------------- closure ----------------------
#define HLUA_EXT_LEN 4
typedef struct FuncContext{
    lua_State *L;
    hffi_closure* closure;
    int ref_ctx;
    int ref_func;
}FuncContext;

static void __delete_FuncContext(FuncContext* fc){
    if(fc->closure){
        hffi_delete_closure(fc->closure);
        fc->closure = NULL;
    }
    if(fc->ref_func != LUA_NOREF){
        luaL_unref(fc->L, LUA_REGISTRYINDEX, fc->ref_func);
    }
    if(fc->ref_ctx != LUA_NOREF){
        luaL_unref(fc->L, LUA_REGISTRYINDEX, fc->ref_ctx);
    }
    FREE(fc);
}

static void Hffi_lua_func(ffi_cif* cif,void* ret,void** args,void* ud){
    H_UNSED(cif);
    H_UNSED(ret);
    H_UNSED(args);
    //ret can be simple int/float. array. struct?
    FuncContext* fc = ud;
    lua_State *L = fc->L;
    lua_rawgeti(L, LUA_REGISTRYINDEX, fc->ref_func);
    lua_rawgeti(L, LUA_REGISTRYINDEX, fc->ref_ctx);
    if(0){
        printf("Hffi_lua_func dump1: >>> \n");
        luaB_dumpStack(L);
    }
    //tab as args
    lua_newtable(L);
    int c = array_list_size(fc->closure->in_vals);
    hffi_value* tmp_val;
    for(int i = 0 ; i < c ; i ++){
        tmp_val = (hffi_value*)array_list_get(fc->closure->in_vals, i);
        hffi_value_set_any(tmp_val, args[i]);
        push_ptr_hffi_value(L, tmp_val);
        lua_rawseti(L, -2, i + 1);
    }
    //-----------------------
#define __Hffi_lua_func_ret(hffi_t, type)\
case hffi_t:{\
    *(type*)ret = (type)lua_tointeger(L, -1);\
}break;
    //ctx, tab(in_vals).
    int ret_call;
    if((ret_call = lua_pcall(L, 2, 1, 0)) == LUA_OK){
        switch (lua_type(L, -1)) {
        case LUA_TNUMBER:{
            DEF_HFFI_BASE_SWITCH(__Hffi_lua_func_ret, fc->closure->ret_val->base_ffi_type)
            if(fc->closure->ret_val->base_ffi_type == HFFI_TYPE_FLOAT){
                *(float*)ret = (float)lua_tonumber(L, -1);
            }else if(fc->closure->ret_val->base_ffi_type == HFFI_TYPE_DOUBLE){
                *(double*)ret = (double)lua_tonumber(L, -1);
            }else{
                //*(lua_Integer*)ret = lua_tointeger(L, -1);
                luaL_error(L, "support type for closure callback.");
                return;
            }
        }break;

        case LUA_TUSERDATA:{
            if(luaL_testudata(L, -1, __STR(hffi_value))){
                hffi_value* hs = get_ptr_hffi_value(L, -1);
                if(hffi_value_get_base(hs, ret) != HFFI_STATE_OK){
                    hffi_struct* str = hffi_value_get_struct(hs);
                    if(str != NULL){
                        if(hs->base_ffi_type == HFFI_TYPE_POINTER){
                            *(void**)ret = str->data;
                        }else{
                            *(void**)ret = *((void**)str->data);
                        }
                        return;
                    }
                    harray* arr = hffi_value_get_harray(hs);
                    if(arr != NULL){
                        if(hs->base_ffi_type == HFFI_TYPE_POINTER){
                            *(void**)ret = arr->data;
                        }else{
                            *(void**)ret = *((void**)arr->data);
                        }
                        return;
                    }
                    //default as ptr.
                    *(void**)ret = hs->ptr;
                }
            }else if(luaL_testudata(L, -1, __STR(hffi_struct))){
                hffi_struct* str = get_ptr_hffi_struct(L, -1);
                if(fc->closure->ret_val->base_ffi_type == HFFI_TYPE_POINTER){
                    *(void**)ret = str->data;
                }else{
                    *(void**)ret = *((void**)str->data);
                }
                return;
            }else if(luaL_testudata(L, -1, __STR(harray))){
                harray* arr = get_ptr_harray(L, -1);
                if(fc->closure->ret_val->base_ffi_type == HFFI_TYPE_POINTER){
                    *(void**)ret = arr->data;
                }else{
                    *(void**)ret = *((void**)arr->data);
                }
                return;
            }
        }break;

        default:
            luaL_error(L, "wrong return type. only support 'number/value/struct/array'.");
        }
        return;
    }
    switch (ret_call) {
    case LUA_ERRRUN:{luaL_error(L, "LUA_ERRRUN(runtime) for call lua func from closure.");}break;
    case LUA_ERRMEM:{luaL_error(L, "LUA_ERRMEM(memory) for call lua func from closure.");}break;
    case LUA_ERRERR:{luaL_error(L, "LUA_ERRERR for call lua func from closure.");}break;
    case LUA_ERRGCMM:{luaL_error(L, "LUA_ERRGCMM(__gc) for call lua func from closure.");}break;
    case LUA_ERRSYNTAX:{luaL_error(L, "LUA_ERRSYNTAX(system) for call lua func from closure.");}break;
    }
}

static int xffi_new_closure(lua_State *L){
/*
void (*fun_proxy)(ffi_cif*,void* ret,void** args,void* ud),
                               struct array_list* in_vals, hffi_value* return_type, void* ud
*/
    //tab, func.
    //tab: {a,b,c, ret = xx, ctx = xx, abi = xx}
    //func: ret type can be. number/hffi_value/hffi_struct/hffi_harray
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    int type;
    int abi = FFI_DEFAULT_ABI;
    int ref_ctx, ref_func = LUA_NOREF;
    //func
    ref_func = luaL_ref(L, LUA_REGISTRYINDEX); // ref func and pop
    //ctx
    ref_ctx = hlua_get_ref(L, 1, "ctx", LUA_REGISTRYINDEX);
    //abi
    abi = hlua_get_int(L, 1, "abi", abi);
    //ret
    type = lua_getfield(L, 1, "ret");
    hffi_value* val_ret;
    switch (type) {
    case LUA_TUSERDATA:{
        val_ret = get_ptr_hffi_value(L, -1);
        hffi_value_ref(val_ret, 1);
        lua_pushnil(L);
        lua_setfield(L, 1 ,"ret");
    }break;

    case LUA_TNUMBER:{
        val_ret = hffi_new_value_raw_type(luaL_checkinteger(L, -1));
        lua_pushnil(L);
        lua_setfield(L, 1 ,"ret");
    }break;

    default:
    case LUA_TNIL:{
        val_ret = hffi_get_void_value();
    }break;
    }
    lua_pop(L, 1);
    //build a context for hold some values
    FuncContext* fc = MALLOC(sizeof(FuncContext));
    memset(fc, 0, sizeof (FuncContext));
    fc->L = L;
    fc->ref_ctx = ref_ctx;
    fc->ref_func = ref_func;
    //tab
    int c = lua_rawlen(L, 1);
    //
    array_list* in_vals = array_list_new_max(c);
    hffi_value* tmp_val;
    for(int i = 0 ; i < c ; i ++){
        lua_rawgeti(L, 1, i + 1);
        tmp_val = __get_value(L, -1);
        if(tmp_val == NULL){
            goto failed;
        }
        array_list_add(in_vals, tmp_val);
        lua_pop(L, 1);
    }
    //closure
    hffi_closure* clo = hffi_new_closure(abi, Hffi_lua_func, in_vals, val_ret, fc, NULL);
    hffi_closure_ref(clo, 1);
    fc->closure = clo;
    push_ptr_hffi_closure(L, clo);
    lua_pushlightuserdata(L, fc);
    lua_setuservalue(L, -2);
    array_list_delete2(in_vals, list_travel_value_delete);
    hffi_delete_value(val_ret);
    return 1;

    failed:
    array_list_delete2(in_vals, list_travel_value_delete);
    __delete_FuncContext(fc);
    hffi_delete_value(val_ret);
    return 0;
}
static int xffi_closure_gc(lua_State* L){
    hffi_closure* clo = get_ptr_hffi_closure(L, -1);
    if(hffi_delete_closure(clo) == 1){
        //only left the one referenced by FuncContext.
        lua_getuservalue(L, 1);
        __delete_FuncContext((FuncContext*)lua_topointer(L, -1));
    }
    return 0;
}
static int xffi_closure_index(lua_State* L){
    //tab, index
    hffi_closure* clo = get_ptr_hffi_closure(L, 1);
    if(lua_type(L, 2) == LUA_TSTRING){
        const char* fun_name = lua_tostring(L, 2);
        //array, str
        if(strcmp("ret", fun_name) == 0){
            hffi_value_ref(clo->ret_val, 1);
            push_ptr_hffi_value(L, clo->ret_val);
            return 1;
        }
        return luaL_error(L, "unsupport method('%s') for closure.", fun_name);
    }
    int index = luaL_checkinteger(L, 2);
    //clo->in_vals
    int size = array_list_size(clo->in_vals) ;
    if(index < 0){
        index = size + index;
    }
    if(index >= size){
        return luaL_error(L, "index outOfRange('%d') for closure.", index);
    }
    hffi_value* val = array_list_get(clo->in_vals, index);
    hffi_value_ref(val, 1);
    return push_ptr_hffi_value(L, val);
}

static const luaL_Reg g_hffi_closure_Methods[] = {
    {"__gc", xffi_closure_gc},
    {"__index", xffi_closure_index},
    {NULL, NULL}
};
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
    lua_pushinteger(L, __N_VAL);
    lua_setglobal(L, "N");

    lua_pushinteger(L, EAGAIN);
    lua_setglobal(L, "EAGAIN");
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
    lua_pushnil(L);
    lua_setglobal(L, "N");
    lua_pushnil(L);
    lua_setglobal(L, "EAGAIN");
    return 0;
}
static int xffi_typeStr(lua_State *L){
    sint8 type = (sint8)luaL_checkinteger(L, 1);
    const BasePair *lib;
    for (lib = _BASE_PAIRS; lib->name; lib++) {
       if(lib->type == type){
           lua_pushstring(L, lib->name);
           return 1;
       }
    }
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
    REG_CLASS(L, hffi_smtype);
    REG_CLASS(L, harray);
    REG_CLASS(L, hffi_closure);

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "hffi");
    // the ffi table is still on top
    setfield_function(L, "defines", xffi_defines);
    setfield_function(L, "undefines", xffi_undefines);
    setfield_function(L, "typestr", xffi_typeStr);

    setfield_function(L, "loadLib", xffi_dym_lib_new);
    setfield_function(L, "value", xffi_value_new);
    setfield_function(L, "valuePtr", xffi_value_ptr_new);
    setfield_function(L, "array", xffi_harray_new);
    setfield_function(L, "struct", xffi_struct_new);
    setfield_function(L, "smtype", xffi_smtype_new);
    setfield_function(L, "closure", xffi_new_closure);
    lua_pop(L, 1);
}
