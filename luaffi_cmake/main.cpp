#include <iostream>
using namespace std;

extern "C"{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "ffi2.h"

void testCall();
void testCall2();
void testClosure();
void hffi_test1();
void hffi_test2();
void hffi_test_struct();
void hffi_test_struct2();
void hffi_test_struct3();
void test_call_unions();
}

#define CALL_LUA(L, func)\
{int s = func(L);\
if(s){\
    cout << "CALL_LUA >> error: "<< lua_tostring(L, -1) << endl;\
}}

static const luaL_Reg funcs[] = {
{"ffi", luaopen_ffi},
{NULL, NULL},
};

LUALIB_API void luaL_openlibs2(lua_State *L, const luaL_Reg funcs[]) {
  const luaL_Reg *lib;
  /* "require" functions from 'loadedlibs' and set results to global table */
  for (lib = funcs; lib->func; lib++) {
    luaL_requiref(L, lib->name, lib->func, 1);
    lua_pop(L, 1);  /* remove lib */
  }
}

#define LUA_DIR "../luaffi_cmake/lua_script/"

static void test_ffi_all(){
    testCall();
    testCall2();
    testClosure();
    hffi_test1();
    hffi_test2();
    hffi_test_struct();
    hffi_test_struct2();
    hffi_test_struct3();
    test_call_unions();
}

extern "C" int main()
{
    //cout << "Hello World!" << endl;
    test_ffi_all();

    printf("-------------- start lua -------------- \n");
    lua_State * L = luaL_newstate();
    luaL_openlibs(L);
    luaL_openlibs2(L, funcs);

    if(luaL_dostring(L, "package.path=\"" LUA_DIR "/?.lua;"
                     "\"..package.path"
                     ";print('package.path = ', package.path)"
                     ";print('package.cpath = ', package.cpath)")){
        cout << "error: "<< lua_tostring(L, -1) << endl;
    }else{
        cout << "lua do string success." << endl;
    }
    CALL_LUA(L, [](lua_State * L){
        return luaL_dofile(L, LUA_DIR "test_libc.lua");
    });

    lua_close(L);
    return 0;
}
