
#include <stdio.h>
#include "ffi_def.h"

static inline ffi_type* to_ffi_type(int8_t v, char** msg){
#define hffi_TO_TYPE(t, v) case t:{ return &v;}
    switch (v) {
    hffi_TO_TYPE(FFI_TYPE_POINTER, ffi_type_pointer)
    hffi_TO_TYPE(FFI_TYPE_SINT8, ffi_type_sint8)
    hffi_TO_TYPE(FFI_TYPE_UINT8, ffi_type_uint8)
    hffi_TO_TYPE(FFI_TYPE_SINT16, ffi_type_sint16)
    hffi_TO_TYPE(FFI_TYPE_UINT16, ffi_type_uint16)
    hffi_TO_TYPE(FFI_TYPE_UINT32, ffi_type_uint32)
    hffi_TO_TYPE(FFI_TYPE_SINT32, ffi_type_sint32)
    hffi_TO_TYPE(FFI_TYPE_INT, ffi_type_sint32)
    hffi_TO_TYPE(FFI_TYPE_UINT64, ffi_type_uint64)
    hffi_TO_TYPE(FFI_TYPE_SINT64, ffi_type_sint64)
    hffi_TO_TYPE(FFI_TYPE_FLOAT, ffi_type_float)
    hffi_TO_TYPE(FFI_TYPE_DOUBLE, ffi_type_double)
    hffi_TO_TYPE(FFI_TYPE_VOID, ffi_type_void)
    }
#undef hffi_TO_TYPE
    char str[32];
    snprintf(str, 32, "unknwn ffi_type = %d", v);
    strcpy(*msg, str);
    return NULL;
}

int hffi_call(void (*fn)(void), hffi_value** in, unsigned int in_count,hffi_value* out, char** msg){
    //param types
    ffi_type ** argTypes = alloca(sizeof(ffi_type *) *in_count);
    for(int i = 0 ; i < in_count ; i ++){
        argTypes[i] = to_ffi_type(in[i]->type.base_ffi_type, msg);
        if(argTypes[i] == NULL){
            return 1;
        }
    }
    //params
    void **args = alloca(sizeof(void *) *in_count);
    for(int i = 0 ; i < in_count ; i ++){
        //cast param value
    }
    ffi_cif cif;
    ffi_type *return_type = to_ffi_type(out->type.base_ffi_type, msg);
    if(return_type == NULL){
        return 1;
    }
    ffi_status s = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, in_count, return_type, argTypes);
    switch (s) {
    case FFI_OK:{
        void *returnPtr = NULL;
        if (return_type->size) {
            returnPtr = alloca(return_type->size);
        }
        ffi_call(&cif, fn, returnPtr, args);
        //TODO cast return value

        return 0;
    }break;
    case FFI_BAD_ABI:
        *msg = "FFI_BAD_ABI";
        break;
    case FFI_BAD_TYPEDEF:
        *msg = "FFI_BAD_TYPEDEF";
        break;
    }
    return 1;
}
