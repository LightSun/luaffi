
#include <stdio.h>
#include "h_array.h"
#include "atomic.h"
#include "h_alloctor.h"
#include "h_list.h"
#include "h_string.h"

static inline int get_element_size(sint8 hffi_t){
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
harray* harray_new_structs(struct array_list* structs){
    int count = array_list_size(structs);
    if(count == 0) return NULL;
    int every_size = hffi_struct_get_data_size(array_list_get(structs, 0));
    harray* arr = MALLOC(sizeof (harray));
    arr->free_data = 1;
    arr->ele_count = count;
    arr->data_size = count * every_size;
    arr->data = MALLOC(arr->data_size);
    arr->ref = 1;
    arr->hffi_t = HFFI_TYPE_STRUCT;
    arr->ele_list = MALLOC(sizeof (void*) * count);
    memset(arr->ele_list, 0 , sizeof (sizeof (void*) * count));
    //copy data
    int startPos = 0;
    struct hffi_struct* _item;
    for(int i = 0 ; i < count ; i ++){
        _item = array_list_get(structs, i);
        memcpy(arr->data + startPos, hffi_struct_get_data(_item), every_size);
        startPos += every_size;
        //ref to ele list
        hffi_struct_ref(_item, 1);
        arr->ele_list[i] = _item;
    }
    return arr;
}
harray* harray_new_arrays(struct array_list* arrays){
    int count = array_list_size(arrays);
    if(count == 0) return NULL;
    int every_size = ((harray*)array_list_get(arrays, 0))->data_size;
    harray* arr = MALLOC(sizeof (harray));
    arr->free_data = 1;
    arr->ele_count = count;
    arr->data_size = count * every_size;
    arr->data = MALLOC(arr->data_size);
    arr->ref = 1;
    arr->hffi_t = HFFI_TYPE_HARRAY;
    arr->ele_list = MALLOC(sizeof (void*) * count);
    memset(arr->ele_list, 0 , sizeof (sizeof (void*) * count));
    //copy data
    int startPos = 0;
    harray* _item;
    for(int i = 0 ; i < count ; i ++){
        _item = array_list_get(arrays, i);
        memcpy(arr->data + startPos, _item->data, every_size);
        startPos += every_size;
        //ref to ele list
        harray_ref(_item, 1);
        arr->ele_list[i] = _item;
    }
    return arr;
}
harray* harray_new_struct_ptr(int count){
    return harray_new(HFFI_TYPE_STRUCT_PTR, count);
}
harray* harray_new_array_ptr(int count){
    return harray_new(HFFI_TYPE_HARRAY_PTR, count);
}
harray* harray_new(sint8 hffi_t, int count){
    int ele_size = get_element_size(hffi_t);
    harray* arr = MALLOC( sizeof (harray));
    arr->free_data = 1;
    arr->hffi_t = hffi_t;
    arr->data = MALLOC(ele_size * count);
    arr->data_size = ele_size * count;
    arr->ele_count = count;
    arr->ref = 1;
    switch (hffi_t) {
    case HFFI_TYPE_STRUCT_PTR:
    case HFFI_TYPE_STRUCT:
    case HFFI_TYPE_HARRAY_PTR:
    case HFFI_TYPE_HARRAY:{
        arr->ele_list = MALLOC(sizeof (void*) * count);
        memset(arr->ele_list, 0, sizeof (void*) * count);
    }break;
    default:
        arr->ele_list = NULL;
    }
    return arr;
}
//default as string
harray* harray_new_char(int count){
    harray* arr = harray_new(HFFI_TYPE_SINT8, count);
    ((char*)arr->data)[count - 1] = '\0';
    return arr;
}
harray* harray_new_chars(const char* str){
    int c = strlen(str) + 1;
    harray* arr = harray_new(HFFI_TYPE_SINT8, c);
    ((char*)arr->data)[c - 1] = '\0';
    strcpy((char*)arr->data, str);
    return arr;
}
harray* harray_new_chars2(const char* str, int len){
    int c = strlen(str) + 1;
    harray* arr = harray_new(HFFI_TYPE_SINT8, len);
    ((char*)arr->data)[c - 1] = '\0';
    memcpy(arr->data, str, strlen(str));
    return arr;
}
harray* harray_new_from_data(sint8 hffi_t, void* data, int data_size, int ele_count, sint8 free_data){
    harray* arr = MALLOC( sizeof (harray));
    arr->free_data = free_data;
    arr->data_size = data_size;
    arr->ele_count = ele_count;
    arr->hffi_t = hffi_t;
    arr->data = data;
    arr->ref = 1;

    switch (hffi_t) {
    case HFFI_TYPE_STRUCT_PTR:
    case HFFI_TYPE_STRUCT:
    case HFFI_TYPE_HARRAY_PTR:
    case HFFI_TYPE_HARRAY:{
        arr->ele_list = MALLOC(sizeof (void*) * ele_count);
        memset(arr->ele_list, 0 , sizeof (sizeof (void*) * ele_count));
    }break;
    default:
        arr->ele_list = NULL;
    }
    return arr;
}
harray* harray_copy(harray* src){
    harray* arr = MALLOC( sizeof (harray));
    arr->free_data = 1;
    arr->ele_list = NULL;
    arr->hffi_t = src->hffi_t;
    arr->data = MALLOC(src->data_size);
    arr->data_size = src->data_size;
    arr->ele_count = src->ele_count;
    arr->ref = 1;
    if(src->ele_list){
        arr->ele_list = MALLOC(sizeof (void*) * src->ele_count);
        memset(arr->ele_list, 0, sizeof (void*) * src->ele_count);
        switch (src->hffi_t) {
        case HFFI_TYPE_HARRAY:
        {
            memcpy(arr->data, src->data, src->data_size);
            for(int i = 0 ; i < src->ele_count ; i ++){
                if(src->ele_list[i]){
                    arr->ele_list[i] = harray_copy((harray*)src->ele_list[i]);
                }else{
                    arr->ele_list[i] = NULL;
                }
            }
        }break;
        case HFFI_TYPE_HARRAY_PTR:
        {
            for(int i = 0 ; i < src->ele_count ; i ++){
                if(src->ele_list[i]){
                    ((void**)arr->data)[i] = arr->ele_list[i] = harray_copy((harray*)src->ele_list[i]);
                }else{
                    ((void**)arr->data)[i] = arr->ele_list[i] = NULL;
                }
            }
        }break;

        case HFFI_TYPE_STRUCT:{
            memcpy(arr->data, src->data, src->data_size);
            for(int i = 0 ; i < src->ele_count ; i ++){
                 if(src->ele_list[i]){
                     arr->ele_list[i] = hffi_struct_copy((struct hffi_struct*)src->ele_list[i]);
                 }else{
                     arr->ele_list[i] = NULL;
                 }
            }
        }break;

        case HFFI_TYPE_STRUCT_PTR:{
            for(int i = 0 ; i < src->ele_count ; i ++){
                if(src->ele_list[i]){
                    ((void**)arr->data)[i] = arr->ele_list[i] = hffi_struct_copy((struct hffi_struct*)src->ele_list[i]);
                }else{
                    ((void**)arr->data)[i] = arr->ele_list[i] = NULL;
                }
            }
        }break;

        default:
            abort();//can't reach here
        }
    }else{
        memcpy(arr->data, src->data, src->data_size);
    }
    return arr;
}
static inline void __delete_ele(void* ud,void* ele){
    if(ele){
        int hffi_t = *((int*)ud);
        if(hffi_t == HFFI_TYPE_HARRAY || hffi_t == HFFI_TYPE_HARRAY_PTR){
            harray_delete((harray*)ele);
        }else if(hffi_t == HFFI_TYPE_STRUCT || hffi_t == HFFI_TYPE_STRUCT_PTR){
            hffi_delete_struct((struct hffi_struct*)ele);
        }
    }
}
void harray_delete(harray* arr){
    if(atomic_add(&arr->ref, -1) == 1){
        int hffi_t = arr->hffi_t;
        if(arr->ele_list){
            for(int i = 0 ; i < arr->ele_count ; i ++){
                if(arr->ele_list[i]){
                    __delete_ele(&hffi_t, arr->ele_list[i]);
                }
            }
            FREE(arr->ele_list);
        }
        if(arr->free_data){
            FREE(arr->data);
        }
        FREE(arr);
    }
}
void harray_ref(harray* arr, int c){
    atomic_add(&arr->ref, c);
}
int harray_get_count(harray* arr){
    return arr->ele_count;
}

#define __GET_I(hffi_t, t)\
case hffi_t:{\
    ptr->_##t = ((t*)arr->data)[index];\
}return HFFI_STATE_OK;

int harray_geti(harray* arr, int index, union harray_ele* ptr){
    if(index >= arr->ele_count){
        return HFFI_STATE_FAILED;
    }
    DEF_HFFI_BASE_SWITCH(__GET_I, arr->hffi_t)
    switch (arr->hffi_t) {       
        case HFFI_TYPE_STRUCT_PTR:
        case HFFI_TYPE_HARRAY_PTR:{
            if(arr->ele_list == NULL){
                return HFFI_STATE_FAILED;
            }
            void *data = arr->ele_list[index];
            if(data){
                ptr->_extra = data;
                //printf("get harray: index = %d, arr_addr = %p\n", index, ptr->_extra);
                return HFFI_STATE_OK;
            }
        }break;

        case HFFI_TYPE_HARRAY:{
            //find old
            void *data =  arr->ele_list[index];
            if(data){
                ptr->_extra = data;
                //must provide an array to copy daya.
                int target_size = ((harray*)ptr->_extra)->data_size;
                void* data_ptr = arr->data + index * (arr->data_size / arr->ele_count);
                memcpy(((harray*)ptr->_extra)->data, data_ptr, target_size);
                return HFFI_STATE_OK;
            }
        }break;

        case HFFI_TYPE_STRUCT:{
            //find old
            void *data = arr->ele_list[index];
            if(data){
                ptr->_extra = data;
                int target_size = hffi_struct_get_data_size((struct hffi_struct*)ptr->_extra);
                void* data_ptr = arr->data + index * (arr->data_size / arr->ele_count);
                memcpy(hffi_struct_get_data((struct hffi_struct*)ptr->_extra), data_ptr, target_size);
                return HFFI_STATE_OK;
            }
        }break;
    }
    return HFFI_STATE_FAILED;
}

#define __SET_I(hffi_t, t)\
case hffi_t:{\
    ((t*)arr->data)[index] = ptr->_##t;\
}return HFFI_STATE_OK;

#define __SET_I_2(hffi_t, t)\
case hffi_t:{\
    ((t*)arr->data)[index] = *((t*)ptr);\
}return HFFI_STATE_OK;

int harray_seti(harray* arr, int index, union harray_ele* ptr){
    //for harray or struct we need make data-connected
    if(index >= arr->ele_count){
        return HFFI_STATE_FAILED;
    }
    DEF_HFFI_BASE_SWITCH(__SET_I, arr->hffi_t)

    switch (arr->hffi_t) {   

    case HFFI_TYPE_HARRAY_PTR:{
        if(arr->ele_list[index] != NULL){
            harray_delete(((harray*)arr->ele_list[index]));
        }
        if(ptr->_extra == NULL){
            void** data = arr->data;
            data[index] = NULL;
            arr->ele_list[index] = NULL;
        }else{
            harray_ref((harray*)ptr->_extra, 1);
            void** data = arr->data;
            data[index] = ((harray*)ptr->_extra)->data;
            arr->ele_list[index] = ptr->_extra;
        }
        return HFFI_STATE_OK;
    }break;
    case HFFI_TYPE_STRUCT_PTR:{
        if(arr->ele_list[index] != NULL){
            hffi_delete_struct((struct hffi_struct*)arr->ele_list[index]);
        }
        if(ptr->_extra == NULL){
            void** data = arr->data;
            data[index] = NULL;
            arr->ele_list[index] = NULL;
        }else{
            hffi_struct_ref((struct hffi_struct*)ptr->_extra, 1);
            void** data = arr->data;
            data[index] = hffi_struct_get_data((struct hffi_struct*)ptr->_extra);
            arr->ele_list[index] = ptr->_extra;
        }
        return HFFI_STATE_OK;
    }break;

    case HFFI_TYPE_HARRAY:{
        if(ptr->_extra == NULL){
            return HFFI_STATE_FAILED;
        }
        //if current no entity, we hold it for latter use.
        if(arr->ele_list[index] == NULL){
            arr->ele_list[index] = ptr->_extra;
            harray_ref((harray*)ptr->_extra, 1);
        }
        //copy data
        int target_size = ((harray*)ptr->_extra)->data_size;
        if(target_size != arr->data_size / arr->ele_count){
            return HFFI_STATE_FAILED;
        }
        void* data_ptr = arr->data + index * (arr->data_size / arr->ele_count);
        memcpy(data_ptr, ((harray*)ptr->_extra)->data, target_size);
        return HFFI_STATE_OK;
    }break;
    case HFFI_TYPE_STRUCT:{
        if(ptr->_extra == NULL){
            return HFFI_STATE_FAILED;
        }
        //if current no entity, we hold it for latter use.
        if(arr->ele_list[index] == NULL){
            arr->ele_list[index] = ptr->_extra;
            hffi_struct_ref((struct hffi_struct*)ptr->_extra, 1);
        }
        //copy data
        int target_size = hffi_struct_get_data_size((struct hffi_struct*)ptr->_extra);
        if(target_size != arr->data_size / arr->ele_count){
            return HFFI_STATE_FAILED;
        }
        void* data_ptr = arr->data + index * (arr->data_size / arr->ele_count);
        memcpy(data_ptr, hffi_struct_get_data((struct hffi_struct*)ptr->_extra), target_size);
        return HFFI_STATE_OK;
    }break;
    }
    return HFFI_STATE_FAILED;
}

int harray_seti2(harray* arr, int index, void* ptr){
    if(index >= arr->ele_count){
        return HFFI_STATE_FAILED;
    }
    DEF_HFFI_BASE_SWITCH(__SET_I_2, arr->hffi_t)
    switch (arr->hffi_t) {    
    case HFFI_TYPE_HARRAY_PTR:{
        if(arr->ele_list[index] != NULL){
            harray_delete(((harray*)arr->ele_list[index]));
        }
        if(ptr == NULL){
            void** data = arr->data;
            data[index] = NULL;
            arr->ele_list[index] = NULL;
        }else{
            harray_ref((harray*)ptr, 1);
            void** data = arr->data;
            data[index] = ((harray*)ptr)->data;
            arr->ele_list[index] = ptr;
        }
        return HFFI_STATE_OK;
    }break;
    case HFFI_TYPE_STRUCT_PTR:{
        if(arr->ele_list[index] != NULL){
            hffi_delete_struct((struct hffi_struct*)arr->ele_list[index]);
        }
        if(ptr == NULL){
            void** data = arr->data;
            data[index] = NULL;
            arr->ele_list[index] = NULL;
        }else{
            hffi_struct_ref((struct hffi_struct*)ptr, 1);
            void** data = arr->data;
            data[index] = hffi_struct_get_data((struct hffi_struct*)ptr);
            arr->ele_list[index] = ptr;
        }
        return HFFI_STATE_OK;
    }break;

    case HFFI_TYPE_HARRAY:{
        if(ptr == NULL){
            return HFFI_STATE_FAILED;
        }
        //if current no entity, we hold it for latter use.
        if(arr->ele_list[index] == NULL){
            arr->ele_list[index] = ptr;
            harray_ref((harray*)ptr, 1);
        }
        //must provide an array to copy daya.
        int target_size = ((harray*)ptr)->data_size;
        if(target_size != arr->data_size / arr->ele_count){
            return HFFI_STATE_FAILED;
        }
        void* data_ptr = arr->data + index * (arr->data_size / arr->ele_count);
        memcpy(data_ptr, ((harray*)ptr)->data, target_size);
        return HFFI_STATE_OK;
    }break;
    case HFFI_TYPE_STRUCT:{
        if(ptr == NULL){
            return HFFI_STATE_FAILED;
        }
        //if current no entity, we hold it for latter use.
        if(arr->ele_list[index] == NULL){
            arr->ele_list[index] = ptr;
            hffi_struct_ref((struct hffi_struct*)ptr, 1);
        }
        //copy-data
        int target_size = hffi_struct_get_data_size((struct hffi_struct*)ptr);
        if(target_size != arr->data_size / arr->ele_count){
            return HFFI_STATE_FAILED;
        }
        void* data_ptr = arr->data + index * (arr->data_size / arr->ele_count);
        memcpy(data_ptr, hffi_struct_get_data((struct hffi_struct*)ptr), target_size);
        return HFFI_STATE_OK;
    }break;
    }
    return HFFI_STATE_FAILED;
}


#define harray_dump_impl(hffi_t, type, format)\
case hffi_t:{\
    for(int i = 0 ; i < arr->ele_count ; i ++){\
        if(i != 0){hstring_append(hs, ", ");}\
        hstring_appendf(hs, format, ((type*)arr->data)[i]);\
    }\
}break;

void harray_dump(harray* arr, struct hstring* hs){
    hstring_append(hs, "[");
    switch (arr->hffi_t) {
    harray_dump_impl(HFFI_TYPE_SINT8, sint8, "%d");
    harray_dump_impl(HFFI_TYPE_UINT8, uint8, "%d");
    harray_dump_impl(HFFI_TYPE_SINT16, sint16, "%d");
    harray_dump_impl(HFFI_TYPE_UINT16, uint16, "%u");
    harray_dump_impl(HFFI_TYPE_SINT32, sint32, "%d");
    harray_dump_impl(HFFI_TYPE_UINT32, uint32, "%u");
    harray_dump_impl(HFFI_TYPE_SINT64, sint64, "%lld");
    harray_dump_impl(HFFI_TYPE_UINT64, uint64, "%llu");
    harray_dump_impl(HFFI_TYPE_INT, sint32, "%d");

    case HFFI_TYPE_HARRAY:
    case HFFI_TYPE_HARRAY_PTR:{
        union harray_ele ele;
        for(int i = 0 ; i < arr->ele_count ; i ++){
            harray_seti(arr, i, &ele);
            if(i != 0){hstring_append(hs, ", ");}
            harray_dump(ele._extra, hs);
        }
    }break;
    //TODO latter support struct
    case HFFI_TYPE_STRUCT:{
         hstring_append(hs, "<struct>");
    }break;
    case HFFI_TYPE_STRUCT_PTR:{
        hstring_append(hs, "<struct_ptr>");
    }break;
    }
    hstring_append(hs, "]");
}
