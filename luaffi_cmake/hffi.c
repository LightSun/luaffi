
#include <stdio.h>
#include "hffi.h"
#include "atomic.h"
#include "h_linklist.h"
#include "h_alloctor.h"
#include "h_list.h"
#include "h_array.h"
#include "h_string.h"
#include "h_float_bits.h"
#include "hffi_pri.h"

#define hffi_new_value_auto_x(ffi_t,type) \
hffi_value* hffi_new_value_##type(type val){\
    hffi_value* v = hffi_new_value(ffi_t, HFFI_TYPE_VOID, sizeof(type));\
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
          return HFFI_STATE_OK;\
       }else{\
          strcpy(*msg, "wrong value type.");\
       }\
    }\
    return HFFI_STATE_FAILED;\
}

#define DEF_NEW_VALUE_RAW(ffi_t, type)\
case ffi_t:{\
    hffi_value* hv = hffi_new_value(ffi_t, HFFI_TYPE_VOID, sizeof (type));\
    *((type*)hv->ptr) = *((type*)val_ptr);\
    /* printf("hffi_new_value_raw_type2: ptr = %d, val = %d\n", hv->ptr, *((type*)hv->ptr));*/\
    return hv;\
}break;

#define DEF_NEW_VALUE_PTR2(ffi_t, type)\
case ffi_t:{\
    hffi_value* hv = hffi_new_value(HFFI_TYPE_POINTER, ffi_t, sizeof (type*));\
    *((type*)hv->ptr) = *((type*)val_ptr);\
    return hv;\
}break;

static hffi_value* _val_void = NULL;

hffi_value* hffi_get_void_value(){
    if(_val_void == NULL){
        _val_void = hffi_new_value_raw_type(HFFI_TYPE_VOID);
        hffi_value_ref(_val_void, 1);//keep never release for void.
    }
    return _val_void;
}

typedef struct struct_item{
    int index;      //the index of struct
    sint8 hffi_t;
    sint8 hffi_t2; //if is pointer. may need second. or else indicate if copy data(1) or set data-ptr(0)
    void* ptr;     // harray* or hffi_struct*
}struct_item;

#define _ITEM_COPY_DATA 1    // need copy struct data
#define _ITEM_SET_DATA_PTR 0 // need set data ptr from parent

static int __struct_item_eq(struct_item* i1, struct_item* i2){
    if(i1->index != i2->index) return HFFI_STATE_FAILED;
    if(i1->hffi_t != i2->hffi_t) return HFFI_STATE_FAILED;
    if(i1->hffi_t == HFFI_TYPE_POINTER){
        if(i1->hffi_t2 != i2->hffi_t2) return HFFI_STATE_FAILED;
    }
    if(i1->hffi_t == HFFI_TYPE_HARRAY || i1->hffi_t == HFFI_TYPE_HARRAY_PTR){
        if(harray_eq((harray*)i1->ptr, (harray*)i2->ptr) != HFFI_STATE_OK){
            return HFFI_STATE_FAILED;
        }
    }
    if(i1->hffi_t == HFFI_TYPE_STRUCT || i1->hffi_t == HFFI_TYPE_STRUCT_PTR){
        if(hffi_struct_eq((hffi_struct*)i1->ptr, (hffi_struct*)i2->ptr) != HFFI_STATE_OK){
            return HFFI_STATE_FAILED;
        }
    }
    return HFFI_STATE_OK;
}

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
static void __struct_set_children_data(hffi_struct* p);

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
static harray* hffi_get_pointer_as_array_impl(sint8 ffi_t, void* _ptr, int rows, int cols,
                                              int continue_mem, int share_memory){
    DEF_HFFI_BASE_SWITCH(DEF_GET_AS_ARRAY_IMPL, ffi_t);
    return NULL;
}

//---------------------------------------------------

