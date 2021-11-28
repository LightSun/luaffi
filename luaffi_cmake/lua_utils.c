
#include "lua_utils.h"
#include "h_alloctor.h"
#include "atomic.h"

struct HDataWrapper{
    void* data;
    Fun_delete func;
    volatile int ref;
};

#define __CONCAT_PRINTF(f, expre)\
if(first){\
    first = 0;\
    printf(f, expre);\
}else{\
    char _buf[20];\
    snprintf(_buf, 20, f, expre);\
    printf(", %s", _buf);\
}

int hlua_get_ref(lua_State* L, int tab_id, const char* key, int t){
    tab_id = HLUA_ADJUST_ID(tab_id);
    int ref_ctx = LUA_NOREF;
    if(lua_getfield(L, tab_id, key) != LUA_TNIL){
        ref_ctx = luaL_ref(L, t);
        lua_pushnil(L);
        lua_setfield(L, tab_id, key);
    }
    lua_pop(L, 1);
    return ref_ctx;
}

int hlua_get_int(lua_State* L, int tab_id, const char* key, int def_val){
    tab_id = HLUA_ADJUST_ID(tab_id);
    int t = lua_getfield(L, tab_id, key);
    if(t == LUA_TNUMBER){
        def_val = luaL_checkinteger(L, -1);
        lua_pushnil(L);
        lua_setfield(L, tab_id, key);
    }
    lua_pop(L, 1);
    return def_val;
}

int hlua_get_number(lua_State* L, int tab_id, const char* key, lua_Number def_val){
    tab_id = HLUA_ADJUST_ID(tab_id);
    int t = lua_getfield(L, tab_id, key);
    if(t == LUA_TNUMBER){
        def_val = lua_tonumber(L, -1);
        lua_pushnil(L);
        lua_setfield(L, tab_id, key);
    }
    lua_pop(L, 1);
    return def_val;
}

