
#include <string.h>
#include "ffi2.h"

#define __CONCAT_PRINTF(f, expre)\
if(first){\
    first = 0;\
    printf(f, expre);\
}else{\
    char _buf[20];\
    snprintf(_buf, 20, f, expre);\
    printf(", %s", _buf);\
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
