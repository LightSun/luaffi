
#include <stdio.h>
#include "hffi.h"
#include "atomic.h"
#include "h_linklist.h"
#include "h_alloctor.h"
#include "h_list.h"
#include "h_array.h"

#define hffi_new_value_auto_x(ffi_t,type) \
hffi_value* hffi_new_value_##type(type val){\
    hffi_value* v = hffi_new_value(ffi_t, HFFI_TYPE_VOID,sizeof(type*));\
    type* p = (type*)v->ptr;\
    *p = val;\
    return v;\
}

#define hffi_value_get_auto_x(t)\
int hffi_value_get_##t(hffi_value* val, t* out_ptr){\
    char _m[128];\
    char* msg[1];\
    msg[0] = _m;\
    ffi_type* ft = to_ffi_type(val->base_ffi_type, msg);\
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

#define DEF_NEW_VALUE_RAW(ffi_t, type)\
case ffi_t:{\
    hffi_value* hv = hffi_new_value(ffi_t, HFFI_TYPE_VOID, sizeof (type*));\
    *((type*)hv->ptr) = *((type*)val_ptr);\
    return hv;\
}break;

#define DEF_NEW_VALUE_PTR2(ffi_t, type)\
case ffi_t:{\
    hffi_value* hv = hffi_new_value(HFFI_TYPE_POINTER, ffi_t, sizeof (type*));\
    *((type*)hv->ptr) = *((type*)val_ptr);\
    return hv;\
}break;

typedef struct struct_item{
    int index;      //the index of struct
    sint8 hffi_t;
    sint8 hffi_t2; //if is pointer. may need second. or else indicate if copy data(1) or set data-ptr(0)
    void* ptr;     // harray* or hffi_struct*
}struct_item;

#define _ITEM_COPY_DATA 1    // need copy struct data
#define _ITEM_SET_DATA_PTR 0 // need set data ptr from parent

static inline struct_item* __new_struct_item(int index, sint8 hffi_t, sint8 hffi_t2, void* ptr){
    struct_item* item = MALLOC(sizeof (struct_item));
    item->index = index;
    item->hffi_t = hffi_t;
    item->hffi_t2 = hffi_t2;
    item->ptr = ptr;
    return item;
}
static void __release_struct_item(void* d){
    struct_item* item = d;

    switch (item->hffi_t) {
    case HFFI_TYPE_STRUCT:
    case HFFI_TYPE_STRUCT_PTR:{
        hffi_delete_struct((hffi_struct*)item->ptr);
    }break;
    case HFFI_TYPE_HARRAY_PTR:
    case HFFI_TYPE_HARRAY:{
        harray_delete((harray*)item->ptr);
    }break;
    }
    FREE(item);
}

static int __struct_find_struct_harray(void* ud,int size, int index,void* ele){
    H_UNSED2(size, index)
    int req_index = *((int*)ud);
    struct_item* item = ele;
    if(item->index == req_index){
        return 0;
    }
    return -1;
}

#define DEF_GET_AS_ARRAY_IMPL(ffi_t, type)\
case ffi_t:{\
    if(cols == 0){ /* int* a = ..., ptr = a*/ \
        int len = rows * sizeof (type);\
        void* data;\
        if(share_memory){\
            data = _ptr;\
        }else{\
            data = MALLOC(len);\
            memcpy(data, _ptr, len);\
        }\
        return harray_new_from_data(ffi_t, data, len, rows, !share_memory);\
    }else{\
        harray* arr; harray* tmp_arr;\
        if(continue_mem){\
            /*int** a = ..., ptr = a */\
            int len = rows * sizeof (void*);\
            void* data = MALLOC(len);\
            arr = harray_new_from_data(HFFI_TYPE_HARRAY_PTR, data, len, rows, 1);\
            type* sd = _ptr;\
            for(int i = 0 ; i < rows ; i ++){\
                tmp_arr = harray_new_from_data(ffi_t, sd, cols * sizeof (type), cols, 0);\
                tmp_arr->ref = 0;\
                harray_seti2(arr, i, tmp_arr);\
                sd += cols;\
            }\
        }else{\
            /* int** a = malloc...., *a[0] = malloc..., *a[1] = malloc...,  ptr = a; */\
            int len = rows * cols * sizeof (void*);\
            void* data;\
            if(share_memory){\
                data = _ptr;\
            }else{\
                data = MALLOC(len);\
                memcpy(data, _ptr, len);\
            }\
            arr = harray_new_from_data(HFFI_TYPE_HARRAY_PTR, data, len, rows, !share_memory);\
            /*no continue memory. the data[i] need free.*/\
            for(int i = 0 ; i < rows ; i ++){\
                tmp_arr = harray_new_from_data(ffi_t, ((void**)data)[i], cols * sizeof (type), cols, 1);\
                tmp_arr->ref = 0;\
                harray_seti2(arr, i, tmp_arr);\
            }\
        }\
        return arr;\
    }\
}break;
static harray* hffi_get_pointer_as_array_impl(sint8 ffi_t, void* _ptr, int rows, int cols,int continue_mem, int share_memory){
    DEF_HFFI_BASE_SWITCH(DEF_GET_AS_ARRAY_IMPL, ffi_t);
//    switch (ffi_t) {
//    case HFFI_TYPE_SINT8:{
//        if(cols == 0){//int* a = ..., ptr = a
//            int len = rows * sizeof (sint8);
//            void* data;
//            if(share_memory){
//                data = _ptr;
//            }else{
//                data = MALLOC(len);
//                memcpy(data, _ptr, len);
//            }
//            return harray_new_from_data(HFFI_TYPE_SINT8, data, len, rows, !share_memory);
//        }else{
//            harray* arr; harray* tmp_arr;
//            if(continue_mem){
//                //int** a = ..., ptr = a
//                int len = rows * cols * sizeof (sint8);
//                void* data;
//                if(share_memory){
//                    data = _ptr;
//                }else{
//                    data = MALLOC(len);
//                    memcpy(data, _ptr, len);
//                }
//                arr = harray_new_from_data(HFFI_TYPE_HARRAY, data, len, rows, !share_memory);
//                sint8** sd = data;
//                for(int i = 0 ; i < rows ; i ++){
//                    tmp_arr = harray_new_from_data(HFFI_TYPE_SINT8, sd[i], cols * sizeof (sint8), cols, 0);
//                    tmp_arr->ref = 0;
//                    harray_seti2(arr, i, tmp_arr);
//                }
//            }else{
//                //int** a = malloc...., *a[0] = malloc..., *a[1] = malloc...,  ptr = a;
//                int len = rows * cols * sizeof (void*);
//                void* data;
//                if(share_memory){
//                    data = _ptr;
//                }else{
//                    data = MALLOC(len);
//                    memcpy(data, _ptr, len);
//                }
//                arr = harray_new_from_data(HFFI_TYPE_HARRAY_PTR, data, len, rows, !share_memory);
//                //no continue memory. the data[i] need free.
//                for(int i = 0 ; i < rows ; i ++){
//                    tmp_arr = harray_new_from_data(HFFI_TYPE_SINT8, ((void**)data)[i], cols * sizeof (sint8), cols, 1);
//                    tmp_arr->ref = 0;
//                    harray_seti2(arr, i, tmp_arr);
//                }
//            }
//            return arr;
//        }
//    }break;
//    }
    return NULL;
}

