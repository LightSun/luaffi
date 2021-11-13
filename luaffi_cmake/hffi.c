
#include <stdio.h>
#include "hffi.h"
#include "atomic.h"
#include "h_linklist.h"
#include "h_alloctor.h"
#include "h_list.h"
#include "h_array.h"

#define hffi_new_value_auto_x(ffi_t,type) \
hffi_value* hffi_new_value_##type(type val){\
    hffi_value* v = hffi_new_value(ffi_t, HFFI_TYPE_VOID,sizeof(type));\
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
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    memset(val_ptr, 0, sizeof (hffi_value));

    val_ptr->ptr = MALLOC(sizeof (void*));
    val_ptr->base_ffi_type = FFI_TYPE_POINTER;
    val_ptr->pointer_base_type = hffi_t2;
    val_ptr->ref = 1;
    return val_ptr;
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

hffi_value* hffi_new_value_raw_type(sint8 ffi_t){
    int size = 0;
    switch (ffi_t) {
    case HFFI_TYPE_SINT8:{size = sizeof (sint8);}break;
    case HFFI_TYPE_UINT8:{size = sizeof (uint8);}break;
    case HFFI_TYPE_SINT16:{size = sizeof (sint16);}break;
    case HFFI_TYPE_UINT16:{size = sizeof (uint16);}break;
    case HFFI_TYPE_SINT32:{size = sizeof (sint32);}break;
    case HFFI_TYPE_UINT32:{size = sizeof (uint32);}break;
    case HFFI_TYPE_SINT64:{size = sizeof (sint64);}break;
    case HFFI_TYPE_UINT64:{size = sizeof (uint64);}break;
    case HFFI_TYPE_FLOAT:{size = sizeof (float);}break;
    case HFFI_TYPE_DOUBLE:{size = sizeof (double);}break;

    case HFFI_TYPE_INT:{size = sizeof (sint32);}break;
    default:
        return NULL;
    }
    return hffi_new_value(ffi_t, HFFI_TYPE_VOID, size);
    //return hffi_new_value(ffi_t, HFFI_TYPE_VOID, sizeof(void*));
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
void hffi_delete_value(hffi_value* val){
    int old = atomic_add(&val->ref, -1);
    if(old == 1){
        //if is struct.
        if(val->base_ffi_type == HFFI_TYPE_STRUCT){
            hffi_struct* c = val->ptr;
            hffi_delete_struct(c);
        }else{
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
void hffi_value_set_ptr_base_type(hffi_value* val, sint8 base){
    val->pointer_base_type = base;
}
ffi_type* hffi_value_get_rawtype(hffi_value* val, char** msg){
    if(val->base_ffi_type == HFFI_TYPE_STRUCT){
        return ((hffi_struct*)(val->ptr))->type;
    }
    if(val->base_ffi_type == HFFI_TYPE_HARRAY){
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

hffi_smtype* hffi_new_smtype(sint8 ffi_type, hffi_smtype** member_types){
    hffi_smtype* ptr = MALLOC(sizeof (hffi_smtype));
    ptr->ffi_type = ffi_type;
    ptr->ref = 1;
    ptr->_struct = NULL;
    ptr->_harray = NULL;
    ptr->elements = member_types;
    if(member_types != NULL){
        int i = 0;
        while(member_types[i] != NULL){
            atomic_add(&member_types[i]->ref, 1);
            i++;
        }
    }
    return ptr;
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
    ptr->_harray = NULL;
    ptr->elements = NULL;
    atomic_add(&array->ref, 1);
    return ptr;
}
hffi_smtype* hffi_new_smtype_harray_ptr(struct harray* array){
    hffi_smtype* ptr = MALLOC(sizeof (hffi_smtype));
    ptr->ffi_type = HFFI_TYPE_POINTER;
    ptr->ref = 1;
    ptr->_harray = array;
    ptr->_harray = NULL;
    ptr->elements = NULL;
    atomic_add(&array->ref, 1);
    return ptr;
}
void hffi_delete_smtype(hffi_smtype* type){
    int old = atomic_add(&type->ref, -1);
    if(old == 1){
        if(type->elements != NULL){
            int count = 0;
            while(type->elements[count] != NULL){
                hffi_delete_smtype(type->elements[count]);
                count++;
            }
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
//--------------------------------------------------------------
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

//--------------------------------------

static void  __set_children_data_travel(void* ud, int size, int index,void* ele){
    hffi_struct* p = ud;
    size_t * offsets = (size_t *) &p->type->elements[p->count+1];
    struct_item* item = ele;

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
static inline void __set_children_data(hffi_struct* p){
    array_list_travel(p->children, __set_children_data_travel, p);
}

//------------------------
static hffi_struct* hffi_new_struct_from_list0(int abi,struct array_list* list, sint16 parent_pos, char** msg);
static hffi_struct* hffi_new_struct_abi0(int abi,hffi_smtype** member_types, sint16 parent_pos, char** msg);
//------------------------
hffi_struct* hffi_new_struct_base(sint8* types, int count){
    hffi_smtype* smtypes[count +1];
    for(int i = 0 ; i < count; i ++){
        smtypes[i] = hffi_new_smtype(types[i], NULL);
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
    array_list* children = array_list_new(count / 2 < 4 ? 4 : count / 2, 0.75);
    //sub_types(ffi_type*) which need manmul release
    array_list* sub_types = array_list_new_simple();

    //struct data
    int type_size = sizeof(ffi_type) + sizeof (ffi_type *) * (count+1) + sizeof (size_t) * count;
    ffi_type *type;
    size_t * offsets = NULL;
    //raw_type, elements, offsets.
    type = (ffi_type *) MALLOC(type_size);
    type->size = type->alignment = 0;
    type->type = FFI_TYPE_STRUCT;
    type->elements = (ffi_type**)(type + 1);

    /*
     * 1, already-struct : copy struct data to parent target-pos.
     * 2. already-struct-ptr:set parent target-pos-ptr to the struct data-ptr
     * 3, already-harray: copy array data to parent (note: harray-nested struct.)
     * 4, already-harray-ptr: set parent target-pos-ptr to the harray data-ptr
     * 5, hffi_smtype for struct: create new struct and make the struct data-ptr to parent's.
     */
    ffi_type* tmp_type;
    hffi_smtype* tmp_smtype;
    hffi_struct* tm_struct;
    harray* tmp_harr;
    for(int i = count -1 ; i >=0 ; i --){
        //struct.
        tmp_smtype = array_list_get(list, i);
        if(tmp_smtype->ffi_type == HFFI_TYPE_STRUCT){
            if(tmp_smtype->_struct != NULL){
                //is already a struct
                tm_struct = (hffi_struct*)tmp_smtype->_struct;
                atomic_add(&tm_struct->ref, 1);
                tmp_type = tm_struct->type;
                array_list_add(children, __new_struct_item(i, HFFI_TYPE_STRUCT, _ITEM_COPY_DATA, tm_struct));
            }else{
                //create sub struct. latter will set data-memory-pointer
                tm_struct = hffi_new_struct_abi0(abi, tmp_smtype->elements, i, msg);
                if(tm_struct == NULL){
                    goto failed;
                }
                tmp_type = tm_struct->type;
                array_list_add(children, __new_struct_item(i, HFFI_TYPE_STRUCT, _ITEM_SET_DATA_PTR, tm_struct));
            }
        }else if(tmp_smtype->ffi_type == HFFI_TYPE_HARRAY){
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
                tm_struct = (hffi_struct*)tmp_smtype->_struct;
                atomic_add(&tm_struct->ref, 1);
                array_list_add(children, __new_struct_item(i, HFFI_TYPE_STRUCT_PTR, _ITEM_SET_DATA_PTR, tm_struct));
            }else if(tmp_smtype->_harray != NULL){
                tmp_harr = tmp_smtype->_harray;
                atomic_add(&tmp_harr->ref, 1);
                array_list_add(children, __new_struct_item(i, HFFI_TYPE_HARRAY_PTR, _ITEM_SET_DATA_PTR, tmp_harr));
            }
        }else{
            tmp_type = to_ffi_type(tmp_smtype->ffi_type, msg);
            if(tmp_type == NULL){
                goto failed;
            }
        }
        type->elements[i] = tmp_type;
    }
    type->elements[count] = NULL;
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
    ptr->type = type;
    ptr->type_size = type_size;
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
    hffi_struct* ptr = MALLOC(sizeof(hffi_struct));
    ptr->type = MALLOC(_hs->type_size);
    ptr->type_size = _hs->type_size;
    ptr->count = _hs->count;
    ptr->data_size = _hs->data_size;
   // ptr->children = children;//TODO deep copy
    ptr->ref = 1;
    ptr->parent_pos = -1;
    if(_hs->data){
        ptr->data = MALLOC(_hs->data_size);
        memcpy(ptr->data, _hs->data,  _hs->data_size);
    }
    //copy data.
    memcpy(ptr->type, _hs->type,  _hs->type_size);
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
//--------------------------------------------
//x  eg: func_data(old->data);
#define RELEASE_LINK_LIST(list, x) \
if(list){\
    linklist_node* node = list;\
    linklist_node* old;\
    do{\
        old = node;\
        node = node->next;\
        {x;}\
        FREE(old);\
    }while(node != NULL);\
}
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
    if(index < 0){
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
