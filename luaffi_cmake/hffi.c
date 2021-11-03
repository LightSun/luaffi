
#include <stdio.h>
#include "hffi.h"
#include "atomic.h"

#define hffi_new_value_auto_x(type) \
hffi_value* hffi_new_value_##type(hffi_type* t, type val){\
    hffi_value* v = hffi_new_value_auto(t, sizeof (type));\
    type* p = (type*)v->ptr;\
    *p = val;\
    return v;\
}

#define hffi_get_value_auto_x(t)\
int hffi_get_value_##t(hffi_value* val, t* out_ptr){\
    char _m[128];\
    char* msg[1];\
    msg[0] = _m;\
    ffi_type* ft = to_ffi_type(val->type->base_ffi_type, msg);\
    if(ft != NULL && ft->size >= sizeof(t)){\
       if(ft->size >= sizeof(t)){\
          *out_ptr = *((t*)val->ptr);\
          return 0;\
       }else{\
          strcpy(*msg, "wrong value type.");\
       }\
    }\
    return 1;\
}

inline ffi_type* to_ffi_type(int8_t v, char** msg){
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
//#undef hffi_TO_TYPE
    if(msg){
        char str[32];
        snprintf(str, 32, "unknwn ffi_type = %d", v);
        strcpy(*msg, str);
    }
    return NULL;
}
//---------------------------------------------------------------
hffi_type* hffi_new_type_base(int8_t ffi_type){
    hffi_type* ptr = malloc(sizeof (hffi_type));
    ptr->base_ffi_type = ffi_type;
    ptr->pointer_level = 0;
    ptr->pointer_base_type = FFI_TYPE_VOID;
    ptr->ref = 1;
    return ptr;
}
hffi_type* hffi_new_type(int8_t ffi_type, int8_t pointer_type,uint8_t pointer_level){
    hffi_type* ptr = malloc(sizeof (hffi_type));
    ptr->base_ffi_type = ffi_type;
    ptr->pointer_level = pointer_level;
    ptr->pointer_base_type = pointer_type;
    ptr->ref = 1;
    return ptr;
}
int hffi_type_size(hffi_type* t){
    ffi_type* ft = to_ffi_type(t->base_ffi_type, NULL);
    return ft ? ft->size : 0;
}
void hffi_free_type(hffi_type* t){
    int old = atomic_add(&t->ref, -1);
    if(old == 1){
        free(t);
    }
}

hffi_value* hffi_new_value_ptr(hffi_type* type){
    if(type->base_ffi_type != FFI_TYPE_POINTER) return NULL;
#define INIT_MEM(t, s)\
case t:{\
    val_ptr->ptr = malloc(s);\
}break;

    hffi_value* val_ptr = malloc(sizeof(hffi_value));
    switch (type->pointer_base_type) {
        INIT_MEM(FFI_TYPE_SINT8, sizeof (sint8*))
        INIT_MEM(FFI_TYPE_UINT8, sizeof (uint8*))
        INIT_MEM(FFI_TYPE_SINT16, sizeof (sint16*))
        INIT_MEM(FFI_TYPE_UINT16, sizeof (uint16*))
        INIT_MEM(FFI_TYPE_SINT32, sizeof (sint32*))
        INIT_MEM(FFI_TYPE_UINT32, sizeof (uint32*))
        INIT_MEM(FFI_TYPE_SINT64, sizeof (sint64*))
        INIT_MEM(FFI_TYPE_UINT64, sizeof (uint64*))

        INIT_MEM(FFI_TYPE_INT, sizeof (sint32*))
        INIT_MEM(FFI_TYPE_FLOAT, sizeof (float*))
        INIT_MEM(FFI_TYPE_DOUBLE, sizeof (double*))
        INIT_MEM(FFI_TYPE_VOID, sizeof (void*))
    }
    val_ptr->type = type;
    val_ptr->ref = 1;
    atomic_add(&type->ref, 1);
    return val_ptr;
}
hffi_value* hffi_new_value_auto(hffi_type* type, int size){
    hffi_value* val_ptr = malloc(sizeof(hffi_value));
    val_ptr->ptr = malloc(size);
    val_ptr->type = type;
    val_ptr->ref = 1;
    atomic_add(&type->ref, 1);
    return val_ptr;
}

hffi_new_value_auto_x(sint8)
hffi_new_value_auto_x(sint16)
hffi_new_value_auto_x(sint32)
hffi_new_value_auto_x(sint64)
hffi_new_value_auto_x(uint8)
hffi_new_value_auto_x(uint16)
hffi_new_value_auto_x(uint32)
hffi_new_value_auto_x(uint64)
hffi_new_value_auto_x(int)
hffi_new_value_auto_x(float)
hffi_new_value_auto_x(double)

hffi_get_value_auto_x(sint8)
hffi_get_value_auto_x(sint16)
hffi_get_value_auto_x(sint32)
hffi_get_value_auto_x(sint64)
hffi_get_value_auto_x(uint8)
hffi_get_value_auto_x(uint16)
hffi_get_value_auto_x(uint32)
hffi_get_value_auto_x(uint64)
hffi_get_value_auto_x(int)
hffi_get_value_auto_x(float)
hffi_get_value_auto_x(double)

void hffi_delete_value(hffi_value* val){
    int old = atomic_add(&val->ref, -1);
    if(old == 1){
        if(val->ptr !=NULL){
            free(val->ptr);
        }
        hffi_free_type(val->type);
        free(val);
    }
}
int hffi_call(void (*fn)(void), hffi_value** in, int in_count,hffi_value* out, char** msg){
    //param types with values
    ffi_type ** argTypes = alloca(sizeof(ffi_type *) *in_count);
    void **args = alloca(sizeof(void *) *in_count);
    for(int i = 0 ; i < in_count ; i ++){
        argTypes[i] = to_ffi_type(in[i]->type->base_ffi_type, msg);
        if(argTypes[i] == NULL){
            return 1;
        }
        //cast param value
        args[i] = in[i]->type->base_ffi_type == FFI_TYPE_POINTER ? &in[i]->ptr : in[i]->ptr;
    }
    //prepare call
    ffi_cif cif;
    ffi_type *return_type = to_ffi_type(out->type->base_ffi_type, msg);
    if(return_type == NULL){
        return 1;
    }
    ffi_status s = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, (unsigned int)in_count, return_type, argTypes);
    switch (s) {
    case FFI_OK:{
        ffi_call(&cif, fn, out->ptr, args);
        return 0;
    }break;

    case FFI_BAD_ABI:
        strcpy(*msg, "FFI_BAD_ABI");
        break;
    case FFI_BAD_TYPEDEF:
        strcpy(*msg, "FFI_BAD_TYPEDEF");
        break;
    }
    return 1;
}