int hlua_get_boolean(lua_State* L, int tab_id, const char* key, int def_val){
    tab_id = HLUA_ADJUST_ID(tab_id);
    int t = lua_getfield(L, tab_id, key);
    if(t == LUA_TNUMBER){
        def_val = lua_toboolean(L, -1);
        lua_pushnil(L);
        lua_setfield(L, tab_id, key);
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
    tab_idx = HLUA_ADJUST_ID(tab_idx);
    struct HDataWrapper* w = MALLOC(sizeof (struct HDataWrapper));
    w->data = ud;
    w->func = delete;
    w->ref = 1;
    lua_pushlightuserdata(L, w);
    lua_setuservalue(L, tab_idx);
}
void* hlua_get_light_uservalue(lua_State* L, int tab_index){
    if(lua_getuservalue(L, tab_index) == LUA_TNIL){
        lua_pop(L, 1);
        return NULL;
    }
    struct HDataWrapper* w = (struct HDataWrapper*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    return w->data;
}
void hlua_delete_light_uservalue(lua_State* L, int tab_index){
    if(lua_getuservalue(L, tab_index) == LUA_TNIL){
        lua_pop(L, 1);
        return;
    }
    struct HDataWrapper* w = (struct HDataWrapper*)lua_touserdata(L, -1);
    lua_pop(L, 1);
    if(atomic_add(&w->ref, -1) == 1){
        if(w->data) w->func(w->data);
        FREE(w);
    }
}

void hlua_share_light_uservalue(lua_State* L, int tab_old, int tab_new){
    if(lua_getuservalue(L, tab_old) == LUA_TNIL){
        lua_pop(L, 1);
        return;
    }
    struct HDataWrapper* w = (struct HDataWrapper*)lua_touserdata(L, -1);
    atomic_add(&w->ref, 1);
    lua_setuservalue(L, tab_new);
}

//--------------------------------------------------
int hlua_get_struct_member_index(lua_State* L, int tab_index, int count, const char* in_name){
    array_list* sm_names = (array_list*)hlua_get_light_uservalue(L, tab_index);
    int index = -1;
    char* tmp_name;
//    if(strcmp("arr", in_name) == 0){
//        tmp_name = NULL;
//    }
    for(int i = count - 1 ; i >= 0 ; i --){
        tmp_name = array_list_get(sm_names, i);
        if(tmp_name != NULL && strcmp((char*)tmp_name, in_name) == 0){
            index = i;
            break;
        }
    }
    return index;
}
LUALIB_API int luaB_dumpStack(lua_State* L){
    printf("\nbegin dump lua stack: \n");
    int i = 0;
    int top = lua_gettop(L);

    printf("{ ");
    char buf[20]; //if string is to long. may cause stack exception.
    int first = 1;
    for (i = 1; i <= top; ++i) {
        int t = lua_type(L, i);
        switch (t) {
            case LUA_TSTRING:
            {
                __CONCAT_PRINTF("%s", lua_tostring(L, i));
            }
                break;
            case LUA_TBOOLEAN:
            {
                __CONCAT_PRINTF("%s", lua_toboolean(L, i) ? "true " : "false ");
            }
                break;

            case LUA_TNUMBER:
            {
                sprintf(buf, "%g ", lua_tonumber(L, i));
                __CONCAT_PRINTF("%s", buf);
            }
                break;
            default:
            {
                sprintf(buf, "%s ", lua_typename(L, t));
                __CONCAT_PRINTF("%s", buf);
            }
                break;
        }
    }
    printf(" }\n");
    return 0;
}
//---------------------------------------------

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

harray* hlua_new_harray_from_table(lua_State* L, int idx, Func_get_ptr_struct func_struct,
                                   Func_get_ptr_harray func_harray){
    //table(harray/struct/string/base/...)
    //len: the char array size, which used to create string
    //base_type: base ffi type, which indicate the memory size
    //asPtr: element as ptr or not. for harrays and structs. need this
    int len = 0;
    int asPtr = 0;
    sint8 base_type = HFFI_TYPE_INT;
    int has_data = 1;
    if(lua_type(L, idx + 1) == LUA_TTABLE){
        len = hlua_get_int(L, idx + 1, "len", len);
        base_type = (sint8)hlua_get_int(L, idx + 1, "type", base_type);
        asPtr = hlua_get_boolean(L, idx + 1, "asptr", asPtr);
        has_data = !hlua_get_boolean(L, idx + 1, "nodata", 0);
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
            array_list_add(list, func_harray(L, -1));
            lua_pop(L, 1);
            for(int i = 1 ; i < count ; i ++){
                lua_rawgeti(L, idx, i + 1);
                if(luaL_testudata(L, -1, __STR(harray))){
                    array_list_add(list, func_harray(L, -1));
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
                if(has_data){
                    arr = harray_new_array_ptr(count);
                    //set data
                    union harray_ele ele;
                    for(int i = 0 ; i < count ; i ++){
                        ele._extra = array_list_get(list, i);
                        harray_seti(arr, i, &ele);
                    }
                 }else{
                     arr = harray_new_array_ptr_nodata(count);
                     for(int i = 0 ; i < count ; i ++){
                         harray_set_harray_ptr(arr, i, (harray*)array_list_get(list, i));
                     }
                }
            }else{
                arr = has_data ? harray_new_arrays(list) : harray_new_arrays_nodata(list);
            }
            array_list_delete2(list, NULL);
            return arr;

        }else if(luaL_testudata(L, -1, __STR(hffi_struct))){
            array_list_add(list, func_struct(L, -1));
            lua_pop(L, 1);
            for(int i = 1 ; i < count ; i ++){
                lua_rawgeti(L, idx, i + 1);
                if(luaL_testudata(L, -1, __STR(hffi_struct))){
                    array_list_add(list, func_struct(L, -1));
                }else{
                    array_list_delete2(list, NULL);
                    luaL_error(L, "create harray for structs. we need only 'hffi_struct'.");
                    return NULL;
                }
                lua_pop(L, 1);
            }
            harray* arr;
            if(asPtr){
                if(has_data){
                    arr = harray_new_struct_ptr(count);
                    //set data
                    union harray_ele ele;
                    for(int i = 0 ; i < count ; i ++){
                        ele._extra = array_list_get(list, i);
                        harray_seti(arr, i, &ele);
                    }
                }else{
                    arr = harray_new_struct_ptr_nodata(count);
                    //set data
                    for(int i = 0 ; i < count ; i ++){
                        harray_set_struct_ptr(arr, i, (hffi_struct*)array_list_get(list, i));
                    }
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