static inline void __release_ffi_type_simple(void* data){
     FREE(data);
}
static inline ffi_type* __harray_to_ffi_type(int abi, harray* arr, array_list* sub_types){
    ffi_type* type = NULL;
    int success = 1;
    int c = arr->ele_count;
    //
    switch (arr->hffi_t) {
    case HFFI_TYPE_STRUCT:{
        type = MALLOC(HFFI_STRUCT_TYPE_SIZE(c));
        type->type = FFI_TYPE_STRUCT;
        type->alignment = type->size = 0; //compute by get_struct_offset
        type->elements = (ffi_type**)(type + 1);
        type->elements[c] = NULL;       
        union harray_ele ele;
        for(int i = 0 ; i < c ; i ++){
            harray_geti(arr, i , &ele);
            type->elements[i] = ((hffi_struct*)ele._extra)->type;
        }
        size_t* offsets = HFFI_STRUCT_OFFSETS(type, c);
        if(ffi_get_struct_offsets(abi, type, offsets) != FFI_OK){
            success = 0;
        }
    }break;

    case HFFI_TYPE_HARRAY:{
        type = MALLOC(HFFI_STRUCT_TYPE_SIZE(c));
        type->type = FFI_TYPE_STRUCT;
        type->alignment = type->size = 0;
        type->elements = (ffi_type**)(type + 1);
        type->elements[c] = NULL;
        ffi_type* tmp_type;
        union harray_ele ele;
        for(int i = 0 ; i < c ; i ++){
            harray_geti(arr, i , &ele);
            tmp_type = __harray_to_ffi_type(abi, (harray*)ele._extra, sub_types);
            if(tmp_type == NULL){
                success = 0;
                break;
            }
            type->elements[i] = tmp_type;
        }
        if(success){
            size_t* offsets = HFFI_STRUCT_OFFSETS(type, c);
            if(ffi_get_struct_offsets(abi, type, offsets) != FFI_OK){
                success = 0;
            }
        }
    }break;

    case HFFI_TYPE_STRUCT_PTR:
    case HFFI_TYPE_HARRAY_PTR:{
        type = MALLOC(HFFI_STRUCT_TYPE_SIZE(c));
        type->type = FFI_TYPE_STRUCT;
        type->alignment = type->size = 0;
        type->elements = (ffi_type**)(type + 1);
        type->elements[c] = NULL;
        for(int i = 0 ; i < c ; i ++){
            type->elements[i] = &ffi_type_pointer;
        }
        size_t* offsets = HFFI_STRUCT_OFFSETS(type, c);
        if(ffi_get_struct_offsets(abi, type, offsets) != FFI_OK){
            success = 0;
        }
    }break;

    default:
    {
        //https://github.com/libffi/libffi/issues/394, not work(test on windows).
        ffi_type* sub_type = to_ffi_type(arr->hffi_t, NULL);
        type = MALLOC(HFFI_STRUCT_TYPE_SIZE(c));
        type->type = FFI_TYPE_STRUCT;
        type->alignment = type->size = 0;
        type->elements = (ffi_type**)(type + 1);
        type->elements[c] = NULL;
        for(int i = 0 ; i < c ; i ++){
            type->elements[i] = sub_type;
        }
        size_t* offsets = HFFI_STRUCT_OFFSETS(type, c);
        if(ffi_get_struct_offsets(abi, type, offsets) != FFI_OK){
            success = 0;
        }
    }break;
    }
    if(type){
        array_list_add(sub_types, type);
    }
    return success ? type : NULL;
}
//---------------------------------------------------------------
//------------------------- value ------------------------
hffi_value* hffi_new_value_ptr(sint8 hffi_t2){
    return hffi_new_value(FFI_TYPE_POINTER, hffi_t2, sizeof (void*));
}
hffi_value* hffi_new_value(sint8 hffi_t, sint8 hffi_t2, int size){
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    memset(val_ptr, 0, sizeof (hffi_value));
    val_ptr->base_ffi_type = hffi_t;
    val_ptr->pointer_base_type = hffi_t2;
    val_ptr->ref = 1;   
    if(size > 0){
        val_ptr->ptr = MALLOC(size);
        memset(val_ptr->ptr, 0, size);
    }else{
        val_ptr->ptr = NULL;
    }
    return val_ptr;
}
hffi_value* hffi_new_value_ptr_nodata(sint8 hffi_t2){
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    memset(val_ptr, 0, sizeof (hffi_value));
    val_ptr->ptr = NULL;
    //val_ptr->multi_level_ptr = 1;
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
    return hffi_new_value(ffi_t, HFFI_TYPE_VOID,
                          //sizeof (void*)
                          hffi_base_type_size(ffi_t)
                          );
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
hffi_value* hffi_new_value_closure(hffi_closure* c){
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    memset(val_ptr, 0, sizeof (hffi_value));
    val_ptr->ptr = c;
    val_ptr->base_ffi_type = HFFI_TYPE_POINTER;
    val_ptr->pointer_base_type = HFFI_TYPE_CLOSURE;
    val_ptr->ref = 1;
    atomic_add(&c->ref, 1);
    return val_ptr;
}
hffi_value* hffi_new_value_harray(struct harray* arr){
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    val_ptr->sub_types = array_list_new2(4);
    val_ptr->ffi_type = __harray_to_ffi_type(FFI_DEFAULT_ABI, arr, val_ptr->sub_types);
    val_ptr->ptr = arr;
    val_ptr->base_ffi_type = HFFI_TYPE_HARRAY;
    val_ptr->pointer_base_type = HFFI_TYPE_VOID;
    val_ptr->ref = 1;
    atomic_add(&arr->ref, 1);
    return val_ptr;
}
hffi_value* hffi_value_copy(hffi_value* val){
    if(val == _val_void){
        return _val_void;
    }
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
        val_ptr->ffi_type = __harray_to_ffi_type(FFI_DEFAULT_ABI, (harray*)val->ptr, val_ptr->sub_types);
    }break;
    case HFFI_TYPE_STRUCT:{
         val_ptr->ptr = hffi_struct_copy((hffi_struct*)val->ptr);
    }break;
    case HFFI_TYPE_POINTER:{
        if(val->pointer_base_type == HFFI_TYPE_HARRAY){
            val_ptr->ptr = harray_copy((harray*)val->ptr);
        }else if(val->pointer_base_type == HFFI_TYPE_STRUCT){
            val_ptr->ptr = hffi_struct_copy((hffi_struct*)val->ptr);
        }else if(val->pointer_base_type == HFFI_TYPE_CLOSURE){
            val_ptr->ptr = hffi_closure_copy((hffi_closure*)val->ptr);
        }else{
            if(val->ptr != NULL){
                val_ptr->ptr = MALLOC(sizeof (void*));
                memcpy(val_ptr->ptr, val->ptr, sizeof (void*));
            }
        }
    }break;
    default:
        if(val->ptr != NULL){
            val_ptr->ptr = MALLOC(sizeof (void*));
            memcpy(val_ptr->ptr, val->ptr, sizeof (void*));
        }
    }
    return val_ptr;
}
void hffi_delete_value(hffi_value* val){
    if(val == _val_void){//ignore void
        return;
    }
    int old = atomic_add(&val->ref, -1);
    if(old == 1){
       // printf("hffi_delete_value(will delete): addr = %p\n", val);
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
            }else if(val->pointer_base_type == HFFI_TYPE_CLOSURE){
                hffi_delete_closure((hffi_closure*)val->ptr);
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
int hffi_value_hasData(hffi_value* val){
    if(val->ptr == NULL){
        return 0;
    }
    hffi_struct* hs = hffi_value_get_struct(val);
    if(hs != NULL){
        return hs->data != NULL;
    }
    harray* arr = hffi_value_get_harray(val);
    if(arr != NULL){
        return arr->data != NULL;
    }
    return 1;
}
hffi_smtype* hffi_value_to_smtype(hffi_value* val){
#define DEF_VALUE_TO_SMTYPE_BASE_IMPL(ffi_t, type)\
case ffi_t:{\
    return hffi_new_smtype(ffi_t);\
}break;
    DEF_HFFI_BASE_SWITCH(DEF_VALUE_TO_SMTYPE_BASE_IMPL, val->base_ffi_type)
    switch (val->base_ffi_type) {
        case HFFI_TYPE_HARRAY:{
            harray* arr = val->ptr;
            harray_ref(arr, 1);
            return hffi_new_smtype_harray(arr);
        }break;
        case HFFI_TYPE_STRUCT:{
            hffi_struct* c = val->ptr;
            hffi_struct_ref(c, 1);
            return hffi_new_smtype_struct(c);
        }break;

        case HFFI_TYPE_POINTER:{
            if(val->pointer_base_type == HFFI_TYPE_STRUCT){
                hffi_struct* c = val->ptr;
                hffi_struct_ref(c, 1);
                return hffi_new_smtype_struct_ptr(c);
            }else if(val->pointer_base_type == HFFI_TYPE_HARRAY){
                harray* arr = val->ptr;
                harray_ref(arr, 1);
                return hffi_new_smtype_harray_ptr(arr);
            }else if(val->pointer_base_type == HFFI_TYPE_CLOSURE){
                hffi_closure* clo = (hffi_closure*)val->ptr;
                hffi_closure_ref(clo, 1);
                return hffi_new_smtype_closure(clo);
            }else{
                return hffi_new_smtype(HFFI_TYPE_POINTER);
            }
        }break;
    }
    return NULL;
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
int hffi_value_set_any(hffi_value* val, void* val_ptr){
#define DEF_hffi_value_set_any_impl(ffi_t, type)\
case ffi_t:{\
    *((type*)val->ptr) = *((type*)val_ptr);\
    return HFFI_STATE_OK;\
}break;

#define DEF_hffi_value_set_any_impl2(ffi_t, type)\
case ffi_t:{\
    *((type*)val->ptr) = **((type**)val_ptr);\
    return HFFI_STATE_OK;\
}break;

    int ffi_t = val->base_ffi_type;
    DEF_HFFI_BASE_SWITCH(DEF_hffi_value_set_any_impl, ffi_t)
    DEF_HFFI_BASE_SWITCH(DEF_hffi_value_set_any_impl2, val->pointer_base_type)
    if(ffi_t == HFFI_TYPE_STRUCT){
       hffi_struct* hstru = hffi_value_get_struct(val);
       if(hstru != NULL){
           hffi_struct_set_all(hstru, *((void**)val_ptr));
           return HFFI_STATE_OK;
       }
    }
    if(ffi_t == HFFI_TYPE_HARRAY){
        harray* hstru = hffi_value_get_harray(val);
        if(hstru != NULL){
            harray_set_all(hstru, *(void**)val_ptr);
            return HFFI_STATE_OK;
        }
    }
    if(ffi_t == HFFI_TYPE_POINTER){
        if(val->pointer_base_type == HFFI_TYPE_HARRAY){
            harray* hstru = hffi_value_get_harray(val);
            if(hstru != NULL){
                harray_set_all(hstru, ((void**)val_ptr)[0]);
                return HFFI_STATE_OK;
            }
        }else if(val->pointer_base_type == HFFI_TYPE_STRUCT){
            hffi_struct* hstru = hffi_value_get_struct(val);
            if(hstru != NULL){
                hffi_struct_set_all(hstru, ((void**)val_ptr)[0]);
                return HFFI_STATE_OK;
            }
        }
    }
    return HFFI_STATE_FAILED;
}

ffi_type* hffi_value_get_rawtype(hffi_value* val, char** msg){
    if(val->base_ffi_type == HFFI_TYPE_STRUCT){
        return ((hffi_struct*)(val->ptr))->type;
    }
    if(val->base_ffi_type == HFFI_TYPE_HARRAY){
        if(!val->ffi_type){
            if(!val->sub_types) val->sub_types = array_list_new2(4);
            val->ffi_type = __harray_to_ffi_type(FFI_DEFAULT_ABI,val->ptr, val->sub_types);
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
int hffi_value_eq(hffi_value* val, hffi_value* val2){
    if(val == val2){
        return HFFI_TRUE;
    }
    if(val->base_ffi_type != val2->base_ffi_type || val->pointer_base_type != val2->pointer_base_type){
        return 1;
    }
    if(val->ptr == NULL){
        return val2->ptr == NULL ? HFFI_TRUE :HFFI_FALSE;
    }else{
        if(val2->ptr == NULL){
            return HFFI_FALSE;
        }
    }
#define DEF_hffi_value_eq_base(ffi_t, type)\
case ffi_t:{\
    if(*((type*)val->ptr) == *((type*)val2->ptr)){\
        return HFFI_TRUE;\
    }\
    return HFFI_FALSE;\
}break;

    int ffi_t = val->base_ffi_type;
    if(ffi_t == HFFI_TYPE_POINTER){
        ffi_t = val->pointer_base_type;
    }
    DEF_HFFI_BASE_SWITCH_INT(DEF_hffi_value_eq_base, ffi_t)
    if(ffi_t == HFFI_TYPE_FLOAT){
        return H_FLOAT_EQ(*((float*)val->ptr), *((float*)val2->ptr));
    }else if(ffi_t == HFFI_TYPE_FLOAT){
        return H_DOUBLE_EQ(*((double*)val->ptr), *((double*)val2->ptr));
    }

    switch (val->base_ffi_type) {
        case HFFI_TYPE_HARRAY:
        case HFFI_TYPE_HARRAY_PTR:{
            return harray_eq(hffi_value_get_harray(val), hffi_value_get_harray(val2));
        }break;

        case HFFI_TYPE_STRUCT_PTR:
        case HFFI_TYPE_STRUCT:{
            return hffi_struct_eq(hffi_value_get_struct(val), hffi_value_get_struct(val2));
        }break;

        case HFFI_TYPE_POINTER:{
            if(val->pointer_base_type == HFFI_TYPE_HARRAY){
                return harray_eq(hffi_value_get_harray(val), hffi_value_get_harray(val2));
            }else if(val->pointer_base_type == HFFI_TYPE_STRUCT){
                return hffi_struct_eq(hffi_value_get_struct(val), hffi_value_get_struct(val2));
            }
        }break;
    }
    return HFFI_FALSE;
}
void hffi_value_dump(hffi_value* val, struct hstring* hs){
    if(val->ptr == NULL){
        return;
    }
#define DEF_hffi_value_dump_IMPL(ffi_t, type, format)\
case ffi_t:{\
    hstring_appendf(hs, format, *((type*)val->ptr));\
    /*printf("hffi_value_dump: %d\n", *((type*)val->ptr));*/\
    return;\
}break;
    int ffi_t = val->base_ffi_type;
    if(ffi_t == HFFI_TYPE_POINTER){
        ffi_t = val->pointer_base_type;
    }
    DEF_HFFI_SWITCH_BASE_FORMAT(DEF_hffi_value_dump_IMPL, ffi_t);
    switch (val->base_ffi_type) {
        case HFFI_TYPE_HARRAY:{
            hstring_append(hs, "<array> ");
            harray* harr = hffi_value_get_harray(val);
            harray_dump(harr, hs);
        }break;
        case HFFI_TYPE_HARRAY_PTR:{
            hstring_append(hs, "<array_ptr> ");
            harray* harr = hffi_value_get_harray(val);
            harray_dump(harr, hs);
        }break;
        case HFFI_TYPE_STRUCT:{
            hstring_append(hs, "<struct> ");
            hffi_struct* child_hs = hffi_value_get_struct(val);
            hffi_struct_dump(child_hs, hs);
        }break;
        case HFFI_TYPE_STRUCT_PTR:{
            hstring_append(hs, "<struct_ptr> ");
            hffi_struct* child_hs = hffi_value_get_struct(val);
            hffi_struct_dump(child_hs, hs);
        }break;

        case HFFI_TYPE_POINTER:{
            if(val->pointer_base_type == HFFI_TYPE_HARRAY){
                hstring_append(hs, "<array_ptr> ");
                harray* harr = hffi_value_get_harray(val);
                harray_dump(harr, hs);
            }else if(val->pointer_base_type == HFFI_TYPE_STRUCT){
                hstring_append(hs, "<struct_ptr> ");
                hffi_struct* child_hs = hffi_value_get_struct(val);
                hffi_struct_dump(child_hs, hs);
            }else{
                hstring_append(hs, "<pointer> ");
                hstring_appendf(hs, "%p", val->ptr);
            }
        }break;
        default:{
            hstring_append(hs, "<unknown> ");
        }break;
    }
}

void list_travel_value_dump(void* d, struct hstring* hs){
    if(d) hffi_value_dump((hffi_value*)d, hs);
    else hstring_append(hs, "null");
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
        }else if(v->pointer_base_type == HFFI_TYPE_CLOSURE){
            return &((hffi_closure*)(v->ptr))->func_ptr;
        }
        else{
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

int hffi_call(void (*fn)(void), hffi_value** in, int var_count,hffi_value* out, char** msg){
    return hffi_call_abi(FFI_DEFAULT_ABI, fn, in, var_count, out, msg);
}

int hffi_call_abi(int abi,void (*fn)(void), hffi_value** in,int var_count,hffi_value* out, char** msg){
    hffi_get_pointer_count(in_count, in);
    array_list* list = array_list_new_max(in_count);
    for(int i = 0 ; i < in_count ; i ++){
        array_list_add(list, in[i]);
    }
    int result = hffi_call_from_list(abi, fn, list, var_count, out, msg);
    array_list_delete2(list, NULL);
    return result;
}

int hffi_call_from_list(int abi, void (*fn)(void), struct array_list* in, int var_count, hffi_value* out, char** msg){
    int in_count = array_list_size(in);
    if(var_count > in_count){
        return HFFI_STATE_FAILED;
    }
    //param types with values
    ffi_type ** argTypes = NULL;
    void **args = NULL;
    if(in_count > 0){
        argTypes = alloca(sizeof(ffi_type *) *in_count);
        args = alloca(sizeof(void *) *in_count);
        for(int i = 0 ; i < in_count ; i ++){
            argTypes[i] = hffi_value_get_rawtype((hffi_value*)array_list_get(in, i), msg);
            if(argTypes[i] == NULL){
                return HFFI_STATE_FAILED;
            }
            //cast param value
            args[i] = __get_data_ptr((hffi_value*)array_list_get(in, i));
        }
    }
    //prepare call
    ffi_cif cif;
    ffi_type *return_type = hffi_value_get_rawtype(out, msg);
    if(return_type == NULL){
        return HFFI_STATE_FAILED;
    }
    ffi_status s;
    if(var_count > 0){
        s = ffi_prep_cif_var(&cif, abi, in_count - var_count,(unsigned int)in_count, return_type, argTypes);
    }else{
        s = ffi_prep_cif(&cif, abi, (unsigned int)in_count, return_type, argTypes);
    }
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
    ptr->hffi_type = ffi_type;
    ptr->ref = 1;
    ptr->_ptr = NULL;
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
    ptr->hffi_type = HFFI_TYPE_STRUCT;
    ptr->ref = 1;
    ptr->_ptr = _struct;
    ptr->elements = NULL;
    atomic_add(&_struct->ref, 1);
    return ptr;
}
hffi_smtype* hffi_new_smtype_struct_ptr(hffi_struct* _struct){
    hffi_smtype* ptr = MALLOC(sizeof (hffi_smtype));
    ptr->hffi_type = HFFI_TYPE_STRUCT_PTR;
    ptr->ref = 1;
    ptr->_ptr = _struct;
    ptr->elements = NULL;
    atomic_add(&_struct->ref, 1);
    return ptr;
}
hffi_smtype* hffi_new_smtype_harray(struct harray* array){
    hffi_smtype* ptr = MALLOC(sizeof (hffi_smtype));
    ptr->hffi_type = HFFI_TYPE_HARRAY;
    ptr->ref = 1;
    ptr->_ptr = array;
    ptr->elements = NULL;
    atomic_add(&array->ref, 1);
    return ptr;
}
hffi_smtype* hffi_new_smtype_harray_ptr(struct harray* array){
    hffi_smtype* ptr = MALLOC(sizeof (hffi_smtype));
    ptr->hffi_type = HFFI_TYPE_HARRAY_PTR;
    ptr->ref = 1;
    ptr->_ptr = array;
    ptr->elements = NULL;
    atomic_add(&array->ref, 1);
    return ptr;
}
hffi_smtype* hffi_new_smtype_closure(hffi_closure* closure){
    hffi_smtype* ptr = MALLOC(sizeof (hffi_smtype));
    ptr->hffi_type = HFFI_TYPE_CLOSURE;
    ptr->ref = 1;
    ptr->_ptr = closure;
    ptr->elements = NULL;
    atomic_add(&closure->ref, 1);
    return ptr;
}
void list_travel_smtype_delete(void* d){
    if(d){
        hffi_delete_smtype((hffi_smtype*)d);
    }
}
void list_travel_struct_delete(void* d){
    if(d){
        hffi_delete_struct((hffi_struct*)d);
    }
}
void list_travel_value_delete(void* d){
    if(d){
        hffi_delete_value((hffi_value*)d);
    }
}
void list_travel_hcif_delete(void* d){
    if(d) hffi_delete_cif((hffi_cif*)d);
}

void hffi_delete_smtype(hffi_smtype* type){
    int old = atomic_add(&type->ref, -1);
    if(old == 1){
        if(type->elements != NULL){
            array_list_delete2(type->elements, list_travel_smtype_delete);
        }
        if(type->_ptr != NULL){
            switch (type->hffi_type) {
            case HFFI_TYPE_STRUCT:
            case HFFI_TYPE_STRUCT_PTR:{
                 hffi_delete_struct((hffi_struct*)type->_ptr);
            }break;
            case HFFI_TYPE_HARRAY:
            case HFFI_TYPE_HARRAY_PTR:{
                 harray_delete((harray*)type->_ptr);
            }break;

            case HFFI_TYPE_CLOSURE:
                hffi_delete_closure((hffi_closure*)type->_ptr);
                break;
            }
        }
        FREE(type);
    }
}
void hffi_smtype_ref(hffi_smtype* src, int c){
    atomic_add(&src->ref, c);
}
hffi_smtype* hffi_smtype_cpoy(hffi_smtype* src){
    hffi_smtype* ptr = MALLOC(sizeof (hffi_smtype));
    ptr->hffi_type = src->hffi_type;
    ptr->ref = 1;

    if(src->_ptr != NULL){
        switch (src->hffi_type) {
        case HFFI_TYPE_STRUCT:
        case HFFI_TYPE_STRUCT_PTR:{
             ptr->_ptr = hffi_struct_copy((hffi_struct*)src->_ptr);
        }break;
        case HFFI_TYPE_HARRAY:
        case HFFI_TYPE_HARRAY_PTR:{
             ptr->_ptr = harray_copy((harray*)src->_ptr);
        }break;

        case HFFI_TYPE_CLOSURE:
            ptr->_ptr = hffi_closure_copy((hffi_closure*)src->_ptr);
            break;
        default:
            printf("unsupport hffi_type: %d", src->hffi_type);
            abort();
        }
    }else{
        ptr->_ptr = NULL;
    }

    if(src->elements){
        int c = array_list_size(src->elements);
        ptr->elements = array_list_new_max(c);
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
static void  __set_children_data_travel(hffi_struct* p,struct_item* item){
    size_t * offsets = HFFI_STRUCT_OFFSETS(p->type, p->count);
    void* target_ptr = (p->data + offsets[item->index]);

    switch (item->hffi_t) {
    case HFFI_TYPE_STRUCT_PTR:{
        hffi_struct* stu = ((hffi_struct*)item->ptr);
        ((void**)target_ptr)[0] = stu->data;
        //memcpy(target_ptr, &stu->data, sizeof (void*));
    }break;
    case HFFI_TYPE_HARRAY_PTR:{
        harray* stu = ((harray*)item->ptr);
        ((void**)target_ptr)[0] = stu->data;
        //memcpy(target_ptr, &stu->data, sizeof (void*));
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
            __struct_set_children_data(stu);
        }
    }break;

    case HFFI_TYPE_CLOSURE:{
        ((void**)target_ptr)[0] = ((hffi_closure*)item->ptr)->func_ptr;
    }break;

    }
}
//just set ptrs. no need set 'HFFI_TYPE_HARRAY' and 'HFFI_TYPE_STRUCT'.

static inline void __struct_set_children_data(hffi_struct* p){
    if(p->data){
        int c = array_list_size(p->children);
        for(int i = 0 ; i < c ; i ++){
            __set_children_data_travel(p, (struct_item*)array_list_get(p->children, i));
        }
    }
}

//------------------------
static hffi_struct* hffi_new_struct_from_list0(int abi,struct array_list* list, sint16 parent_pos, char** msg);
static hffi_struct* hffi_new_struct_abi0(int abi,hffi_smtype** member_types, sint16 parent_pos, char** msg);
//------------------------
hffi_struct* hffi_new_struct_base(sint8* types, int count){
    return hffi_new_struct_base_abi(FFI_DEFAULT_ABI, types, count);
}
hffi_struct* hffi_new_struct_base_abi(int abi,sint8* types, int count){
    hffi_smtype* smtypes[count +1];
    for(int i = 0 ; i < count; i ++){
        smtypes[i] = hffi_new_smtype(types[i]);
    }
    smtypes[count] = NULL;
    hffi_struct* c = hffi_new_struct_abi0(abi, smtypes, HFFI_STRUCT_NO_PARENT, NULL);
    for(int i = 0 ; i < count; i ++){
        hffi_delete_smtype(smtypes[i]);
    }
    return c;
}
hffi_struct* hffi_new_struct_from_list2(int abi,struct array_list* list, char** msg){
    return hffi_new_struct_from_list0(abi, list, HFFI_STRUCT_NO_PARENT, msg);
}
hffi_struct* hffi_new_struct_from_list_nodata(int abi,struct array_list* list, char** msg){
    return hffi_new_struct_from_list0(abi, list, HFFI_STRUCT_NO_DATA, msg);
}
hffi_struct* hffi_new_struct_from_list(struct array_list* list, char** msg){
    return hffi_new_struct_from_list0(FFI_DEFAULT_ABI, list, HFFI_STRUCT_NO_PARENT, msg);
}
static inline hffi_struct* hffi_new_struct_from_list0(int abi,struct array_list* list, sint16 parent_pos, char** msg){
    int count = array_list_size(list);
    //create a struct
    hffi_struct* ptr = MALLOC(sizeof(hffi_struct));
    //child structs
    array_list* children = array_list_new2(count / 2 < 4 ? 4 : count / 2);
    //sub_types(ffi_type*) which need manmul release
    array_list* sub_types = array_list_new_simple();
    //record types for latter use.
    sint8* hffi_types = MALLOC(sizeof (sint8) * count);

    //struct data
    ffi_type *type;
    size_t * offsets = NULL;
    //raw_type, elements, offsets.
    type = (ffi_type *) MALLOC(HFFI_STRUCT_TYPE_SIZE(count));
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
        hffi_types[i] = tmp_smtype->hffi_type;
        switch (tmp_smtype->hffi_type) {
        case HFFI_TYPE_STRUCT:{
            if(tmp_smtype->_ptr != NULL){
                //is already a struct
                tmp_struct = (hffi_struct*)tmp_smtype->_ptr;
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
            HFFI_SET_PARENT(i, tmp_struct, ptr, HFFI_TYPE_STRUCT);
        }break;

        case HFFI_TYPE_HARRAY:{
            if(tmp_smtype->_ptr != NULL){
                tmp_harr = tmp_smtype->_ptr;
                tmp_type = __harray_to_ffi_type(abi, tmp_harr, sub_types);
                if(tmp_type == NULL){
                    goto failed;
                }
                atomic_add(&tmp_harr->ref, 1);
                array_list_add(children, __new_struct_item(i, HFFI_TYPE_HARRAY, _ITEM_COPY_DATA, tmp_harr));
            }else{
                //harray must be set first.
                goto failed;
            }
            HFFI_SET_PARENT(i, tmp_harr, ptr, HFFI_TYPE_STRUCT);
        }break;

        case HFFI_TYPE_STRUCT_PTR:{
            tmp_type = &ffi_type_pointer;
            tmp_struct = (hffi_struct*)tmp_smtype->_ptr;
            atomic_add(&tmp_struct->ref, 1);
            array_list_add(children, __new_struct_item(i, HFFI_TYPE_STRUCT_PTR, _ITEM_SET_DATA_PTR, tmp_struct));
        }break;

        case HFFI_TYPE_HARRAY_PTR:{
            tmp_type = &ffi_type_pointer;
            tmp_harr = tmp_smtype->_ptr;
            atomic_add(&tmp_harr->ref, 1);
            array_list_add(children, __new_struct_item(i, HFFI_TYPE_HARRAY_PTR, _ITEM_SET_DATA_PTR, tmp_harr));
        }break;

        case HFFI_TYPE_POINTER:{
            tmp_type = &ffi_type_pointer;
        }break;

        case HFFI_TYPE_CLOSURE:{
            tmp_type = &ffi_type_pointer;
            hffi_closure* closure = tmp_smtype->_ptr;
            hffi_closure_ref(closure, 1);
            array_list_add(children, __new_struct_item(i, HFFI_TYPE_CLOSURE, _ITEM_SET_DATA_PTR, closure));
        }break;

        default:{
            tmp_type = to_ffi_type(tmp_smtype->hffi_type, msg);
            if(tmp_type == NULL){
                goto failed;
            }
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
    //after call 'ffi_get_struct_offsets' , size now is ensured
    int total_size = type->size;

    //create hffi_struct to manage struct.
    //hffi_struct* ptr = MALLOC(sizeof(hffi_struct));
    ptr->parent = NULL;
    ptr->hffi_types = hffi_types;
    ptr->type = type;
    ptr->count = count;
    ptr->data_size = total_size;
    //of create by parent struct . never need MALLOC memory.
    if(parent_pos == HFFI_STRUCT_NO_PARENT){
        ptr->data = MALLOC(total_size);
        memset(ptr->data, 0, total_size);
    }else{
        ptr->data = NULL;
    }
    ptr->children = children;
    ptr->ref = 1;
    //for HFFI_STRUCT_NO_DATA. the data will be malloc by other.
    ptr->parent_pos = parent_pos;
    ptr->sub_ffi_types = sub_types;
    ptr->abi = (sint8)abi;
    //handle sub structs' data-pointer.
    __struct_set_children_data(ptr);
    return ptr;

failed:
    FREE(hffi_types);
    FREE(type);
    array_list_delete2(children, __release_struct_item);
    array_list_delete2(sub_types, __release_ffi_type_simple);
    FREE(ptr);
    return NULL;
}
hffi_struct* hffi_new_struct(hffi_smtype** member_types, char** msg){
    return hffi_new_struct_abi0(FFI_DEFAULT_ABI, member_types, HFFI_STRUCT_NO_PARENT, msg);
}
hffi_struct* hffi_new_struct_abi(int abi, hffi_smtype** member_types, char** msg){
    return hffi_new_struct_abi0(abi, member_types, HFFI_STRUCT_NO_PARENT, msg);
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
    //
    hffi_struct* ptr = MALLOC(sizeof(hffi_struct));
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
            tmp_type = __harray_to_ffi_type(_hs->abi, tmp_harr, sub_types);
            array_list_add(children, __new_struct_item(i, HFFI_TYPE_HARRAY, _ITEM_COPY_DATA, tmp_harr));
            src_idx ++;
            HFFI_SET_PARENT(i, tmp_harr, ptr, HFFI_TYPE_STRUCT)
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
            HFFI_SET_PARENT(i, tmp_struct, ptr, HFFI_TYPE_STRUCT)
        }break;
        case HFFI_TYPE_STRUCT_PTR:{
            tmp_type = &ffi_type_pointer;
            tmp_item = array_list_get(_hs->children, src_idx);
            tmp_struct = hffi_struct_copy((hffi_struct*)tmp_item->ptr);
            array_list_add(children, __new_struct_item(i, HFFI_TYPE_STRUCT_PTR, _ITEM_SET_DATA_PTR, tmp_struct));
            src_idx ++;
        }break;

        case HFFI_TYPE_CLOSURE:{
            tmp_type = &ffi_type_pointer;
            tmp_item = array_list_get(_hs->children, src_idx);
            hffi_closure* closure = hffi_closure_copy((hffi_closure*)tmp_item->ptr);
            array_list_add(children, __new_struct_item(i, HFFI_TYPE_CLOSURE, _ITEM_SET_DATA_PTR, closure));
            src_idx ++;
        }break;
        default:
            tmp_type = to_ffi_type(_hs->hffi_types[i], NULL);
            break;
        }
        type->elements[i] = tmp_type;
    }
    //copy offsets
    memcpy(HFFI_STRUCT_OFFSETS(type, count), HFFI_STRUCT_OFFSETS(_hs->type, count), sizeof (size_t) * count);

    //type, children, sub_types
   // hffi_struct* ptr = MALLOC(sizeof(hffi_struct));
    ptr->parent = NULL;
    ptr->hffi_types = hffi_types;
    ptr->type = type;
    ptr->count = _hs->count;
    ptr->data_size = _hs->data_size;

    ptr->children = children;
    ptr->ref = 1;
    ptr->abi = _hs->abi;
    ptr->parent_pos = _hs->parent_pos;
    ptr->sub_ffi_types = sub_types;
    //only need data when is parent struct.
    if(_hs->data && _hs->parent_pos < 0){
        ptr->data = MALLOC(_hs->data_size);
        memcpy(ptr->data, _hs->data,  _hs->data_size);
        //some data ptr need re-set
        hffi_struct_set_all(ptr, ptr->data);
    }else{
        ptr->data = NULL;
    }
    return ptr;
}
int hffi_struct_eq(struct hffi_struct* hs1, struct hffi_struct* hs2){
    if(hs1 == hs2){
        return HFFI_TRUE;
    }
    if(hs1->data_size != hs2->data_size || hs1->count != hs2->count) {
        return HFFI_STATE_FAILED;
    }
    if(memcmp(hs1->hffi_types, hs2->hffi_types, sizeof (sint8) * hs1->count) != 0){
        return HFFI_STATE_FAILED;
    }
    size_t * offsets = (size_t *) &hs1->type->elements[hs1->count+1];
    size_t * offsets2 = (size_t *) &hs2->type->elements[hs2->count+1];
    if(memcmp(offsets, offsets2, sizeof (size_t) * hs1->count) != 0){
        return HFFI_STATE_FAILED;
    }
    if(array_list_size(hs1->children) != array_list_size(hs2->children)){
        return HFFI_STATE_FAILED;
    }
    if(hs1->data == NULL || hs2->data == NULL){
        int child_size = array_list_size(hs1->children);
        struct_item* item1; struct_item* item2;
        for(int i = 0 ; i < child_size ; i ++){
            item1 = array_list_get(hs1->children, i);
            item2 = array_list_get(hs2->children, i);
            if(__struct_item_eq(item1, item2) != HFFI_STATE_OK){
                return HFFI_STATE_FAILED;
            }
        }
        return HFFI_STATE_OK;
    }
#define DEF_STRUCT_DATA_EQ_CASE(ffi_t, type)\
case ffi_t:{\
    void* d1 = hs1->data + offsets[i];\
    void* d2 = hs2->data + offsets[i];\
    if(memcmp(d1, d2, sizeof (type)) != 0){\
        return HFFI_STATE_FAILED;\
    }\
}break;
    struct_item* item1;
    struct_item* item2;
    for(int i = 0 ; i < hs1->count ; i ++){
        DEF_HFFI_BASE_SWITCH(DEF_STRUCT_DATA_EQ_CASE, hs1->hffi_types[i])
        switch (hs1->hffi_types[i]) {
            case HFFI_TYPE_STRUCT:{
                item1 = array_list_find(hs1->children, __struct_find_struct_harray, &i);
                item2 = array_list_find(hs2->children, __struct_find_struct_harray, &i);
                if(((hffi_struct*)item1->ptr)->data_size != ((hffi_struct*)item2->ptr)->data_size){
                    return HFFI_STATE_FAILED;
                }
                void* d1 = hs1->data + offsets[i];
                void* d2 = hs2->data + offsets[i];
                if(memcmp(d1, d2, ((hffi_struct*)item1->ptr)->data_size) != 0){
                    return HFFI_STATE_FAILED;
                }
            }break;
            case HFFI_TYPE_HARRAY_PTR:
            case HFFI_TYPE_STRUCT_PTR:{
                item1 = array_list_find(hs1->children, __struct_find_struct_harray, &i);
                item2 = array_list_find(hs2->children, __struct_find_struct_harray, &i);
                if(__struct_item_eq(item1, item2) != HFFI_STATE_OK){
                    return HFFI_STATE_FAILED;
                }
            }break;
            case HFFI_TYPE_HARRAY:{
                item1 = array_list_find(hs1->children, __struct_find_struct_harray, &i);
                item2 = array_list_find(hs2->children, __struct_find_struct_harray, &i);
                if(((harray*)item1->ptr)->data_size != ((harray*)item2->ptr)->data_size){
                    return HFFI_STATE_FAILED;
                }
                void* d1 = hs1->data + offsets[i];
                void* d2 = hs2->data + offsets[i];
                if(memcmp(d1, d2, ((harray*)item1->ptr)->data_size) != 0){
                    return HFFI_STATE_FAILED;
                }
            }break;

            case HFFI_TYPE_POINTER:{
                void* d1 = hs1->data + offsets[i];
                void* d2 = hs2->data + offsets[i];
                if(memcmp(d1, d2, sizeof (void*)) != 0){
                    return HFFI_STATE_FAILED;
                }
            }break;
        }
    }
    return HFFI_STATE_OK;
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
        HFFI_FREE_PARENT(val)
        //self
        FREE(val);
    }
}
int hffi_struct_is_pointer(hffi_struct* hs, int index){
    return hs->hffi_types[index] == HFFI_TYPE_POINTER;
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
    if(ffi_t == HFFI_TYPE_STRUCT && hs->data){
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
    if(ffi_t == HFFI_TYPE_HARRAY && hs->data){
        //need sync data.
        size_t * offsets = (size_t *) &hs->type->elements[hs->count+1];
        void* data_ptr = hs->data + offsets[index];
        memcpy(hstr->data, data_ptr, hstr->data_size);
    }
    return hstr;
}

int hffi_struct_get_base(hffi_struct* hs, int index, sint8 target_hffi, void* ptr){

#define DEF__struct_get_base(ffi_t, type)\
case ffi_t:{\
    *((type*)ptr) = ((type*)data_ptr)[0];\
    printf("hffi_struct_get_base: val = %.2f\n", ((type*)data_ptr)[0]);\
    return HFFI_STATE_OK;\
}break;

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
    //TODO if is base pointer. don't know if is base simple str.
    if(ffi_t == HFFI_TYPE_POINTER){
        DEF_HFFI_BASE_SWITCH(DEF__struct_get_base_ptr, target_hffi);
    }else{
        DEF_HFFI_BASE_SWITCH(DEF__struct_get_base, ffi_t);
    }
    return HFFI_STATE_FAILED;
}
int hffi_struct_set_base(hffi_struct* hs, int index, sint8 target_hffi, void* ptr){
#define DEF__struct_set_base(ffi_t, type)\
case ffi_t:{\
    ((type*)data_ptr)[0] = *((type*)ptr);\
    return HFFI_STATE_OK;\
}break;

#define DEF__struct_set_base_ptr(ffi_t, type)\
case ffi_t:{\
     void* _ptr = ((void**)data_ptr)[0];\
     *((type*)_ptr) = *((type*)ptr);\
     return HFFI_STATE_OK;\
}break;
    if(index >= hs->count) return HFFI_STATE_FAILED;
    int ffi_t = hs->hffi_types[index];

    size_t * offsets = (size_t *) &hs->type->elements[hs->count+1];
    void* data_ptr = hs->data + offsets[index];
    //
    if(ffi_t == HFFI_TYPE_POINTER){
        DEF_HFFI_BASE_SWITCH(DEF__struct_set_base_ptr, target_hffi);
    }else{
        DEF_HFFI_BASE_SWITCH(DEF__struct_set_base, ffi_t);
    }
    return HFFI_STATE_FAILED;
}

//type must be the real data type
harray* hffi_struct_get_as_array(hffi_struct* hs, int index, sint8 type,int rows, int cols,
                                 int continue_mem, int share_memory){
    if(index >= hs->count) return NULL;
    if(hs->hffi_types[index] != HFFI_TYPE_POINTER) return NULL;

    size_t * offsets = (size_t *) &hs->type->elements[hs->count+1];
    void* data_ptr = hs->data + offsets[index];

    return hffi_get_pointer_as_array_impl(type, data_ptr, rows, cols, continue_mem, share_memory);
}

int hffi_struct_set_all(struct hffi_struct* hs, void* ptr){
    if(!hs->data){
        hs->data = ptr;
    }else{
        memcpy(hs->data, ptr, hs->data_size);
    }
    //remove no data flag
    if(hs->parent_pos == HFFI_STRUCT_NO_DATA){
        hs->parent_pos = HFFI_STRUCT_NO_PARENT;
    }
    size_t * offsets = HFFI_STRUCT_OFFSETS(hs->type, hs->count);
    int count = array_list_size(hs->children);

    struct_item* tmp_item;
    void* data_ptr;
    for(int i = 0 ; i < count ; i ++){
        tmp_item = array_list_get(hs->children, i);
        data_ptr = hs->data + offsets[tmp_item->index];

        switch (tmp_item->hffi_t) {
        case HFFI_TYPE_STRUCT:{
             //not need set here?
            if(hffi_struct_set_all((hffi_struct*)tmp_item->ptr, data_ptr) != HFFI_STATE_OK){
                return HFFI_STATE_FAILED;
            }
        }break;
        case HFFI_TYPE_HARRAY:{
            //not need set here?
            if(harray_set_all((harray*)tmp_item->ptr, data_ptr) != HFFI_STATE_OK){
                return HFFI_STATE_FAILED;
            }
        }break;
        case HFFI_TYPE_STRUCT_PTR:{
            if(hffi_struct_set_all((hffi_struct*)tmp_item->ptr, ((void**)data_ptr)[0]) != HFFI_STATE_OK){
                return HFFI_STATE_FAILED;
            }
            ((void**)data_ptr)[0] = ((hffi_struct*)tmp_item->ptr)->data;
        }break;
        case HFFI_TYPE_HARRAY_PTR:{
            if(harray_set_all((harray*)tmp_item->ptr, ((void**)data_ptr)[0]) != HFFI_STATE_OK){
                return HFFI_STATE_FAILED;
            }
            //set ptr
            ((void**)data_ptr)[0] = ((harray*)tmp_item->ptr)->data;
        }break;

        case HFFI_TYPE_CLOSURE:{
            hffi_closure* closure = (hffi_closure*)tmp_item->ptr;
            if(hffi_closure_set_func_ptr(closure, ((void**)data_ptr)[0]) != HFFI_STATE_OK){
                return HFFI_STATE_FAILED;
            }
        }break;
        }
    }
    return HFFI_STATE_OK;
}

int hffi_struct_set_harray(hffi_struct* hs, int index, struct harray* arr){
    if(arr == NULL || index >= hs->count) return HFFI_STATE_FAILED;
    size_t * offsets = HFFI_STRUCT_OFFSETS(hs->type, hs->count);
    void* data_ptr = hs->data + offsets[index];

    int ffi_t = hs->hffi_types[index];
    if(ffi_t == HFFI_TYPE_HARRAY){
        //can't use aligned size to check data_size_match.
//        if((int)hs->type->elements[index]->size != arr->data_size){
//            return HFFI_STATE_FAILED;
//        }
        struct_item* item = array_list_find(hs->children, __struct_find_struct_harray, &index);
        if(item != NULL){
            harray* old_arr = item->ptr;
            if(old_arr->data_size != arr->data_size){
                return HFFI_STATE_FAILED;
            }
            memcpy(data_ptr, arr->data, arr->data_size);
            //clear old parent and set new
            HFFI_CLEAR_PARENT(old_arr)
            HFFI_SET_PARENT(index, arr, hs, HFFI_TYPE_STRUCT);
            harray_delete(old_arr);
            harray_ref(arr, 1);
            item->ptr = arr;
            //sync parent if need
            //HFFI_SYNC_PARENT_I(hs)
            return HFFI_STATE_OK;
        }
    }else if(ffi_t == HFFI_TYPE_HARRAY_PTR){
        struct_item* item = array_list_find(hs->children, __struct_find_struct_harray, &index);
        if(item != NULL){
            harray_ref(arr, 1);
            ((void**)data_ptr)[0] = arr;
            //unref-old ref new
            harray* old_arr = item->ptr;
            harray_delete(old_arr);
            item->ptr = arr;
            return HFFI_STATE_OK;
        }
    }
    return HFFI_STATE_FAILED;
}
int hffi_struct_set_struct(hffi_struct* hs, int index, hffi_struct* arr){
    if(arr == NULL || index >= hs->count) return HFFI_STATE_FAILED;
    size_t * offsets = HFFI_STRUCT_OFFSETS(hs->type, hs->count);
    void* data_ptr = hs->data + offsets[index];

    int ffi_t = hs->hffi_types[index];
    if(ffi_t == HFFI_TYPE_HARRAY){
       //can't use aligned size to check data_size_match.
//        if((int)hs->type->elements[index]->size != arr->data_size){
//            return HFFI_STATE_FAILED;
//        }
        struct_item* item = array_list_find(hs->children, __struct_find_struct_harray, &index);
        if(item != NULL){
            hffi_struct* old_arr = item->ptr;
            if(old_arr->data_size != arr->data_size){
                return HFFI_STATE_FAILED;
            }
            memcpy(data_ptr, arr->data, arr->data_size);
            HFFI_CLEAR_PARENT(old_arr)
            HFFI_SET_PARENT(index, arr, hs, HFFI_TYPE_STRUCT);
            hffi_delete_struct(old_arr);
            hffi_struct_ref(arr, 1);
            item->ptr = arr;

            //HFFI_SYNC_PARENT_I(hs)
            return HFFI_STATE_OK;
        }
    }else if(ffi_t == HFFI_TYPE_HARRAY_PTR){
        struct_item* item = array_list_find(hs->children, __struct_find_struct_harray, &index);
        if(item != NULL){
            ((void**)data_ptr)[0] = arr;
            hffi_struct_ref(arr, 1);
            //unref-old ref new
            hffi_struct* old_arr = item->ptr;
            hffi_delete_struct(old_arr);
            item->ptr = arr;
            return HFFI_STATE_OK;
        }
    }
    return HFFI_STATE_FAILED;
}

void hffi_struct_dump(hffi_struct* arr, struct hstring* hs){
    size_t* offsets = HFFI_STRUCT_OFFSETS(arr->type, arr->count);
    hstring_append(hs, "[ type_desc: ");
    hstring_appendf(hs, "size = %d", arr->type->size);
    hstring_appendf(hs, ", align = %d", arr->type->alignment);
    hstring_append(hs, ", offsets = (");
    for(int i = 0 ; i < arr->count ; i++){
        hstring_appendf(hs, "%d", offsets[i]);
        if(i != arr->count - 1){
            hstring_append(hs, ", ");
        }
    }
    hstring_append(hs, "), member_types = (");

#define __TYPE_STR(ffi_t, type)\
case ffi_t: hstring_append(hs, type); break;

    for(int i = 0 ; i < arr->count ; i++){
        DEF_HFFI_SWITCH_ALL(__TYPE_STR, arr->hffi_types[i])
        if(i != arr->count - 1){
            hstring_append(hs, ", ");
        }
    }
    hstring_append(hs, " )]\n");
    //dump data.
    if(!arr->data){
        hstring_append(hs, " data = NULL");
    }else{
        hstring_append(hs, " data = \n [");
#define __struct_dump_impl(hffi_t, type, format)\
case hffi_t:{\
    void* data_ptr = arr->data + offsets[i];\
    hstring_appendf(hs, format, ((type*)data_ptr)[0]);\
    if(i != arr->count - 1){\
        hstring_append(hs, " ,");\
    }\
}continue;

        for(int i = 0 ; i < arr->count ; i++){
            DEF_HFFI_SWITCH_BASE_FORMAT(__struct_dump_impl, arr->hffi_types[i])
            switch (arr->hffi_types[i]) {
                case HFFI_TYPE_HARRAY:{
                    hstring_append(hs, " \n<array> ");
                    harray* harr = hffi_struct_get_harray(arr, i);
                    harray_dump(harr, hs);
                }break;
                case HFFI_TYPE_HARRAY_PTR:{
                    hstring_append(hs, " \n<array_ptr> ");
                    harray* harr = hffi_struct_get_harray(arr, i);
                    harray_dump(harr, hs);
                }break;
                case HFFI_TYPE_STRUCT:{
                    hstring_append(hs, " \n<struct> ");
                    hffi_struct* child_hs = hffi_struct_get_struct(arr, i);
                    hffi_struct_dump(child_hs, hs);
                }break;
                case HFFI_TYPE_STRUCT_PTR:{
                    hstring_append(hs, " \n<struct_ptr> ");
                    hffi_struct* child_hs = hffi_struct_get_struct(arr, i);
                    hffi_struct_dump(child_hs, hs);
                }break;

                case HFFI_TYPE_POINTER:{
                    hstring_append(hs, " \n<pointer> ");
                    hstring_appendf(hs, "%p", arr->data + offsets[i]);
                }break;
                case HFFI_TYPE_CLOSURE:{
                    hstring_append(hs, " \n<closure> ");
                    hstring_appendf(hs, "%p", arr->data + offsets[i]);
                }break;
                default:{
                    hstring_append(hs, " \n<unknown> ");
                }break;
            }
            if(i != arr->count - 1){\
                hstring_append(hs, " ,");\
            }
        }
        hstring_append(hs, " ]");
    }
}

void hffi_struct_sync_data(struct hffi_struct* hs, int reverse){
    if(hs->data == NULL){
        return;
    }
    size_t * offsets = HFFI_STRUCT_OFFSETS(hs->type, hs->count);
   // void* data_ptr = hs->data + offsets[index];
    void* data_ptr;
    struct_item* item;
    for(int i = 0 ; i < hs->count ; i ++){
        switch (hs->hffi_types[i]) {
        case HFFI_TYPE_STRUCT:{
            item = array_list_find(hs->children, __struct_find_struct_harray, &i);
            if(item != NULL){
                data_ptr = hs->data + offsets[i];
                if(reverse){
                    hffi_struct_sync_data((hffi_struct*)item->ptr, reverse);
                    memcpy(data_ptr, ((hffi_struct*)item->ptr)->data, ((hffi_struct*)item->ptr)->data_size);
                }else{
                    memcpy(((hffi_struct*)item->ptr)->data, data_ptr, ((hffi_struct*)item->ptr)->data_size);
                    hffi_struct_sync_data((hffi_struct*)item->ptr, reverse);
                }
            }else{
                //can't reach here
            }
        }break;
        case HFFI_TYPE_HARRAY:{
            item = array_list_find(hs->children, __struct_find_struct_harray, &i);
            if(item != NULL){
                data_ptr = hs->data + offsets[i];
                if(reverse){
                    harray_sync_data((harray*)item->ptr, reverse);
                    memcpy(data_ptr, ((harray*)item->ptr)->data, ((harray*)item->ptr)->data_size);
                }else{
                    memcpy(((harray*)item->ptr)->data, data_ptr, ((harray*)item->ptr)->data_size);
                    harray_sync_data((harray*)item->ptr, reverse);
                }
            }else{
                //can't reach here
            }
        }break;
        }
    }
}
void hffi_struct_sync_data_i(struct hffi_struct* hs, int i, void* ptr){
    int should_sync_parent = 0;
    size_t * offsets = HFFI_STRUCT_OFFSETS(hs->type, hs->count);
    struct_item* item;
    void* data_ptr;
    switch (hs->hffi_types[i]) {
    case HFFI_TYPE_STRUCT:{
        item = array_list_find(hs->children, __struct_find_struct_harray, &i);
        if(item != NULL){
            assert(item->ptr == ptr);
            data_ptr = hs->data + offsets[i];
            memcpy(data_ptr, ((hffi_struct*)item->ptr)->data, ((hffi_struct*)item->ptr)->data_size);
            should_sync_parent = 1;
        }
    }break;
    case HFFI_TYPE_HARRAY:{
        item = array_list_find(hs->children, __struct_find_struct_harray, &i);
        if(item != NULL){
            assert(item->ptr == ptr);
            data_ptr = hs->data + offsets[i];
            memcpy(data_ptr, ((harray*)item->ptr)->data, ((harray*)item->ptr)->data_size);
            should_sync_parent = 1;
        }
    }break;
    }
    if(should_sync_parent) HFFI_SYNC_PARENT_I(hs);
}
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
static inline int __prepare_closure_cif(int abi,hffi_closure* ptr, void (*fun_proxy)(ffi_cif*,void* ret,void** args,void* ud),
                                        void* ud, char** msg){
    //param
    int in_count = array_list_size(ptr->in_vals);
    ffi_type ** argTypes = NULL;
    if(in_count > 0){
        argTypes = MALLOC(sizeof (ffi_type *) * (in_count + 1));
        argTypes[in_count] = NULL;
        for(int i = 0 ; i < in_count ; i ++){
            argTypes[i] = hffi_value_get_rawtype((hffi_value*)array_list_get(ptr->in_vals, i), msg);
            if(argTypes[i] == NULL){
                return HFFI_STATE_FAILED;
            }
        }
    }
    //return type
    ffi_type *return_type = hffi_value_get_rawtype(ptr->ret_val, msg);
    if(return_type == NULL){
        return HFFI_STATE_FAILED;
    }
    //prepare
    if (ffi_prep_cif(ptr->cif, abi, in_count, return_type, argTypes) == FFI_OK) {
        //concat closure with proxy.
        if (ffi_prep_closure_loc(ptr->closure, ptr->cif, fun_proxy, ud, ptr->func_ptr) == FFI_OK) {
            return HFFI_STATE_OK; //now we can call func
        }else{
            if(msg){
                strcpy(*msg, "concat closure with function proxy failed!");
            }
            return HFFI_STATE_FAILED;
        }
    }else{
        if(msg){
            strcpy(*msg, "ffi_prep_cif(...) failed!");
        }
        return HFFI_STATE_FAILED;
    }
}
#define __CLOSURE__DEREF(val)\
do{\
volatile int* ref_ptr = (void*)val->closure + sizeof(ffi_closure);\
if(atomic_add(ref_ptr, -1) == 1){\
    ffi_closure_free(val->closure);\
}\
}while(0);

hffi_closure* hffi_new_closure(int abi, void (*fun_proxy)(ffi_cif*,void* ret,void** args,void* ud),
                               struct array_list* in_vals, hffi_value* val_return, void* ud, char** msg){
#define DEF_CLOSURE_VALS_REF(c)\
{hffi_value* tmp_val;\
for(int i = 0 ; i < in_count ; i ++){\
    tmp_val = array_list_get(in_vals, i);\
    atomic_add(&tmp_val->ref, c);\
    array_list_add(input_vals, tmp_val);\
}\
atomic_add(&val_return->ref, c);}

    int in_count = array_list_size(in_vals);
    array_list* input_vals = array_list_new_max(in_count);
    //mark reference
    DEF_CLOSURE_VALS_REF(1);

    hffi_closure* ptr = MALLOC(sizeof(hffi_closure));
    memset(ptr, 0, sizeof(hffi_closure));
    ptr->cif = MALLOC(sizeof (ffi_cif) + sizeof (int));
    ptr->in_vals = input_vals;
    ptr->ret_val = val_return;

    ptr->ref = 1;
    //set ref for cif
    HFFI_TAIL_INT_PTR_SET(ptr->cif, sizeof (ffi_cif), 1);
    //create closure, prepare
    ptr->closure = ffi_closure_alloc(sizeof(ffi_closure) + sizeof (int), &ptr->func_ptr);
    if(ptr->closure == NULL){
        if(msg){
            strcpy(*msg, "create closure failed!");
        }
        goto failed;
    }
    HFFI_TAIL_INT_PTR_SET(ptr->closure, sizeof(ffi_closure), 1);

    if(__prepare_closure_cif(abi, ptr, fun_proxy, ud, msg) == HFFI_STATE_OK){
        return ptr; //now we can call func
    }
    failed:
    hffi_delete_closure(ptr);
    return NULL;
}

int hffi_delete_closure(hffi_closure* val){
    int old = atomic_add(&val->ref, -1);
    if(old == 1){
        //cif's lifecycle is same with closure
        if(val->closure != NULL){
            __CLOSURE__DEREF(val)
        }
        //cif with type is
        HFFI_TAIL_INT_PTR_ADD_X(val->cif, sizeof (ffi_cif), -1, {
            if(old == 1){
                FREE(val->cif->arg_types);
                FREE(val->cif);
            }     })
        if(val->in_vals){
            array_list_delete2(val->in_vals, list_travel_value_delete);
        }
        if(val->ret_val){
            hffi_delete_value(val->ret_val);
        }
        FREE(val);
    }
    return old - 1;
}
int hffi_closure_set_func_ptr(hffi_closure* ptr, void* func_ptr){
    void* new_closure = ffi_closure_alloc(sizeof(ffi_closure) + sizeof (int), &func_ptr);
    if(new_closure == NULL){
        return HFFI_STATE_FAILED;
    }
    HFFI_TAIL_INT_PTR_SET(new_closure, sizeof (ffi_closure), 1)
    //save some vars
    void* ud = ptr->closure->user_data;
    void* fun_proxy = ptr->closure->fun;

    //cif is alread prepared. only need closure.
    //check prepare closure
    if (ffi_prep_closure_loc(new_closure, ptr->cif, fun_proxy, ud, func_ptr) == FFI_OK) {
        ptr->func_ptr = func_ptr;
        //free old closure and set new
        __CLOSURE__DEREF(ptr);
        ptr->closure = new_closure;
        return HFFI_STATE_OK; //now we can call func
    }
    ffi_closure_free(new_closure);
    return HFFI_STATE_FAILED;
}
hffi_closure* hffi_closure_copy(hffi_closure* c){
    //add ref
    volatile int* ref_ptr = (void*)c->closure + sizeof(ffi_closure);
    atomic_add(ref_ptr, 1);
    //
    hffi_closure* ptr = MALLOC(sizeof(hffi_closure));
    ptr->ref = 1;
    ptr->in_vals = array_list_new_max(array_list_size(c->in_vals));
    ptr->ret_val = hffi_value_copy(c->ret_val);
    ptr->closure = c->closure;
    ptr->func_ptr = c->func_ptr;
    ptr->cif = c->cif;

    HFFI_TAIL_INT_PTR_ADD(c->closure, sizeof(ffi_closure), 1);
    HFFI_TAIL_INT_PTR_ADD(c->cif, sizeof(ffi_cif), 1);
    int count = array_list_size(c->in_vals);
    for(int i = 0 ; i < count ; i ++){
        array_list_add(ptr->in_vals, hffi_value_copy((hffi_value*)array_list_get(c->in_vals, i)));
    }
    return ptr;
}
void hffi_closure_ref(hffi_closure* hc, int c){
    atomic_add(&hc->ref, c);
}
//------------------------------------
//abi: default is 'FFI_DEFAULT_ABI'
hffi_cif* hffi_new_cif(int abi,array_list* in_vals, int var_count,hffi_value* out, char** msg){
    int in_count = array_list_size(in_vals);
    array_list* ins = array_list_new_max(array_list_size(in_vals));
    //add ref
    hffi_value* val;
    for(int i = 0 ; i < in_count ; i ++){
        val = array_list_get(in_vals, i);
        atomic_add(&val->ref, 1);
        array_list_add(ins, val);
    }
    atomic_add(&out->ref, 1);
    //build args
    ffi_type ** argTypes = NULL;
    void **args = NULL;
    ffi_cif* cif = NULL;
    if(in_count > 0){
        //argTypes must in heap memory when cif is in heap memory.
        argTypes = MALLOC(sizeof(ffi_type *) *in_count);
        args = MALLOC(sizeof(void *) *in_count);
        for(int i = 0 ; i < in_count ; i ++){
            val = array_list_get(in_vals, i);

            argTypes[i] = hffi_value_get_rawtype(val, msg);
            if(argTypes[i] == NULL){
                goto failed;
            }
            //cast param value
            args[i] = __get_data_ptr(val);
        }
    }
    //prepare cif
    ffi_type *return_type = hffi_value_get_rawtype(out, msg);
    if(return_type == NULL){
        goto failed;
    }
    cif = MALLOC(sizeof (ffi_cif));
    ffi_status s;
    if(var_count > 0){
        s = ffi_prep_cif_var(cif, abi, in_count - var_count,(unsigned int)in_count, return_type, argTypes);
    }else{
        s = ffi_prep_cif(cif, abi, (unsigned int)in_count, return_type, argTypes);
    }
    switch (s) {
    case FFI_OK:{
       // ffi_call(&cif, fn, __get_data_ptr(out), args);
        //prepare
        hffi_cif* hcif = MALLOC(sizeof (hffi_cif));
        hcif->cif = cif;
        hcif->args = args;
        hcif->in_vals = ins;
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
    failed:
    FREE(cif);
    array_list_delete2(ins, list_travel_value_delete);
    hffi_delete_value(out);
    FREE(args);
    return NULL;
}
void hffi_cif_ref(hffi_cif* hcif, int c){
    atomic_add(&hcif->ref, c);
}
void hffi_delete_cif(hffi_cif* hcif){
    if(atomic_add(&hcif->ref, -1) == 1){
        FREE(hcif->cif->arg_types);
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