//---------------------------------------------------

static void __set_children_data(hffi_struct* p);
static inline void __release_ffi_type_simple(void* data){
     FREE(data);
}
static inline ffi_type* __harray_to_ffi_type(harray* arr, array_list* sub_types){
    ffi_type* type = NULL;
    int c = harray_get_count(arr);
//HFFI_TYPE_STRUCT_PTR, HFFI_TYPE_HARRAY_PTR
    switch (arr->hffi_t) {
    case HFFI_TYPE_STRUCT:{
        type = MALLOC(sizeof (ffi_type) + sizeof (ffi_type*) * (c + 1));
        type->type = FFI_TYPE_STRUCT;
        type->alignment = arr->data_size/arr->ele_count; // 1?
        type->size = arr->data_size;
        type->elements = (ffi_type**)(type + 1);
        type->elements[c] = NULL;
        union harray_ele ele;
        for(int i = 0 ; i < c ; i ++){
            harray_geti(arr, i , &ele);
            type->elements[i] = ((hffi_struct*)ele._extra)->type;
        }
    }break;

    case HFFI_TYPE_HARRAY:{
        type = MALLOC(sizeof (ffi_type) + sizeof (ffi_type*) * (c + 1));
        type->type = FFI_TYPE_STRUCT;
        type->alignment = arr->data_size/arr->ele_count;
        type->size = arr->data_size;
        type->elements = (ffi_type**)(type + 1);
        type->elements[c] = NULL;
        union harray_ele ele;
        for(int i = 0 ; i < c ; i ++){
            harray_geti(arr, i , &ele);
            type->elements[i] = __harray_to_ffi_type((harray*)ele._extra, sub_types);
        }
    }break;

    case HFFI_TYPE_STRUCT_PTR:
    case HFFI_TYPE_HARRAY_PTR:{
        type = MALLOC(sizeof (ffi_type) + sizeof (ffi_type*) * (c + 1));
        type->type = FFI_TYPE_STRUCT;
        type->alignment = arr->data_size/arr->ele_count;
        type->size = arr->data_size;
        type->elements = (ffi_type**)(type + 1);
        type->elements[c] = NULL;
        for(int i = 0 ; i < c ; i ++){
            type->elements[i] = &ffi_type_pointer;
        }
    }break;

    default:
    {
        type = MALLOC(sizeof (ffi_type));
        type->type = FFI_TYPE_STRUCT;
        //from: https://github.com/libffi/libffi/issues/394
        ffi_type* ffi_type_nullptr = NULL; //Hold a null pointer.

        ffi_type* sub_type = to_ffi_type(arr->hffi_t, NULL);
        type->alignment = sub_type->alignment;
        type->size = arr->data_size;   // sub_type->size * 4
        type->elements = &ffi_type_nullptr;
    }break;
    }
    if(type){
        array_list_add(sub_types, type);
    }
    return type;
}

