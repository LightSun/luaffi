
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
    //struct size not known here.
    if(t->base_ffi_type == FFI_TYPE_STRUCT){
        return 0;
    }
    ffi_type* ft = to_ffi_type(t->base_ffi_type, NULL);
    return ft ? ft->size : 0;
}
void hffi_delete_type(hffi_type* t){
    int old = atomic_add(&t->ref, -1);
    if(old == 1){
        free(t);
    }
}
//------------------------- value ------------------------
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
    if(type){
        atomic_add(&type->ref, 1);
    }
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
        //if is struct.
        if(val->type->base_ffi_type == HHFFI_TYPE_STRUCT){
            hffi_struct* c = val->ptr;
            hffi_delete_struct(c);
        }else{
            if(val->ptr !=NULL){
                free(val->ptr);
            }
        }
        hffi_delete_type(val->type);
        free(val);
    }
}
ffi_type* hffi_value_get_rawtype(hffi_value* val, char** msg){
    if(val->type->base_ffi_type == HHFFI_TYPE_STRUCT){
        return ((hffi_struct*)(val->ptr))->type;
    }
    return to_ffi_type(val->type->base_ffi_type, msg);
}
//----------------------------- ------------------------------
#define __hffi_get_ptr(v) (v->type->base_ffi_type != HHFFI_TYPE_STRUCT ? (v->type->base_ffi_type != FFI_TYPE_POINTER ? v->ptr : &v->ptr): ((hffi_struct*)(v->ptr))->data)

int hffi_call(void (*fn)(void), hffi_value** in, int in_count,hffi_value* out, char** msg){
    //param types with values
    ffi_type ** argTypes = NULL;
    void **args = NULL;
    if(in_count > 0){
        argTypes = alloca(sizeof(ffi_type *) *in_count);
        args = alloca(sizeof(void *) *in_count);
        for(int i = 0 ; i < in_count ; i ++){
            argTypes[i] = hffi_value_get_rawtype(in[i], msg);
            if(argTypes[i] == NULL){
                return 1;
            }
            //cast param value
            args[i] = __hffi_get_ptr(in[i]);
        }
    }
    //prepare call
    ffi_cif cif;
    ffi_type *return_type = hffi_value_get_rawtype(out, msg);
    if(return_type == NULL){
        return 1;
    }
    ffi_status s = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, (unsigned int)in_count, return_type, argTypes);
    switch (s) {
    case FFI_OK:{
        ffi_call(&cif, fn, __hffi_get_ptr(out), args);
        return 0;
    }break;

    case FFI_BAD_ABI:
        if(msg){
            strcpy(*msg, "FFI_BAD_ABI");
        }
        break;
    case FFI_BAD_TYPEDEF:
        if(msg){
            strcpy(*msg, "FFI_BAD_TYPEDEF");
        }
        break;
    }
    return 1;
}

