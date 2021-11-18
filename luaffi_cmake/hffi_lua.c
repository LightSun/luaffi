
#include "hffi_lua.h"
#include "hffi.h"
#include "dym_loader.h"
#include "h_list.h"
#include "h_alloctor.h"
#include "h_array.h"
#include "h_string.h"

#include "lua_utils.h"

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
DEF_HFFI_PUSH_GET(hffi_cif)
DEF_HFFI_PUSH_GET(hffi_smtype)
DEF_HFFI_PUSH_GET(harray)

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
    {"int", HFFI_TYPE_SINT32},
    {"uint", HFFI_TYPE_UINT32},

    {"string", HFFI_TYPE_STRING},
    {NULL, 0},
};

#define __INDEX_METHOD(name, func_lua)\
if(strcmp(fun_name, name) == 0){\
    lua_pushvalue(L, 1);\
    lua_pushcclosure(L, func_lua, 1);\
    return 1;\
}
/**
---------- harray->hffi_smtype->hffi_value-> hffi_struct-> hffi_cif
---------- dym_lib->dym_func -------
  */
//-------------- share funcs --------------
static void string_delete(void* d){
    if(d) FREE(d);
}
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
    harray_delete(arr);
    return 0;
}
static int xffi_harray_newindex(lua_State* L){
    //tab, i, val(single or table)
    harray* arr = get_ptr_harray(L, -3);
    int index = luaL_checkinteger(L, -2);
    if(index < 0) index = arr->ele_count + index;
    if(lua_type(L, -1) == LUA_TTABLE){
        int c = lua_rawlen(L, -1);
        if(c > arr->ele_count - index){
            c = arr->ele_count - index;
        }
        lua_pushvalue(L, -3); // harray
        lua_pushvalue(L, -3); // index
        for(int i = 0 ; i < c ; i ++){
            lua_rawgeti(L, -3, i + 1); //sub element
            lua_pushinteger(L, index + i);
            lua_replace(L, -3);        //replace index and pop
            //array, index, data
            xffi_harray_newindex(L);
            lua_pop(L, 1);
        }
        return 0;
    }else{
        //int ot float
        __harray_set_vals(L, arr, index);
        return 0;
    }
    return luaL_error(L, "wrong element type for __newindex(harray)!");
}
static int __harray_func_set(lua_State* L){
    //index, data
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_insert(L, 1);
    //ud, index, data
    xffi_harray_newindex(L);
    return 0;
}
static int __harray_func_copy(lua_State* L){
    //copy
    harray* arr = get_ptr_harray(L, lua_upvalueindex(1));
    return push_ptr_harray(L, harray_copy(arr));
}
static const luaL_Reg g_harray_str_Methods[] = {
    {"set", __harray_func_set},
    {"copy", __harray_func_copy},
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
            return push_ptr_harray(L, (harray*)ele._extra);
         }
         return 0;
     }break;
     case HFFI_TYPE_STRUCT_PTR:
     case HFFI_TYPE_STRUCT:{
         union harray_ele ele;
         if(harray_geti(arr, index, &ele) == HFFI_STATE_OK){
            return push_ptr_hffi_struct(L, (hffi_struct*)ele._extra);
         }
         return 0;
     }break;

    }
    return luaL_error(L, "unsupport data type for get index data from harray.");
}