ffi_type* to_ffi_type(int8_t v, char** msg){
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
//------------------------- value ------------------------
hffi_value* hffi_new_value_ptr(sint8 hffi_t2){
    return hffi_new_value(FFI_TYPE_POINTER, hffi_t2, sizeof (void*));
}
hffi_value* hffi_new_value(sint8 hffi_t, sint8 hffi_t2, int size){
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    memset(val_ptr, 0, sizeof (hffi_value));
    val_ptr->ptr = MALLOC(size);
    val_ptr->base_ffi_type = hffi_t;
    val_ptr->pointer_base_type = hffi_t2;
    val_ptr->ref = 1;   
    return val_ptr;
}
hffi_value* hffi_new_value_ptr_no_data(sint8 hffi_t2){
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    memset(val_ptr, 0, sizeof (hffi_value));
    val_ptr->ptr = NULL;
    val_ptr->multi_level_ptr = 1;
    val_ptr->base_ffi_type = HFFI_TYPE_POINTER;
    val_ptr->pointer_base_type = hffi_t2;
    val_ptr->ref = 1;
    return val_ptr;
}

hffi_new_value_auto_x(HFFI_TYPE_SINT8, sint8)
hffi_new_value_auto_x(HFFI_TYPE_SINT16, sint16)
hffi_new_value_auto_x(HFFI_TYPE_SINT32, sint32)
hffi_new_value_auto_x(HFFI_TYPE_SINT64, sint64)
hffi_new_value_auto_x(HFFI_TYPE_UINT8, uint8)
hffi_new_value_auto_x(HFFI_TYPE_UINT16, uint16)
hffi_new_value_auto_x(HFFI_TYPE_UINT32, uint32)
hffi_new_value_auto_x(HFFI_TYPE_UINT64, uint64)
hffi_new_value_auto_x(HFFI_TYPE_INT, int)
hffi_new_value_auto_x(HFFI_TYPE_FLOAT, float)
hffi_new_value_auto_x(HFFI_TYPE_DOUBLE, double)

hffi_value_get_auto_x(sint8)
hffi_value_get_auto_x(sint16)
hffi_value_get_auto_x(sint32)
hffi_value_get_auto_x(sint64)
hffi_value_get_auto_x(uint8)
hffi_value_get_auto_x(uint16)
hffi_value_get_auto_x(uint32)
hffi_value_get_auto_x(uint64)
hffi_value_get_auto_x(int)
hffi_value_get_auto_x(float)
hffi_value_get_auto_x(double)

hffi_value* hffi_new_value_raw_type2(sint8 ffi_t, void* val_ptr){
    DEF_HFFI_BASE_SWITCH(DEF_NEW_VALUE_RAW, ffi_t)
    return NULL;
}
hffi_value* hffi_new_value_ptr2(sint8 ffi_t, void* val_ptr){
    DEF_HFFI_BASE_SWITCH(DEF_NEW_VALUE_PTR2, ffi_t)
    return NULL;
}

hffi_value* hffi_new_value_raw_type(sint8 ffi_t){
//    int size = 0;
//    switch (ffi_t) {
//    case HFFI_TYPE_SINT8:{size = sizeof (sint8);}break;
//    case HFFI_TYPE_UINT8:{size = sizeof (uint8);}break;
//    case HFFI_TYPE_SINT16:{size = sizeof (sint16);}break;
//    case HFFI_TYPE_UINT16:{size = sizeof (uint16);}break;
//    case HFFI_TYPE_SINT32:{size = sizeof (sint32);}break;
//    case HFFI_TYPE_UINT32:{size = sizeof (uint32);}break;
//    case HFFI_TYPE_SINT64:{size = sizeof (sint64);}break;
//    case HFFI_TYPE_UINT64:{size = sizeof (uint64);}break;
//    case HFFI_TYPE_FLOAT:{size = sizeof (float);}break;
//    case HFFI_TYPE_DOUBLE:{size = sizeof (double);}break;

//    case HFFI_TYPE_INT:{size = sizeof (sint32);}break;
//    default:
//        return NULL;
//    }
   // return hffi_new_value(ffi_t, HFFI_TYPE_VOID, size);
    return hffi_new_value(ffi_t, HFFI_TYPE_VOID, sizeof(void*));
}
hffi_value* hffi_new_value_struct(hffi_struct* c){
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    memset(val_ptr, 0, sizeof (hffi_value));
    val_ptr->ptr = c;
    val_ptr->base_ffi_type = HFFI_TYPE_STRUCT;
    val_ptr->pointer_base_type = HFFI_TYPE_VOID;
    val_ptr->ref = 1;
    atomic_add(&c->ref, 1);
    return val_ptr;
}
hffi_value* hffi_new_value_struct_ptr(hffi_struct* c){
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    memset(val_ptr, 0, sizeof (hffi_value));
    val_ptr->ptr = c;
    val_ptr->base_ffi_type = HFFI_TYPE_POINTER;
    val_ptr->pointer_base_type = HFFI_TYPE_STRUCT;
    val_ptr->ref = 1;
    atomic_add(&c->ref, 1);
    return val_ptr;
}
hffi_value* hffi_new_value_harray_ptr(harray* c){
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    memset(val_ptr, 0, sizeof (hffi_value));
    val_ptr->ptr = c;
    val_ptr->base_ffi_type = HFFI_TYPE_POINTER;
    val_ptr->pointer_base_type = HFFI_TYPE_HARRAY;
    val_ptr->ref = 1;
    atomic_add(&c->ref, 1);
    return val_ptr;
}
hffi_value* hffi_new_value_harray(struct harray* arr){
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    val_ptr->sub_types = array_list_new2(4);
    val_ptr->ffi_type = __harray_to_ffi_type(arr, val_ptr->sub_types);
    val_ptr->ptr = arr;
    val_ptr->base_ffi_type = HFFI_TYPE_HARRAY;
    val_ptr->pointer_base_type = HFFI_TYPE_VOID;
    val_ptr->ref = 1;
    atomic_add(&arr->ref, 1);
    return val_ptr;
}
hffi_value* hffi_value_copy(hffi_value* val){
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    memset(val_ptr, 0, sizeof (hffi_value));
    val_ptr->base_ffi_type = val->base_ffi_type;
    val_ptr->pointer_base_type = val->pointer_base_type;
    val_ptr->ref = 1;
    val_ptr->multi_level_ptr = val->multi_level_ptr;

    switch (val->base_ffi_type) {
    case HFFI_TYPE_HARRAY:{
        val_ptr->ptr = harray_copy((harray*)val->ptr);
        val_ptr->sub_types = array_list_new2(4);
        val_ptr->ffi_type = __harray_to_ffi_type((harray*)val->ptr, val_ptr->sub_types);
    }break;
    case HFFI_TYPE_STRUCT:{
         val_ptr->ptr = hffi_struct_copy((hffi_struct*)val->ptr);
    }break;
    case HFFI_TYPE_POINTER:{
        if(val->pointer_base_type == HFFI_TYPE_HARRAY){
            val_ptr->ptr = harray_copy((harray*)val->ptr);
        }else if(val->pointer_base_type == HFFI_TYPE_STRUCT){
            val_ptr->ptr = hffi_struct_copy((hffi_struct*)val->ptr);
        }
    }break;
    }
    return val_ptr;
}
void hffi_delete_value(hffi_value* val){
    int old = atomic_add(&val->ref, -1);
    if(old == 1){
        //delete ptr
        switch (val->base_ffi_type) {
        case HFFI_TYPE_STRUCT:{
            hffi_struct* c = val->ptr;
            hffi_delete_struct(c);
        }break;
        case HFFI_TYPE_HARRAY:{
            harray* arr = val->ptr;
            harray_delete(arr);
        }break;

        case HFFI_TYPE_POINTER:{
            if(val->pointer_base_type == HFFI_TYPE_STRUCT){
                hffi_struct* c = val->ptr;
                hffi_delete_struct(c);
            }else if(val->pointer_base_type == HFFI_TYPE_HARRAY){
                harray* arr = val->ptr;
                harray_delete(arr);
            }else{
                if(val->ptr !=NULL){
                    FREE(val->ptr);
                }
            }
        }break;

        default:
            if(val->ptr !=NULL){
                FREE(val->ptr);
            }
        }
        //types
        if(val->sub_types){
            array_list_delete2(val->sub_types, __release_ffi_type_simple);
        }
        FREE(val);
    }
}

void hffi_value_ref(hffi_value* val, int count){
     atomic_add(&val->ref, count);
}

#define DEF_VALUE_GET_BASE_IMPL(ffi_t, type)\
case ffi_t:{\
    *((type*)out_ptr) = *((type*)val->ptr);\
    return HFFI_STATE_OK;\
}break;

int hffi_value_get_base(hffi_value* val, void* out_ptr){
    int ffi_t = val->base_ffi_type;
    if(ffi_t == HFFI_TYPE_POINTER){
        ffi_t = val->pointer_base_type;
    }
    DEF_HFFI_BASE_SWITCH(DEF_VALUE_GET_BASE_IMPL, ffi_t);
    return HFFI_STATE_FAILED;
}
//can be used to normal value. int/float... or its' simple ptr. eg: int*(only have one int value)
int hffi_value_set_base(hffi_value* val, void* in_ptr){
#define DEF_hffi_value_set_base_impl(ffi_t, type)\
case ffi_t:{\
    *((type*)val->ptr) = *((type*)in_ptr);\
    return HFFI_STATE_OK;\
}break;
    int ffi_t = val->base_ffi_type;
    if(ffi_t == HFFI_TYPE_POINTER){
        ffi_t = val->pointer_base_type;
    }
    DEF_HFFI_BASE_SWITCH(DEF_hffi_value_set_base_impl, ffi_t)
    return HFFI_STATE_FAILED;
}
ffi_type* hffi_value_get_rawtype(hffi_value* val, char** msg){
    if(val->base_ffi_type == HFFI_TYPE_STRUCT){
        return ((hffi_struct*)(val->ptr))->type;
    }
    if(val->base_ffi_type == HFFI_TYPE_HARRAY){
        if(!val->ffi_type){
            if(!val->sub_types) val->sub_types = array_list_new2(4);
            val->ffi_type = __harray_to_ffi_type(val->ptr, val->sub_types);
        }
        return val->ffi_type;
    }
    return to_ffi_type(val->base_ffi_type, msg);
}
hffi_struct* hffi_value_get_struct(hffi_value* val){
    if(val->base_ffi_type == HFFI_TYPE_STRUCT){
        return (hffi_struct*)val->ptr;
    }
    if(val->base_ffi_type == HFFI_TYPE_POINTER && val->pointer_base_type == HFFI_TYPE_STRUCT){
        return (hffi_struct*)val->ptr;
    }
    return NULL;
}
harray* hffi_value_get_harray(hffi_value* val){
    if(val->base_ffi_type == HFFI_TYPE_HARRAY){
        return (harray*)val->ptr;
    }
    if(val->base_ffi_type == HFFI_TYPE_POINTER && val->pointer_base_type == HFFI_TYPE_HARRAY){
        return (harray*)val->ptr;
    }
    return NULL;
}
/** continueMemory: sometimes, some memory malloc split for array-array. */
harray* hffi_value_get_pointer_as_array(hffi_value* val, int rows, int cols,int continue_mem, int share_memory){
    return hffi_get_pointer_as_array_impl(val->pointer_base_type, val->ptr, rows, cols, continue_mem, share_memory);
}
//----------------------------- ------------------------------
static inline void* __get_data_ptr(hffi_value* v){
    switch (v->base_ffi_type) {
    case HFFI_TYPE_STRUCT:
    {
        return ((hffi_struct*)(v->ptr))->data;
    }

    case HFFI_TYPE_POINTER:
    {
        if(v->pointer_base_type == HFFI_TYPE_STRUCT){
            return &((hffi_struct*)(v->ptr))->data;
        }else if(v->pointer_base_type == HFFI_TYPE_HARRAY){
            return &((harray*)(v->ptr))->data;
        }else{
            return &v->ptr;
        }
    }break;

    case HFFI_TYPE_HARRAY:{
        return ((harray*)(v->ptr))->data;
    }break;

    default:
    {
        return v->ptr;
    };
    }
}
int hffi_call(void (*fn)(void), hffi_value** in, hffi_value* out, char** msg){
    return hffi_call_abi(FFI_DEFAULT_ABI, fn, in, out, msg);
}
int hffi_call_abi(int abi,void (*fn)(void), hffi_value** in,hffi_value* out, char** msg){
    hffi_get_pointer_count(in_count, in);
    //param types with values
    ffi_type ** argTypes = NULL;
    void **args = NULL;
    if(in_count > 0){
        argTypes = alloca(sizeof(ffi_type *) *in_count);
        args = alloca(sizeof(void *) *in_count);
        for(int i = 0 ; i < in_count ; i ++){
            argTypes[i] = hffi_value_get_rawtype(in[i], msg);
            if(argTypes[i] == NULL){
                return HFFI_STATE_FAILED;
            }
            //cast param value
            args[i] = __get_data_ptr(in[i]);
        }
    }
    //prepare call
    ffi_cif cif;
    ffi_type *return_type = hffi_value_get_rawtype(out, msg);
    if(return_type == NULL){
        return HFFI_STATE_FAILED;
    }
    ffi_status s = ffi_prep_cif(&cif, abi, (unsigned int)in_count, return_type, argTypes);
    switch (s) {
    case FFI_OK:{
        ffi_call(&cif, fn, __get_data_ptr(out), args);
        return HFFI_STATE_OK;
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
    return HFFI_STATE_FAILED;
}
//-----------------------------------------------
static void __smtypes_travel_ref(void* ud, int size, int index,void* ele){
    H_UNSED(ud)
    H_UNSED(size)
    H_UNSED(index)
    hffi_smtype* t = ele;
    atomic_add(&t->ref, 1);
}
static inline hffi_smtype* __hffi_new_smtype0(sint8 ffi_type, array_list* member_types){
    hffi_smtype* ptr = MALLOC(sizeof (hffi_smtype));
    ptr->ffi_type = ffi_type;
    ptr->ref = 1;
    ptr->_struct = NULL;
    ptr->_harray = NULL;
    ptr->elements = member_types;
    if(member_types != NULL){
        array_list_travel(member_types, __smtypes_travel_ref, NULL);
    }
    return ptr;
}
hffi_smtype* hffi_new_smtype(sint8 ffi_type){
    return __hffi_new_smtype0(ffi_type, NULL);
}
hffi_smtype* hffi_new_smtype_members(struct array_list* member_types){
    return __hffi_new_smtype0(HFFI_TYPE_STRUCT, member_types);
}
hffi_smtype* hffi_new_smtype_struct(hffi_struct* _struct){
    hffi_smtype* ptr = MALLOC(sizeof (hffi_smtype));
    ptr->ffi_type = HFFI_TYPE_STRUCT;
    ptr->ref = 1;
    ptr->_struct = _struct;
    ptr->_harray = NULL;
    ptr->elements = NULL;
    atomic_add(&_struct->ref, 1);
    return ptr;
}
hffi_smtype* hffi_new_smtype_struct_ptr(hffi_struct* _struct){
    hffi_smtype* ptr = MALLOC(sizeof (hffi_smtype));
    ptr->ffi_type = HFFI_TYPE_POINTER;
    ptr->ref = 1;
    ptr->_struct = _struct;
    ptr->_harray = NULL;
    ptr->elements = NULL;
    atomic_add(&_struct->ref, 1);
    return ptr;
}
hffi_smtype* hffi_new_smtype_harray(struct harray* array){
    hffi_smtype* ptr = MALLOC(sizeof (hffi_smtype));
    ptr->ffi_type = HFFI_TYPE_HARRAY;
    ptr->ref = 1;
    ptr->_harray = array;
    ptr->_struct = NULL;
    ptr->elements = NULL;
    atomic_add(&array->ref, 1);
    return ptr;
}
hffi_smtype* hffi_new_smtype_harray_ptr(struct harray* array){
    hffi_smtype* ptr = MALLOC(sizeof (hffi_smtype));
    ptr->ffi_type = HFFI_TYPE_POINTER;
    ptr->ref = 1;
    ptr->_harray = array;
    ptr->_struct = NULL;
    ptr->elements = NULL;
    atomic_add(&array->ref, 1);
    return ptr;
}
void list_travel_smtype_delete(void* d){
    hffi_smtype* t = d;
    hffi_delete_smtype(t);
}
void hffi_delete_smtype(hffi_smtype* type){
    int old = atomic_add(&type->ref, -1);
    if(old == 1){
        if(type->elements != NULL){
            array_list_delete2(type->elements, list_travel_smtype_delete);
        }
        if(type->_struct != NULL){
            hffi_delete_struct((hffi_struct*)type->_struct);
        }
        if(type->_harray != NULL){
            harray_delete((harray*)type->_harray);
        }
        FREE(type);
    }
}
void hffi_smtype_ref(hffi_smtype* src, int c){
    atomic_add(&src->ref, c);
}
hffi_smtype* hffi_smtype_cpoy(hffi_smtype* src){
    hffi_smtype* ptr = MALLOC(sizeof (hffi_smtype));
    ptr->ffi_type = src->ffi_type;
    ptr->ref = 1;
    if(src->_harray){
        ptr->_harray = harray_copy((harray*)src->_harray);
    }else{
        ptr->_harray = NULL;
    }
    if(src->_struct){
        ptr->_struct = hffi_struct_copy((hffi_struct*)src->_struct);
    }else{
        ptr->_struct = NULL;
    }
    if(src->elements){
        int c = array_list_size(src->elements);
        ptr->elements = array_list_new2(c * 4 / 3 + 1);
        hffi_smtype* tmp_smtype;
        for(int i = 0 ; i < c ; i ++){
            tmp_smtype = array_list_get(src->elements, i);
            array_list_add(ptr->elements, hffi_smtype_cpoy(tmp_smtype));
        }
    }else{
        src->elements = NULL;
    }
    return ptr;
}
//--------------------------------------------------------------
static inline void  __set_children_data_travel(hffi_struct* p,struct_item* item){
    size_t * offsets = (size_t *) &p->type->elements[p->count+1];
    void* target_ptr = (p->data + offsets[item->index]);

    switch (item->hffi_t) {
    case HFFI_TYPE_STRUCT_PTR:{
        hffi_struct* stu = ((hffi_struct*)item->ptr);        
        memcpy(target_ptr, &stu->data, sizeof (void*));
    }break;
    case HFFI_TYPE_HARRAY_PTR:{
        harray* stu = ((harray*)item->ptr);
        memcpy(target_ptr, &stu->data, sizeof (void*));
    }break;
    case HFFI_TYPE_HARRAY:{
        //copy data
        harray* stu = ((harray*)item->ptr);
        memcpy(target_ptr, stu->data, stu->data_size);
    }break;
    case HFFI_TYPE_STRUCT:{
        hffi_struct* stu = ((hffi_struct*)item->ptr);
        if(item->hffi_t2 == _ITEM_COPY_DATA){
            memcpy(target_ptr, stu->data, stu->data_size);
        }else{
            //make child ptr to parents.
            assert(stu->data == NULL);
            stu->data = target_ptr;
            __set_children_data(stu);
        }
    }break;
    }
}
//just set ptrs. no need set 'HFFI_TYPE_HARRAY' and 'HFFI_TYPE_STRUCT'.

static inline void __set_children_data(hffi_struct* p){
    int c = array_list_size(p->children);
    for(int i = 0 ; i < c ; i ++){
        __set_children_data_travel(p, (struct_item*)array_list_get(p->children, i));
    }
}

//------------------------
static hffi_struct* hffi_new_struct_from_list0(int abi,struct array_list* list, sint16 parent_pos, char** msg);
static hffi_struct* hffi_new_struct_abi0(int abi,hffi_smtype** member_types, sint16 parent_pos, char** msg);
//------------------------
hffi_struct* hffi_new_struct_base(sint8* types, int count){
    hffi_smtype* smtypes[count +1];
    for(int i = 0 ; i < count; i ++){
        smtypes[i] = hffi_new_smtype(types[i]);
    }
    smtypes[count] = NULL;
    hffi_struct* c = hffi_new_struct(smtypes, NULL);
    for(int i = 0 ; i < count; i ++){
        hffi_delete_smtype(smtypes[i]);
    }
    return c;
}
hffi_struct* hffi_new_struct_from_list2(int abi,struct array_list* list, char** msg){
    return hffi_new_struct_from_list0(abi, list, -1, msg);
}

hffi_struct* hffi_new_struct_from_list(struct array_list* list, char** msg){
    return hffi_new_struct_from_list0(FFI_DEFAULT_ABI, list, -1, msg);
}
static inline hffi_struct* hffi_new_struct_from_list0(int abi,struct array_list* list, sint16 parent_pos, char** msg){
    int count = array_list_size(list);
    //child structs
    array_list* children = array_list_new2(count / 2 < 4 ? 4 : count / 2);
    //sub_types(ffi_type*) which need manmul release
    array_list* sub_types = array_list_new_simple();
    //record types for latter use.
    sint8* hffi_types = MALLOC(sizeof (sint8) * count);

    //struct data
    int type_size = sizeof(ffi_type) + sizeof (ffi_type *) * (count+1) + sizeof (size_t) * count;
    ffi_type *type;
    size_t * offsets = NULL;
    //raw_type, elements, offsets.
    type = (ffi_type *) MALLOC(type_size);
    type->size = type->alignment = 0;
    type->type = FFI_TYPE_STRUCT;
    type->elements = (ffi_type**)(type + 1);
    type->elements[count] = NULL;
    /*
     * 1, already-struct : copy struct data to parent target-pos.
     * 2. already-struct-ptr:set parent target-pos-ptr to the struct data-ptr
     * 3, already-harray: copy array data to parent (note: harray-nested struct.)
     * 4, already-harray-ptr: set parent target-pos-ptr to the harray data-ptr
     * 5, hffi_smtype for struct: create new struct and make the struct data-ptr to parent's.
     */
    ffi_type* tmp_type;
    hffi_smtype* tmp_smtype;
    hffi_struct* tmp_struct;
    harray* tmp_harr;
    for(int i = count -1 ; i >=0 ; i --){
        //struct.
        tmp_smtype = array_list_get(list, i);
        if(tmp_smtype->ffi_type == HFFI_TYPE_STRUCT){
            hffi_types[i] = HFFI_TYPE_STRUCT;
            if(tmp_smtype->_struct != NULL){
                //is already a struct
                tmp_struct = (hffi_struct*)tmp_smtype->_struct;
                atomic_add(&tmp_struct->ref, 1);
                tmp_type = tmp_struct->type;
                array_list_add(children, __new_struct_item(i, HFFI_TYPE_STRUCT, _ITEM_COPY_DATA, tmp_struct));
            }else{
                //create sub struct. latter will set data-memory-pointer
                tmp_struct = hffi_new_struct_from_list0(abi, tmp_smtype->elements, i, msg);
                if(tmp_struct == NULL){
                    goto failed;
                }
                tmp_type = tmp_struct->type;
                array_list_add(children, __new_struct_item(i, HFFI_TYPE_STRUCT, _ITEM_SET_DATA_PTR, tmp_struct));
            }
        }else if(tmp_smtype->ffi_type == HFFI_TYPE_HARRAY){
            hffi_types[i] = HFFI_TYPE_HARRAY;
            if(tmp_smtype->_harray != NULL){
                tmp_harr = tmp_smtype->_harray;
                atomic_add(&tmp_harr->ref, 1);
                tmp_type = __harray_to_ffi_type(tmp_harr, sub_types);
                array_list_add(children, __new_struct_item(i, HFFI_TYPE_HARRAY, _ITEM_COPY_DATA, tmp_harr));
            }else{
                //harray must be set first.
                goto failed;
            }
        }else if(tmp_smtype->ffi_type == HFFI_TYPE_POINTER){
            tmp_type = &ffi_type_pointer;
            if(tmp_smtype->_struct != NULL){

                hffi_types[i] = HFFI_TYPE_STRUCT_PTR;
                tmp_struct = (hffi_struct*)tmp_smtype->_struct;
                atomic_add(&tmp_struct->ref, 1);
                array_list_add(children, __new_struct_item(i, HFFI_TYPE_STRUCT_PTR, _ITEM_SET_DATA_PTR, tmp_struct));
            }else if(tmp_smtype->_harray != NULL){

                hffi_types[i] = HFFI_TYPE_HARRAY_PTR;
                tmp_harr = tmp_smtype->_harray;
                atomic_add(&tmp_harr->ref, 1);
                array_list_add(children, __new_struct_item(i, HFFI_TYPE_HARRAY_PTR, _ITEM_SET_DATA_PTR, tmp_harr));
            }else{
                hffi_types[i] = HFFI_TYPE_POINTER;
            }
        }else{
            hffi_types[i] = tmp_smtype->ffi_type;
            tmp_type = to_ffi_type(tmp_smtype->ffi_type, msg);
            if(tmp_type == NULL){
                goto failed;
            }
        }
        type->elements[i] = tmp_type;
    }
    offsets = (size_t *) &type->elements[count+1];

    ffi_status status = ffi_get_struct_offsets(abi, type, offsets);
    if (status != FFI_OK){
        if(msg){
            strcpy(*msg, "ffi_get_struct_offsets() failed.");
        }
        goto failed;
    }
    //compute total size after align. make total_size align of 8?
    int total_size = offsets[count  - 1] +  type->elements[count  - 1]->size;//TODO need align?

    //create hffi_struct to manage struct.
    hffi_struct* ptr = MALLOC(sizeof(hffi_struct));
    ptr->hffi_types = hffi_types;
    ptr->type = type;
    ptr->count = count;
    ptr->data_size = total_size;
    //of create by parent struct . never need MALLOC memory.
    if(parent_pos < 0){
        ptr->data = MALLOC(total_size);
    }else{
        ptr->data = NULL;
    }
    ptr->children = children;
    ptr->ref = 1;
    ptr->parent_pos = parent_pos;
    ptr->sub_ffi_types = sub_types;
    //handle sub structs' data-pointer.
    __set_children_data(ptr);
    return ptr;

failed:
    FREE(hffi_types);
    FREE(type);
    array_list_delete2(children, __release_struct_item);
    array_list_delete2(sub_types, __release_ffi_type_simple);
    return NULL;
}
hffi_struct* hffi_new_struct(hffi_smtype** member_types, char** msg){
    return hffi_new_struct_abi0(FFI_DEFAULT_ABI, member_types, -1, msg);
}
hffi_struct* hffi_new_struct_abi(int abi, hffi_smtype** member_types, char** msg){
    return hffi_new_struct_abi0(abi, member_types, -1, msg);
}
static inline hffi_struct* hffi_new_struct_abi0(int abi,hffi_smtype** member_types, sint16 parent_pos, char** msg){
    if(member_types == NULL){
        if(msg) strcpy(*msg, "member_types can't be null.");
        return NULL;
    }
    array_list* list = array_list_new(12, 0.75f);
    int count = 0;
    while(member_types[count] != NULL){
        array_list_add(list, member_types[count]);
        count++;
    }
    hffi_struct* result = hffi_new_struct_from_list0(abi, list, parent_pos, msg);
    array_list_delete(list, NULL, NULL);
    return result;
}
hffi_struct* hffi_struct_copy(hffi_struct* _hs){
    int count = _hs->count;
    //child structs
    array_list* children = array_list_new2(count / 2 < 4 ? 4 : count / 2);
    //sub_types(ffi_type*) which need manmul release
    array_list* sub_types = array_list_new_simple();
    //copy types
    sint8* hffi_types = MALLOC(sizeof (sint8) * count);
    memcpy(hffi_types, _hs->hffi_types, sizeof (sint8) * count);

    //struct data
    int type_size = sizeof(ffi_type) + sizeof (ffi_type *) * (count+1) + sizeof (size_t) * count;
    ffi_type *type;
    size_t * offsets = NULL;
    //raw_type, elements, offsets.
    type = (ffi_type *) MALLOC(type_size);
    type->size = _hs->type->size;
    type->alignment = _hs->type->alignment;
    type->type = FFI_TYPE_STRUCT;
    type->elements = (ffi_type**)(type + 1);

    ffi_type* tmp_type;
    hffi_struct* tmp_struct;
    harray* tmp_harr;
    struct_item* tmp_item;
    int src_idx = 0;
    for(int i = count -1 ; i >=0 ; i --){
        switch (_hs->hffi_types[i]) {
        case HFFI_TYPE_HARRAY:{
            tmp_item = array_list_get(_hs->children, src_idx);
            tmp_harr = harray_copy((harray*)tmp_item->ptr);
            tmp_type = __harray_to_ffi_type(tmp_harr, sub_types);
            array_list_add(children, __new_struct_item(i, HFFI_TYPE_HARRAY, _ITEM_COPY_DATA, tmp_harr));
            src_idx ++;
        }break;
        case HFFI_TYPE_HARRAY_PTR:{
            tmp_type = &ffi_type_pointer;
            tmp_item = array_list_get(_hs->children, src_idx);
            tmp_harr = harray_copy((harray*)tmp_item->ptr);
            array_list_add(children, __new_struct_item(i, HFFI_TYPE_HARRAY_PTR, _ITEM_SET_DATA_PTR, tmp_harr));
            src_idx ++;
        }break;
        case HFFI_TYPE_STRUCT:{
            tmp_item = array_list_get(_hs->children, src_idx);
            tmp_struct = hffi_struct_copy((hffi_struct*)tmp_item->ptr);
            tmp_type = tmp_struct->type;
            //if old need set ptr, here also.
            array_list_add(children, __new_struct_item(i, HFFI_TYPE_STRUCT, tmp_item->hffi_t2, tmp_struct));
            src_idx ++;
        }break;
        case HFFI_TYPE_STRUCT_PTR:{
            tmp_type = &ffi_type_pointer;
            tmp_item = array_list_get(_hs->children, src_idx);
            tmp_struct = hffi_struct_copy((hffi_struct*)tmp_item->ptr);
            array_list_add(children, __new_struct_item(i, HFFI_TYPE_STRUCT_PTR, _ITEM_SET_DATA_PTR, tmp_struct));
            tmp_type = tmp_struct->type;
            src_idx ++;
        }break;
        default:
            tmp_type = to_ffi_type(_hs->hffi_types[i], NULL);
            break;
        }
        type->elements[i] = tmp_type;
    }
    offsets = (size_t *) &type->elements[count+1];
    //copy offsets
    memcpy(offsets, (size_t *) &_hs->type->elements[count+1], sizeof (size_t) * count);

    //type, children, sub_types
    hffi_struct* ptr = MALLOC(sizeof(hffi_struct));
    ptr->hffi_types = hffi_types;
    ptr->type = type;
    ptr->count = _hs->count;
    ptr->data_size = _hs->data_size;

    ptr->children = children;
    ptr->ref = 1;
    ptr->parent_pos = _hs->parent_pos;
    ptr->sub_ffi_types = sub_types;
    //only need data when is parent struct.
    if(_hs->data && _hs->parent_pos < 0){
        ptr->data = MALLOC(_hs->data_size);
        memcpy(ptr->data, _hs->data,  _hs->data_size);
    }else{
        ptr->data = NULL;
    }
    //some data ptr need re-set
    __set_children_data(ptr);
    return ptr;
}
void hffi_delete_structs(hffi_struct** cs, int count){
    if(cs){
        for(int i = count - 1; i >=0 ; i--){
             if(cs[i]){
                 hffi_delete_struct(cs[i]);
             }
        }
        FREE(cs);
    }
}
void hffi_delete_struct(hffi_struct* val){
    int old = atomic_add(&val->ref, -1);
    if(old == 1){
        if(val->type){
            FREE(val->type);
        }
        //children
        if(val->sub_ffi_types != NULL){
            array_list_delete2(val->sub_ffi_types, __release_ffi_type_simple);
        }
        if(val->children != NULL){
            array_list_delete2(val->children, __release_struct_item);
        }
        // MALLOC data by self
        if(val->parent_pos < 0 && val->data){
            FREE(val->data);
        }
        FREE(val->hffi_types);
        //self
        FREE(val);
    }
}
void hffi_struct_ref(hffi_struct* c, int ref_count){
    atomic_add(&c->ref, ref_count);
}
void* hffi_struct_get_data(struct hffi_struct* c){
    return c->data;
}
int hffi_struct_get_data_size(struct hffi_struct* c){
    return c->data_size;
}

hffi_struct* hffi_struct_get_struct(hffi_struct* hs, int index){
    if(index >= hs->count) return NULL;
    int ffi_t = hs->hffi_types[index];
    hffi_struct* hstr;
    if(ffi_t == HFFI_TYPE_STRUCT || ffi_t == HFFI_TYPE_STRUCT_PTR){
        struct_item* item = array_list_find(hs->children, __struct_find_struct_harray, &index);
        if(item == NULL){
            return NULL;
        }
        hstr = item->ptr;
    }else{
        return NULL;
    }
    if(ffi_t == HFFI_TYPE_STRUCT){
        //need sync data.
        size_t * offsets = (size_t *) &hs->type->elements[hs->count+1];
        void* data_ptr = hs->data + offsets[index];
        memcpy(hstr->data, data_ptr, hstr->data_size);
    }
    return hstr;
}
struct harray* hffi_struct_get_harray(hffi_struct* hs, int index){
    if(index >= hs->count) return NULL;
    int ffi_t = hs->hffi_types[index];
    harray* hstr;
    if(ffi_t == HFFI_TYPE_HARRAY || ffi_t == HFFI_TYPE_HARRAY_PTR){
        struct_item* item = array_list_find(hs->children, __struct_find_struct_harray, &index);
        if(item == NULL){
            return NULL;
        }
        hstr = item->ptr;
    }else{
        return NULL;
    }
    if(ffi_t == HFFI_TYPE_HARRAY){
        //need sync data.
        size_t * offsets = (size_t *) &hs->type->elements[hs->count+1];
        void* data_ptr = hs->data + offsets[index];
        memcpy(hstr->data, data_ptr, hstr->data_size);
    }
    return hstr;
}

int hffi_struct_get_base(hffi_struct* hs, int index, void* ptr){

#define DEF__struct_get_base(ffi_t, type)\
    case ffi_t:{\
    *((type*)ptr) = ((type*)data_ptr)[0];\
    return HFFI_STATE_OK;\
}break;

    if(index >= hs->count) return HFFI_STATE_FAILED;
    int ffi_t = hs->hffi_types[index];
    size_t * offsets = (size_t *) &hs->type->elements[hs->count+1];
    void* data_ptr = hs->data + offsets[index];
    //TODO if is base pointer. don't know if is base simple str.
    DEF_HFFI_BASE_SWITCH(DEF__struct_get_base, ffi_t);
    return HFFI_STATE_FAILED;
}

int hffi_struct_get_base_for_simple_ptr(hffi_struct* hs, int index, void* ptr){
#define DEF__struct_get_base_ptr(ffi_t, type)\
case ffi_t:{\
     void* _ptr = ((void**)data_ptr)[0];\
     *((type*)ptr) = *((type*)_ptr);\
     return HFFI_STATE_OK;\
}break;

    if(index >= hs->count) return HFFI_STATE_FAILED;
    int ffi_t = hs->hffi_types[index];
    size_t * offsets = (size_t *) &hs->type->elements[hs->count+1];
    void* data_ptr = hs->data + offsets[index];
    //TODO test
    DEF_HFFI_BASE_SWITCH(DEF__struct_get_base_ptr, ffi_t);
    return HFFI_STATE_FAILED;
}
//type must be the real data type
harray* hffi_struct_get_as_array(hffi_struct* hs, int index, sint8 type,int rows, int cols, int continue_mem, int share_memory){
    if(index >= hs->count) return NULL;
    if(hs->hffi_types[index] != HFFI_TYPE_POINTER) return NULL;

    size_t * offsets = (size_t *) &hs->type->elements[hs->count+1];
    void* data_ptr = hs->data + offsets[index];

    return hffi_get_pointer_as_array_impl(type, data_ptr, rows, cols, continue_mem, share_memory);
}

// return hffi_get_pointer_as_array_impl(val->pointer_base_type, val->ptr, rows, cols, continue_mem, share_memory);

//--------------------------------------------
#define ADD_VAL_TO_MANAGER(vals, c)\
if(hm->vals == NULL){\
    hm->vals = linklist_create(v);\
}else{\
    hm->vals = linklist_insert_beginning(hm->values, v);\
}\
hm->c++;\
return hm->c;

hffi_manager* hffi_new_manager(){
    hffi_manager* ptr = MALLOC(sizeof (hffi_manager));
    memset(ptr, 0, sizeof (hffi_manager));
    return ptr;
}
void hffi_delete_manager(hffi_manager* ptr){
    RELEASE_LINK_LIST(ptr->values, hffi_delete_value((hffi_value*)old->data))
    RELEASE_LINK_LIST(ptr->structs, hffi_delete_struct((hffi_struct*)old->data))
    RELEASE_LINK_LIST(ptr->smtypes, hffi_delete_smtype((hffi_smtype*)old->data))
    RELEASE_LINK_LIST(ptr->hcifs, hffi_delete_cif((hffi_cif*)old->data))
    RELEASE_LINK_LIST(ptr->harrays, harray_delete((harray*)old->data))

    RELEASE_LINK_LIST(ptr->list, FREE(old->data));
    FREE(ptr);
}
int hffi_manager_add_value(hffi_manager* hm,hffi_value* v){
    ADD_VAL_TO_MANAGER(values, value_count)
}
int hffi_manager_add_smtype(hffi_manager* hm,hffi_smtype* v){
    ADD_VAL_TO_MANAGER(smtypes, smtype_count)
}
int hffi_manager_add_struct(hffi_manager* hm,hffi_struct* v){
   ADD_VAL_TO_MANAGER(structs, struct_count)
}
int hffi_manager_add_cif(hffi_manager* hm,hffi_cif* v){
   ADD_VAL_TO_MANAGER(hcifs, hcif_count)
}
int hffi_manager_add_harray(hffi_manager* hm,struct harray* v){
    ADD_VAL_TO_MANAGER(harrays, harray_count)
}
void* hffi_manager_alloc(hffi_manager* hm,int size){
    void* data = MALLOC(size);
    if(hm->list == NULL){
        hm->list = linklist_create(data);
    }else{
        hm->list = linklist_insert_beginning(hm->list, data);
    }
    return data;
}

//------------------------- closure impl ---------------------------------
hffi_closure* hffi_new_closure(void* func_ptr, void (*fun_proxy)(ffi_cif*,void* ret,void** args,void* ud),
                               hffi_value** val_params, hffi_value* val_return, void* ud, char** msg){
    hffi_get_pointer_count(in_count, val_params);
    hffi_closure* ptr = MALLOC(sizeof(hffi_closure));
    memset(ptr, 0, sizeof(hffi_closure));
    ptr->ref = 1;
    ptr->code = &func_ptr;
    ptr->val_params = val_params;
    ptr->val_return = val_return;
    //mark reference
    for(int i = 0 ; i < in_count ; i ++){
        atomic_add(&val_params[i]->ref, 1);
    }
    atomic_add(&val_return->ref, 1);

    //create closure, prepare
    ptr->closure = ffi_closure_alloc(sizeof(ffi_closure), &func_ptr);
    if(ptr->closure == NULL){
        hffi_delete_closure(ptr);
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
            argTypes[i] = hffi_value_get_rawtype(val_params[i], msg);
            if(argTypes[i] == NULL){
                hffi_delete_closure(ptr);
                return NULL;
            }
        }
    }
    //return type
    ffi_type *return_type = hffi_value_get_rawtype(val_return, msg);
    if(return_type == NULL){
        hffi_delete_closure(ptr);
        return NULL;
    }
    //prepare
    ffi_cif cif;
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, in_count, return_type, argTypes) == FFI_OK) {
        //concat closure with proxy.
        if (ffi_prep_closure_loc(ptr->closure, &cif, fun_proxy, ud, func_ptr) == FFI_OK) {
            return ptr; //now we can call func
        }else{
            hffi_delete_closure(ptr);
            if(msg){
                strcpy(*msg, "concat closure with function proxy failed!");
            }
        }
    }else{
        hffi_delete_closure(ptr);
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
        if(val->val_params){
            int n = 0;
            while(val->val_params[n] != NULL){
                hffi_delete_value(val->val_params[n]);
                n++;
            }
        }
        if(val->val_return){
            hffi_delete_value(val->val_return);
        }
        FREE(val);
    }
}
//------------------------------------
//abi: default is 'FFI_DEFAULT_ABI'
hffi_cif* hffi_new_cif(int abi,array_list* in_vals, hffi_value* out, char** msg){
    int in_count = array_list_size(in_vals);
    //add ref
    hffi_value* val;
    for(int i = 0 ; i < in_count ; i ++){
        val = array_list_get(in_vals, i);
        atomic_add(&val->ref, 1);
    }
    atomic_add(&out->ref, 1);
    //build args
    ffi_type ** argTypes = NULL;
    void **args = NULL;
    if(in_count > 0){
        argTypes = alloca(sizeof(ffi_type *) *in_count);
        args = MALLOC(sizeof(void *) *in_count);
        for(int i = 0 ; i < in_count ; i ++){
            val = array_list_get(in_vals, i);
            argTypes[i] = hffi_value_get_rawtype(val, msg);
            if(argTypes[i] == NULL){
                return NULL;
            }
            //cast param value
            args[i] = __get_data_ptr(val);
        }
    }
    //prepare cif
    ffi_type *return_type = hffi_value_get_rawtype(out, msg);
    if(return_type == NULL){
        return NULL;
    }
    ffi_cif* cif = MALLOC(sizeof (ffi_cif));

    ffi_status s = ffi_prep_cif(cif, abi, (unsigned int)in_count, return_type, argTypes);
    switch (s) {
    case FFI_OK:{
       // ffi_call(&cif, fn, __get_data_ptr(out), args);
        //prepare
        hffi_cif* hcif = MALLOC(sizeof (hffi_cif));
        hcif->cif = cif;
        hcif->args = args;
        hcif->in_vals = in_vals;
        hcif->out = out;
        hcif->ref = 1;
        return hcif;
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
    FREE(cif);
    //ref
    for(int i = 0 ; i < in_count ; i ++){
        val = array_list_get(in_vals, i);
        hffi_delete_value(val);
    }
    hffi_delete_value(out);
    FREE(args);
    return NULL;
}
void hffi_delete_cif(hffi_cif* hcif){
    if(atomic_add(&hcif->ref, -1) == 1){
        FREE(hcif->cif);
        FREE(hcif->args);
        hffi_value* val;
        int c = array_list_size(hcif->in_vals);
        for(int i = 0 ; i < c ; i ++){
            val = array_list_get(hcif->in_vals, i);
            hffi_delete_value(val);
        }
        hffi_delete_value(hcif->out);
        FREE(hcif);
    }
}
void hffi_cif_call(hffi_cif* hcif, void* fn){
    ffi_call(hcif->cif, fn, __get_data_ptr(hcif->out), hcif->args);
}
hffi_value* hffi_cif_get_result_value(hffi_cif* hcif){
    return hcif->out;
}
hffi_value* hffi_cif_get_param_value(hffi_cif* hcif, int index){
    int c = array_list_size(hcif->in_vals);
    if(index < 0){ //-1 -> size -1
        index = c + index;
    }
    if(c == 0 || index >= c){
        return NULL;
    }
    return array_list_get(hcif->in_vals, index);
}
int hffi_cif_get_param_count(hffi_cif* hcif){
    return array_list_size(hcif->in_vals);
}