//----------------------------------------------------------
hffi_closure* hffi_new_closure(void* func_ptr, void (*fun_proxy)(ffi_cif*,void* ret,void** args,void* ud),
                               sint8* in, int in_count, sint8 return_t, void* ud, char** msg){
    hffi_closure* ptr = malloc(sizeof(hffi_closure));
    ptr->ref = 1;
    ptr->code = &func_ptr;
    ptr->param_count = in_count;
    ptr->return_type = return_t;
    for(int i = 0 ; i < in_count ; i ++){
       ptr->param_types[i] = in[i];
    }
    //create closure, prepare
    ptr->closure = ffi_closure_alloc(sizeof(ffi_closure), &func_ptr);
    if(ptr->closure == NULL){
        if(msg){
            strcpy(*msg, "create closure failed!");
        }
        return NULL;
    }

    //param
    ffi_type ** argTypes = NULL;
    if(in_count > 0){
        argTypes = alloca(sizeof(ffi_type *) *in_count);
        for(int i = 0 ; i < in_count ; i ++){
            //TODO struct
            argTypes[i] = to_ffi_type(in[i], msg);
            if(argTypes[i] == NULL){
                return NULL;
            }
        }
    }
    //return type
    //TODO struct
    ffi_type *return_type = to_ffi_type(return_t, msg);
    if(return_type == NULL){
        return NULL;
    }
    //prepare
    ffi_cif cif;
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, in_count, return_type, argTypes) == FFI_OK) {
        //concat closure with proxy.
        if (ffi_prep_closure_loc(ptr->closure, &cif, fun_proxy, ud, func_ptr) == FFI_OK) {
            return ptr; //now we can call func
        }else{
            if(msg){
                strcpy(*msg, "concat closure with function proxy failed!");
            }
        }
    }else{
        if(msg){
            strcpy(*msg, "ffi_prep_cif(...) failed!");
        }
    }
    return NULL;
}
void hffi_delete_closure(hffi_closure* val){
    int old = atomic_add(&val->ref, -1);
    if(old == 1){
        if(val->closure){
            ffi_closure_free(val->closure);
        }
        free(val);
    }
}

hffi_struct* hffi_new_struct(sint8* types, int count, char** msg){
    //struct data
    ffi_type *type;
    size_t *offsets;
    //raw_type, elements, offsets.
    type = (ffi_type *) malloc(sizeof(*type) + sizeof (ffi_type *) * (count+1) + sizeof (offsets[0]) * count);
    type->size = type->alignment = 0;
    type->type = FFI_TYPE_STRUCT;
    type->elements = (ffi_type **) (type + 1);
    ffi_type* tmp_type;
    for(int i = count -1 ; i >=0 ; i --){
        //TODO struct.
        if(types[i] == HHFFI_TYPE_STRUCT){

        }
        tmp_type = to_ffi_type(types[i], msg);
        if(tmp_type == NULL){
            free(type);
            return NULL;
        }
        type->elements[i] = tmp_type;
    }
    type->elements[count] = NULL;
    offsets = (size_t *) &type->elements[count+1];
    ffi_status status = ffi_get_struct_offsets(FFI_DEFAULT_ABI, type, offsets);
    if (status != FFI_OK){
        free(type);
        if(msg){
            strcpy(*msg, "ffi_get_struct_offsets() failed.");
        }
        return NULL;
    }
    //compute total size after align. make total_size align of 8
    int total_size = offsets[count  - 1] +  type->elements[count  - 1]->size;//TODO verify ok ?
    if(total_size % sizeof (long long) != 0){
        total_size += 8 - (total_size % sizeof (long long));
    }

    //create hffi_struct to manage struct.
    hffi_struct* ptr = malloc(sizeof(hffi_struct));
    ptr->type = type;
    ptr->ref = 1;
    ptr->count = count;
    ptr->data_size = total_size;
    ptr->data = malloc(total_size);
    return ptr;
}

void hffi_delete_struct(hffi_struct* val){
    int old = atomic_add(&val->ref, -1);
    if(old == 1){
        if(val->type){
            free(val->type);
        }
        if(val->data){
            free(val->data);
        }
        free(val);
    }
}
hffi_value* hffi_struct_to_value(hffi_struct* c){
    hffi_value* val_ptr = malloc(sizeof(hffi_value));
    val_ptr->ptr = c;
    val_ptr->type = hffi_new_type_base(HHFFI_TYPE_STRUCT);
    val_ptr->ref = 1;
    atomic_add(&c->ref, 1);
    return val_ptr;
}
hffi_value* hffi_struct_ptr_to_value(hffi_struct* c){
    hffi_value* val_ptr = malloc(sizeof(hffi_value));
    val_ptr->ptr = c;
    val_ptr->type = hffi_new_type_base(HHFFI_TYPE_POINTER);
    val_ptr->ref = 1;
    atomic_add(&c->ref, 1);
    return val_ptr;
}



