
#include <stdio.h>
#include "hffi_common.h"
#include "h_list.h"
#include "h_string.h"

ffi_type* to_ffi_type(int8_t v, char** msg){
#define hffi_TO_TYPE(t, v) case t:{ return &v;}
    switch (v) {
    hffi_TO_TYPE(HFFI_TYPE_POINTER, ffi_type_pointer)
    hffi_TO_TYPE(HFFI_TYPE_SINT8, ffi_type_sint8)
    hffi_TO_TYPE(HFFI_TYPE_UINT8, ffi_type_uint8)
    hffi_TO_TYPE(HFFI_TYPE_SINT16, ffi_type_sint16)
    hffi_TO_TYPE(HFFI_TYPE_UINT16, ffi_type_uint16)
    hffi_TO_TYPE(HFFI_TYPE_UINT32, ffi_type_uint32)
    hffi_TO_TYPE(HFFI_TYPE_SINT32, ffi_type_sint32)
    hffi_TO_TYPE(HFFI_TYPE_INT, ffi_type_sint32)
    hffi_TO_TYPE(HFFI_TYPE_UINT64, ffi_type_uint64)
    hffi_TO_TYPE(HFFI_TYPE_SINT64, ffi_type_sint64)
    hffi_TO_TYPE(HFFI_TYPE_FLOAT, ffi_type_float)
    hffi_TO_TYPE(HFFI_TYPE_DOUBLE, ffi_type_double)
    hffi_TO_TYPE(HFFI_TYPE_VOID, ffi_type_void)
    }
//#undef hffi_TO_TYPE
    if(msg){
        char str[32];
        snprintf(str, 32, "unknwn ffi_type = %d", v);
        strcpy(msg[0], str);
    }
    return NULL;
}

int hffi_base_type_size(sint8 hffi_t){
    switch (hffi_t) {
    case HFFI_TYPE_SINT8: return ffi_type_sint8.size;
    case HFFI_TYPE_UINT8: return ffi_type_uint8.size;
    case HFFI_TYPE_SINT16: return ffi_type_sint16.size;
    case HFFI_TYPE_UINT16: return ffi_type_uint16.size;
    case HFFI_TYPE_SINT32: return ffi_type_sint32.size;
    case HFFI_TYPE_UINT32: return ffi_type_uint32.size;
    case HFFI_TYPE_SINT64: return ffi_type_sint64.size;
    case HFFI_TYPE_UINT64: return ffi_type_uint64.size;
    case HFFI_TYPE_FLOAT: return ffi_type_float.size;
    case HFFI_TYPE_DOUBLE: return ffi_type_double.size;
    case HFFI_TYPE_INT: return ffi_type_sint32.size;
   // case HFFI_TYPE_POINTER:
    case HFFI_TYPE_HARRAY_PTR:
    case HFFI_TYPE_STRUCT_PTR:
        return sizeof (void*);
    }
    return 0;
}

void array_list_dump(struct array_list* list, struct hstring* hs, void(*func)(void* ele, struct hstring* hs)){
    int c = array_list_size(list);
    for(int i = 0 ; i < c ; i ++){
        func(array_list_get(list, i), hs);
        if(i != c -1){
            hstring_append(hs, ", ");
        }
    }
}
void array_list_dump_values(struct array_list* list, struct hstring* hs){
    array_list_dump(list, hs, list_travel_value_dump);
}