static int xffi_harray_len(lua_State* L){
    harray* arr = get_ptr_harray(L, 1);
    lua_pushinteger(L, harray_get_count(arr));
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
static harray* __harray_new_from_table(lua_State* L, int idx);

static int xffi_harray_new(lua_State* L){
    if(lua_gettop(L) < 1){
        return luaL_error(L, "create harray need [type & count]/[harray...]/[struct...].");
    }
    switch (lua_type(L, 1)) {
    case LUA_TNUMBER:{
        if(lua_gettop(L) != 2){
            return luaL_error(L, "create harray need [type & count]/[harray...]/[struct...].");
        }
        harray* arr;
        if(lua_type(L, 2) == LUA_TNUMBER){
            int c = luaL_checkinteger(L, 2);
            arr = harray_new(luaL_checkinteger(L, 1), c);
        }else{
            int c = lua_rawlen(L, 2);
            arr = harray_new(luaL_checkinteger(L, 1), c);
            for(int i = 0 ; i < c; i ++){
                lua_rawgeti(L, 2, i + 1); //type, tab, ele
                __harray_set_vals(L, arr, i);
                lua_pop(L, 1);
            }
        }
        return push_ptr_harray(L, arr);
    }break;
    case LUA_TTABLE:{
        harray* arr = __harray_new_from_table(L, 1);
        if(arr){
            return push_ptr_harray(L, arr);
        }
    }break;
    case LUA_TSTRING:{
        const char* str = lua_tostring(L, 1);
        return push_ptr_harray(L, harray_new_chars(str));
    }break;
    }


    /**
        const char* str = lua_tostring(L, 1);
        harray* arr = harray_new_chars(str);
        */
    return luaL_error(L, "unsupport args for create harray. need [type & count]/[harray...]/[struct...].");
}

static const luaL_Reg g_harray_Methods[] = {
    {"__gc", xffi_harray_gc},
    {"__index", xffi_harray_index},
    {"__newindex", xffi_harray_newindex},
    {"__len", xffi_harray_len},
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
    //harray/hffi_struct. bool(ptr or not)
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
    luaL_checktype(L, -1, LUA_TTABLE);
    if(lua_rawlen(L, -1) == 0) return 0;

    array_list* sm_list = array_list_new_simple();
    array_list* sm_names = array_list_new_simple();

    if(build_smtypes(L, sm_list, sm_names, get_ptr_hffi_struct,
                     get_ptr_harray, get_ptr_hffi_smtype) == HFFI_STATE_FAILED){
        array_list_delete2(sm_list, list_travel_smtype_delete);
        array_list_delete2(sm_names, string_delete);
        return luaL_error(L, "build struct met unsupport data type.");
    }
    //options
    int no_data = 0;
    int abi = FFI_DEFAULT_ABI;
    if(lua_type(L, 2) == LUA_TTABLE){
        no_data = hlua_get_boolean(L, 2, "no_data", no_data);
        abi = hlua_get_int(L, 2, "abi", abi);
    }

    //msg
    char _m[128];
    char* msg[1];
    msg[0] = _m;
    //create struct
    hffi_struct* _struct;
    if(no_data){
        _struct = hffi_new_struct_from_list_no_data(abi, sm_list, msg);
    }else{
        _struct = hffi_new_struct_from_list2(abi, sm_list, msg);
    }
    if(_struct == NULL){
        array_list_delete2(sm_list, list_travel_smtype_delete);
        array_list_delete2(sm_names, string_delete);
        return luaL_error(L, "create struct failed by '%s'", msg[0]);
    }
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
    push_ptr_hffi_struct(L, hffi_struct_copy(hs));
    hlua_share_light_uservalue(L, lua_upvalueindex(1), -1);
    return 1;
}
static int __struct_get(lua_State *L){
    hffi_struct* hs = get_ptr_hffi_struct(L, lua_upvalueindex(1));
    if(hs->data == NULL){
        return 0;
    }
    //index, target_ffi, ptr
    //multi level ptr
    if(hs->parent_pos == HFFI_STRUCT_NO_DATA){
        //int rows, int cols, int continue_mem, int share_mem
        luaL_checktype(L, 1, LUA_TTABLE);
        int continue_mem = 1;
        int share_mem = 1;
        int index = 0;
        sint8 hffi_t = HFFI_TYPE_SINT8;
        continue_mem = hlua_get_boolean(L, 1, "continue_mem", continue_mem);
        share_mem = hlua_get_boolean(L, 1, "share_mem", share_mem);
        index = hlua_get_int(L, 1, "index", index);
        hffi_t = (sint8)hlua_get_int(L, 1, "type", hffi_t);

        if(lua_rawlen(L, 1) == 0) return 0;
        int rows = hlua_rawgeti_int(L, 1, 1);
        int cols = 0;
        if(lua_rawlen(L, 1) >= 2){
            cols = hlua_rawgeti_int(L, 1, 2);
        }
        harray* arr = hffi_struct_get_as_array(hs, index, hffi_t, rows, cols, continue_mem, share_mem);
        return push_ptr_harray(L, arr);
    }
    //index
    int index = luaL_checkinteger(L, 1);
    hffi_struct* hstr = hffi_struct_get_struct(hs, index);
    if(hstr != NULL){
         hffi_struct_ref(hstr, 1);
         return push_ptr_hffi_struct(L, hstr);
    }
    harray* arr = hffi_struct_get_harray(hs, index);
    if(hstr != NULL){
         harray_ref(arr, 1);
         return push_ptr_harray(L, arr);
    }
    //target element type.
    int ffi_t = HFFI_TYPE_SINT8;
    if(hffi_struct_is_pointer(hs, index)){
        ffi_t = luaL_checkinteger(L, 2);
    }
    if(ffi_t == HFFI_TYPE_FLOAT || ffi_t == HFFI_TYPE_DOUBLE){
        lua_Number num;
        hffi_struct_get_base(hs, index, ffi_t, &num);
        lua_pushnumber(L, num);
        return 1;
    }else{
        lua_Integer num;
        hffi_struct_get_base(hs, index, ffi_t, &num);
        lua_pushinteger(L, num);
        return 1;
    }
    return 0;
}
static int xffi_struct_index(lua_State *L){
    if(lua_type(L, 2) == LUA_TSTRING){
        const char* fun_name = lua_tostring(L, 2);
        __INDEX_METHOD("copy", __struct_copy)
        __INDEX_METHOD("get", __struct_get)
        return luaL_error(L, "unsupport method('%s') for smtype.", fun_name);
    }
    return 0;
}

static const luaL_Reg g_hffi_struct_Methods[] = {
    {"__gc", xffi_struct_gc},
    {"__index", xffi_struct_index},
    {NULL, NULL}
};
//------------------ hffi_value ------------
static int xffi_value_new(lua_State *L){
    // base_type[, val]
    // pointer, ptr_base_type, [val]
    // struct/harray [, is_pointer]
    // string[, is_pointer]
    hffi_value* val = NULL;
    switch (lua_type(L, 1)) {
    case LUA_TNUMBER:{
        sint8 type = (sint8)luaL_checkinteger(L, 1);       
        //ptr
        if(type == HFFI_TYPE_POINTER){
            type = (sint8)luaL_checkinteger(L, 2);
            if(lua_gettop(L) >= 3){
                if(lua_type(L, 3) == LUA_TBOOLEAN){
                    //bool: indicate alloc val.ptr or not
                    if(lua_toboolean(L, 3)){
                        val = hffi_new_value_ptr_no_data(type);
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
                    lua_Number num = lua_tonumber(L, 3);
                    val = hffi_new_value_raw_type2(type, &num);
                }else{
                    lua_Integer num = lua_tointeger(L, 3);
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
        }
    }break;
    case LUA_TSTRING:{
        const char* str = lua_tostring(L, 1);
        harray* arr = harray_new_chars(str);
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
    //multi level ptr
    if(val->multi_level_ptr){
        //int rows, int cols, int continue_mem, int share_mem
        luaL_checktype(L, 1, LUA_TTABLE);
        int continue_mem = 1;
        int share_mem = 1;
        continue_mem = hlua_get_boolean(L, 1, "continue_mem", continue_mem);
        share_mem = hlua_get_boolean(L, 1, "share_mem", share_mem);

        if(lua_rawlen(L, 1) == 0) return 0;
        int rows = hlua_rawgeti_int(L, 1, 1);
        int cols = 0;
        if(lua_rawlen(L, 1) >= 2){
            cols = hlua_rawgeti_int(L, 1, 2);
        }
        harray* arr = hffi_value_get_pointer_as_array(val, rows, cols, continue_mem, share_mem);
        return push_ptr_harray(L, arr);
    }
    //int rows, int cols, int continue_mem, int share_mem
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
    }
    if(ffi_t == HFFI_TYPE_FLOAT || ffi_t == HFFI_TYPE_DOUBLE){
        lua_Number num;
        hffi_value_get_base(val, &num);
        lua_pushnumber(L, num);
        return 1;
    }else{
        lua_Integer num;
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
static int xffi_value_index(lua_State *L){
    if(lua_type(L, 2) == LUA_TSTRING){
        const char* fun_name = lua_tostring(L, 2);
        if(strcmp(fun_name, "copy") == 0){
            lua_pushvalue(L, 1);
            lua_pushcclosure(L, __hiff_value_copy, 1);
            return 1;
        }
        if(strcmp(fun_name, "get") == 0){
            lua_pushvalue(L, 1);
            lua_pushcclosure(L, __hiff_value_get, 1);
            return 1;
        }
    }
    return 0;
}

static const luaL_Reg g_hffi_value_Methods[] = {
    {"__gc", xffi_value_gc},
    {"__index", xffi_value_index},
    {NULL, NULL}
};
//------------------- dym_func ------------
static int xffi_dym_func_gc(lua_State *L){
    dym_func* func = get_ptr_dym_func(L, -1);
    hlua_delete_light_uservalue(L, -1);
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

static int __dym_func_bind(lua_State *L){
    int func_index = lua_upvalueindex(1);
    hffi_cif* cif = get_ptr_hffi_cif(L, -1);
    hffi_cif_ref(cif, 1);
    hlua_push_light_uservalue(L, func_index, cif, list_travel_hcif_delete);
    lua_pushvalue(L, func_index);
    return 1;
}

static int __dym_func_call(lua_State *L){
    //func, cif
    int func_index = lua_upvalueindex(1);
    dym_func* func = get_ptr_dym_func(L, func_index);
    hffi_cif* cif = (hffi_cif*)hlua_get_light_uservalue(L, func_index);
    if(cif == NULL){
        cif = get_ptr_hffi_cif(L, -1);
    }
    hffi_cif_call(cif, func->func_ptr);

    hffi_value_ref(cif->out, 1);
    push_ptr_hffi_value(L, cif->out);
    return 1;
}
int xffi_dym_func_index(lua_State* L){
    if(lua_type(L, 2) == LUA_TSTRING){
        const char* fun_name = lua_tostring(L, 2);
        __INDEX_METHOD("call", __dym_func_call)
        __INDEX_METHOD("bind", __dym_func_bind)
        return luaL_error(L, "unsupport method('%s') for smtype.", fun_name);
    }
}
static const luaL_Reg g_dym_func_Methods[] = {
    {"__gc", xffi_dym_func_gc},
    {"__index", xffi_dym_func_index},
    {NULL, NULL}
};
//------------------ dym_lib ----------------------------

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
    luaL_checktype(L, -1, LUA_TSTRING);
    dym_lib* lib = dym_new_lib(luaL_checkstring(L, -1));
    push_ptr_dym_lib(L, lib);
    lua_pushvalue(L, -2);      // str, lib, str
    lua_setuservalue(L, -2);   // str, lib
    return 1;
}
//--------------------- cif -------------------

static int xffi_cif_gc(lua_State *L){
    hffi_cif* cif = get_ptr_hffi_cif(L, 1);
    hffi_delete_cif(cif);
    return 0;
}

static const luaL_Reg g_hffi_cif_Methods[] = {
    {"__gc", xffi_cif_gc},
    {NULL, NULL}
};

static inline hffi_value* __get_value(lua_State *L, int idx){
    if(lua_type(L, idx) == LUA_TNUMBER){
        return hffi_new_value_raw_type(luaL_checkinteger(L, idx));
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
//cif { ret = pointer; pointer, sint, uint32 }
static int xffi_cif(lua_State *L){
    //ret_type, tab_param_types.
    luaL_checktype(L, 1, LUA_TTABLE);
    int len = lua_rawlen(L, 1);
    //abi
    int abi = FFI_DEFAULT_ABI;
    abi = hlua_get_int(L, 1, "abi", abi);
    //ret
    hffi_value* ret_val;
    if(lua_getfield(L, 1, "ret") != LUA_TNIL){
        ret_val = __get_value(L, -1);
        if(ret_val == NULL){
             return luaL_error(L, "unsupport cif type. type must be (sint8,hffi_struct,hffi_value)");
        }
        lua_pushnil(L);
        lua_setfield(L, 1, "ret");
    }else{
        ret_val = hffi_new_value_raw_type(HFFI_TYPE_VOID);
    }
    lua_pop(L, 1);//pop ret.
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
            array_list_delete2(params, list_travel_value_delete);
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
        array_list_delete2(params, list_travel_value_delete);
        hffi_delete_value(ret_val);
        return luaL_error(L, "%s", msg);
    }
    //de-ref
    hffi_delete_value(ret_val);
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
    REG_CLASS(L, hffi_smtype);
    REG_CLASS(L, harray);

    lua_newtable(L);
    lua_pushvalue(L, -1);
    lua_setglobal(L, "ffi");
    // the ffi table is still on top
    setfield_function(L, "defines", xffi_defines);
    setfield_function(L, "undefines", xffi_undefines);

    setfield_function(L, "loadLib", xffi_dym_lib_new);
    setfield_function(L, "struct", xffi_struct_new);
    setfield_function(L, "cif", xffi_cif);
    setfield_function(L, "value", xffi_value_new);
    setfield_function(L, "array", xffi_harray_new);
    setfield_function(L, "smtype", xffi_smtype_new);
    lua_pop(L, 1);
}

//------------------------------------------------------------------------
static harray* __harray_new_from_table(lua_State* L, int idx){
    //table(harray/struct/string/base/...)
    //len: the char array size, which used to create string
    //base_type: base ffi type, which indicate the memory size
    //asPtr: element as ptr or not. for harrays and structs. need this
    int len = 0;
    int asPtr = 0;
    sint8 base_type = HFFI_TYPE_INT;
    if(lua_type(L, idx + 1) == LUA_TTABLE){
        len = hlua_get_int(L, idx + 1, "len", len);
        base_type = (sint8)hlua_get_int(L, idx + 1, "type", base_type);
        asPtr = hlua_get_boolean(L, idx + 1, "asPtr", asPtr);
    }

    int count = lua_rawlen(L, idx);
    if(count == 0) return NULL;

    //do build harray
    array_list* list = array_list_new2(count * 4 / 3 + 1);
    switch (lua_rawgeti(L, idx, 1)) {
    case LUA_TNUMBER: {
        lua_pop(L, 1);
        harray* p_arr = harray_new(base_type, count);
        for(int i = 0 ; i < count ; i ++){
            lua_rawgeti(L, idx, i + 1);
            if(base_type == HFFI_TYPE_FLOAT || base_type == HFFI_TYPE_DOUBLE){
                lua_Number num = lua_tonumber(L, 3);
                harray_seti2(p_arr, i, &num);
            }else{
                lua_Integer num = lua_tointeger(L, 3);
                harray_seti2(p_arr, i, &num);
            }
            lua_pop(L, 1);
        }
        return p_arr;
    } break;

    case LUA_TSTRING: {
        if(len <= 0){
            //find max len
            const char* chs = lua_tostring(L, -1);
            len = strlen(chs) +1;
            lua_pop(L, 1);
            for(int i = 1 ; i < count ; i ++){
                lua_rawgeti(L, idx, i + 1);
                const char* chs1 = lua_tostring(L, -1);
                if((int)strlen(chs1) + 1 > len){
                    len = strlen(chs1) + 1;
                }
                lua_pop(L, 1);
            }
        }else{
            lua_pop(L, 1);
        }
        harray* p_arr = harray_new(asPtr ? HFFI_TYPE_HARRAY_PTR: HFFI_TYPE_HARRAY, count);
        harray* tmp_arr;
        union harray_ele ele;
        for(int i = 0 ; i < count ; i ++){
            lua_rawgeti(L, idx, i + 1);
            ele._extra = tmp_arr = harray_new_chars2(lua_tostring(L, -1), len);
            //harray_seti will ref + 1, so we need - 1.
            harray_ref(tmp_arr, -1);
            harray_seti(p_arr, i, &ele);
            lua_pop(L, 1);
        }
        return p_arr;
    } break;

    case LUA_TUSERDATA: {
        if(luaL_testudata(L, -1, __STR(harray))){
            array_list_add(list, get_ptr_harray(L, -1));
            lua_pop(L, 1);
            for(int i = 1 ; i < count ; i ++){
                lua_rawgeti(L, idx, i + 1);
                if(luaL_testudata(L, -1, __STR(harray))){
                    array_list_add(list, get_ptr_harray(L, -1));
                }else{
                    array_list_delete2(list, NULL);
                    luaL_error(L, "create harray for arrays. we need only harray.");
                    return NULL;
                }
                lua_pop(L, 1);
            }
            //prepare out harray
            harray* arr;
            if(asPtr){
                 arr = harray_new_array_ptr(count);
                 //set data
                 union harray_ele ele;
                 for(int i = 0 ; i < count ; i ++){
                     ele._extra = array_list_get(list, i);
                     harray_seti(arr, i, &ele);
                 }
            }else{
                arr = harray_new_arrays(list);
            }
            array_list_delete2(list, NULL);
            return arr;

        }else if(luaL_testudata(L, -1, __STR(hffi_struct))){
            array_list_add(list, get_ptr_hffi_struct(L, -1));
            lua_pop(L, 1);
            for(int i = 1 ; i < count ; i ++){
                lua_rawgeti(L, idx, i + 1);
                if(luaL_testudata(L, -1, __STR(hffi_struct))){
                    array_list_add(list, get_ptr_hffi_struct(L, -1));
                }else{
                    array_list_delete2(list, NULL);
                    luaL_error(L, "create harray for structs. we need only 'hffi_struct'.");
                    return NULL;
                }
                lua_pop(L, 1);
            }
            harray* arr;
            if(asPtr){
                arr = harray_new_struct_ptr(count);
                //set data
                union harray_ele ele;
                for(int i = 0 ; i < count ; i ++){
                    ele._extra = array_list_get(list, i);
                    harray_seti(arr, i, &ele);
                }
            }else{
                arr = harray_new_structs(list);
            }
            array_list_delete2(list, NULL);
            return arr;
        }
    }break;
    }

    return NULL;
}
