
#include <stdio.h>
#include "h_array.h"
#include "atomic.h"
#include "h_alloctor.h"
#include "h_list.h"
#include "h_string.h"
#include "h_float_bits.h"

#define DEF_HARRAY_ALLOC_DATA(x)\
harray* arr = x;\
if(arr == NULL) return NULL;\
arr->data = MALLOC(arr->data_size);\
memset(arr->data, 0, arr->data_size);\
arr->free_data = 1;

static inline void __harray_detach_child_array(harray* arr, int index){
    harray* sub_arr = arr->ele_list[index];
    if(sub_arr){
        HFFI_CLEAR_PARENT(sub_arr);
        harray_delete(sub_arr);
        arr->ele_list[index] = NULL;
    }
}
static inline void __harray_detach_child_struct(harray* arr, int index){
    struct hffi_struct* sub_arr = arr->ele_list[index];
    if(sub_arr){
        HFFI_CLEAR_PARENT(sub_arr);
        hffi_delete_struct(sub_arr);
        arr->ele_list[index] = NULL;
    }
}
static inline void __harray_attach_child_array(harray* arr, int index, harray* child, int need_parent){
    arr->ele_list[index] = child;
    if(child){
        harray_ref(child, 1);
        if(need_parent){
            HFFI_SET_PARENT(index, child, arr, HFFI_TYPE_HARRAY);
        }
    }
}
static inline void __harray_attach_child_struct(harray* arr, int index, struct hffi_struct* child, int need_parent){
    arr->ele_list[index] = child;
    if(child){
        hffi_struct_ref(child, 1);
        if(need_parent){
            HFFI_SET_PARENT(index, child, arr, HFFI_TYPE_HARRAY);
        }
    }
}

//======================================================

harray* harray_new_structs(struct array_list* structs){
    int every_size = hffi_struct_get_data_size(array_list_get(structs, 0));
    DEF_HARRAY_ALLOC_DATA(harray_new_structs_nodata(structs))
    //copy data
    int startPos = 0;
    struct hffi_struct* _item;
    void* item_data;
    for(int i = 0 ; i < arr->ele_count ; i ++){
        _item = array_list_get(structs, i);
        item_data = hffi_struct_get_data(_item);
        if(item_data == NULL){
            harray_delete(arr);
            printf("struct with no data can't used to create harray");
            return NULL;
        }
        memcpy(arr->data + startPos, item_data, every_size);
        startPos += every_size;
    }
    return arr;
}
harray* harray_new_arrays(struct array_list* arrays){
    DEF_HARRAY_ALLOC_DATA(harray_new_arrays_nodata(arrays))
    //
    int every_size = ((harray*)array_list_get(arrays, 0))->data_size;
    //copy data
    int startPos = 0;
    harray* _item;
    for(int i = 0 ; i < arr->ele_count ; i ++){
        _item = array_list_get(arrays, i);
        if(_item->data == NULL){
            harray_delete(arr);
            printf("harray with no data can't used to create harray.");
            return NULL;
        }
        memcpy(arr->data + startPos, _item->data, every_size);
        startPos += every_size;
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
    DEF_HARRAY_ALLOC_DATA(harray_new_nodata(hffi_t, count))
    return arr;
}
//-------------------------------------------------
harray* harray_new_structs_nodata(struct array_list* structs){
    int count = array_list_size(structs);
    if(count == 0) return NULL;
    int every_size = hffi_struct_get_data_size(array_list_get(structs, 0));
    harray* arr = MALLOC(sizeof (harray));
    arr->parent = NULL;
    arr->free_data = 1;
    arr->ele_count = count;
    arr->data_size = count * every_size;
    arr->data =  NULL;//MALLOC(arr->data_size);
    arr->ref = 1;
    arr->hffi_t = HFFI_TYPE_STRUCT;
    arr->ele_list = MALLOC(sizeof (void*) * count);
    memset(arr->ele_list, 0 , sizeof (sizeof (void*) * count));
    //copy data
    struct hffi_struct* _item;
    for(int i = 0 ; i < count ; i ++){
        _item = array_list_get(structs, i);
        //ref to ele list
        hffi_struct_ref(_item, 1);
        arr->ele_list[i] = _item;
    }
    return arr;
}
harray* harray_new_arrays_nodata(struct array_list* arrays){
    int count = array_list_size(arrays);
    if(count == 0) return NULL;
    int every_size = ((harray*)array_list_get(arrays, 0))->data_size;
    harray* arr = MALLOC(sizeof (harray));
    arr->parent = NULL;
    arr->free_data = 1;
    arr->ele_count = count;
    arr->data_size = count * every_size;
    arr->data = NULL; //MALLOC(arr->data_size);
    arr->ref = 1;
    arr->hffi_t = HFFI_TYPE_HARRAY;
    arr->ele_list = MALLOC(sizeof (void*) * count);
    memset(arr->ele_list, 0, sizeof (sizeof (void*) * count));
    //ref array
    harray* _item;
    for(int i = 0 ; i < count ; i ++){
        _item = array_list_get(arrays, i);
        //ref to ele list
        harray_ref(_item, 1);
        arr->ele_list[i] = _item;
    }
    return arr;
}
harray* harray_new_array_ptr_nodata(int count){
    return harray_new_nodata(HFFI_TYPE_HARRAY_PTR, count);
}
harray* harray_new_struct_ptr_nodata(int count){
    return harray_new_nodata(HFFI_TYPE_HARRAY_PTR, count);
}
harray* harray_new_nodata(sint8 hffi_t, int count){
    if(hffi_t == HFFI_TYPE_STRUCT || hffi_t == HFFI_TYPE_HARRAY){
        return NULL;
    }
    int ele_size = hffi_base_type_size(hffi_t);
    harray* arr = MALLOC( sizeof (harray));
    arr->parent = NULL;
    arr->free_data = 1;
    arr->hffi_t = hffi_t;
    arr->data = NULL; //MALLOC(ele_size * count);
    arr->data_size = ele_size * count;
    arr->ele_count = count;
    arr->ref = 1;
    switch (hffi_t) {
    case HFFI_TYPE_STRUCT_PTR:
    case HFFI_TYPE_HARRAY_PTR:
    {
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
    arr->parent = NULL;
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
        memset(arr->ele_list, 0 , sizeof (void*) * ele_count);
    }break;
    default:
        arr->ele_list = NULL;
    }
    return arr;
}
harray* harray_copy(harray* src){
    harray* arr = MALLOC( sizeof (harray));
    arr->parent = NULL;
    arr->free_data = 1;
    arr->ele_list = NULL;
    arr->hffi_t = src->hffi_t;
    arr->free_data = 1;
    if(src->data){
        arr->data = MALLOC(src->data_size);
    }else{
        arr->data = NULL;
    }
    arr->data_size = src->data_size;
    arr->ele_count = src->ele_count;
    arr->ref = 1;
    //no data.
    if(!arr->data){
        return arr;
    }
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
                    HFFI_SET_PARENT(i,((harray*)src->ele_list[i]), arr, HFFI_TYPE_HARRAY);
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
                     HFFI_SET_PARENT(i, ((struct hffi_struct*)src->ele_list[i]), arr, HFFI_TYPE_HARRAY);
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
        //printf("-- harray_delete >>> start delete harray: %p\n", arr);
        int hffi_t = arr->hffi_t;
        if(arr->ele_list){
            for(int i = 0 ; i < arr->ele_count ; i ++){
                if(arr->ele_list[i]){
                    __delete_ele(&hffi_t, arr->ele_list[i]);
                }
            }
            FREE(arr->ele_list);
        }
        if(arr->free_data && arr->data){
            FREE(arr->data);
        }
        HFFI_FREE_PARENT(arr)
        FREE(arr);
    }
}
void harray_ref(harray* arr, int c){
    atomic_add(&arr->ref, c);
}

#define __GET_I(hffi_t, t)\
case hffi_t:{\
    ptr->_##t = ((t*)arr->data)[index];\
}return HFFI_STATE_OK;

int harray_geti(harray* arr, int index, union harray_ele* ptr){
    if(arr->hffi_t == HFFI_TYPE_SINT16){
        printf("harray_geti: index = %d, val = %d\n", index, ((sint16*)arr->data)[index]);
    }
    if(index >= arr->ele_count){
        return HFFI_STATE_FAILED;
    }
    DEF_HFFI_BASE_SWITCH(__GET_I, arr->hffi_t)
    switch (arr->hffi_t) {
        case HFFI_TYPE_POINTER:{
             ptr->_extra = ((void**)arr->data)[index];
             return HFFI_STATE_OK;
        }break;
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
    /*if(hffi_t == HFFI_TYPE_SINT16) printf("__SET_I:  index = %d, val = %d\n", index, ((t*)arr->data)[index]); */\
}return HFFI_STATE_OK;

#define __SET_I_2(hffi_t, t)\
case hffi_t:{\
    ((t*)arr->data)[index] = *((t*)ptr);\
}return HFFI_STATE_OK;

int harray_seti(harray* arr, int index, union harray_ele* ptr){
    if(!arr->data){
        return HFFI_STATE_FAILED;
    }
    //for harray or struct we need make data-connected
    if(index >= arr->ele_count){
        return HFFI_STATE_FAILED;
    }
    DEF_HFFI_BASE_SWITCH(__SET_I, arr->hffi_t)

    switch (arr->hffi_t) {

    case HFFI_TYPE_POINTER:{
        if(!arr->data){
            arr->data = MALLOC(sizeof (void*) * arr->ele_count);
            arr->free_data = 1;
        }
        ((void**)arr->data)[index] = ptr->_extra;
    } return HFFI_STATE_OK;

    case HFFI_TYPE_HARRAY_PTR:{
        __harray_detach_child_array(arr, index);

        if(ptr->_extra == NULL){
            void** data = arr->data;
            data[index] = NULL;
            arr->ele_list[index] = NULL;
        }else{
            __harray_attach_child_array(arr, index, (harray*)ptr->_extra, 0);
            void** data = arr->data;
            data[index] = ((harray*)ptr->_extra)->data;
        }
        return HFFI_STATE_OK;
    }break;
    case HFFI_TYPE_STRUCT_PTR:{
        __harray_detach_child_struct(arr, index);

        if(ptr->_extra == NULL){
            void** data = arr->data;
            data[index] = NULL;
            arr->ele_list[index] = NULL;
        }else{
            __harray_attach_child_struct(arr, index, (struct hffi_struct*)ptr->_extra, 0);
            void** data = arr->data;
            data[index] = hffi_struct_get_data((struct hffi_struct*)ptr->_extra);
        }
        return HFFI_STATE_OK;
    }break;

    case HFFI_TYPE_HARRAY:{
        if(ptr->_extra == NULL || !((harray*)ptr->_extra)->data){
            return HFFI_STATE_FAILED;
        }
        //check-datas-size
        int target_size = ((harray*)ptr->_extra)->data_size;
        if(target_size != arr->data_size / arr->ele_count){
            return HFFI_STATE_FAILED;
        }
        //detach old and attach new
        __harray_detach_child_array(arr, index);
        __harray_attach_child_array(arr, index, (harray*)ptr->_extra, 1);
        //copy data
        void* data_ptr = arr->data + index * (arr->data_size / arr->ele_count);
        memcpy(data_ptr, ((harray*)ptr->_extra)->data, target_size);
        return HFFI_STATE_OK;
    }break;
    case HFFI_TYPE_STRUCT:{
        if(ptr->_extra == NULL || hffi_struct_get_data((struct hffi_struct*)ptr->_extra) == NULL){
            return HFFI_STATE_FAILED;
        }
        //check-datas-size
        int target_size = hffi_struct_get_data_size((struct hffi_struct*)ptr->_extra);
        if(target_size != arr->data_size / arr->ele_count){
            return HFFI_STATE_FAILED;
        }
        //detach old and attach new
        __harray_detach_child_struct(arr, index);
        __harray_attach_child_struct(arr, index, (struct hffi_struct*)ptr->_extra, 1);
         //copy data
        void* data_ptr = arr->data + index * (arr->data_size / arr->ele_count);
        memcpy(data_ptr, hffi_struct_get_data((struct hffi_struct*)ptr->_extra), target_size);
        return HFFI_STATE_OK;
    }break;
    }
    return HFFI_STATE_FAILED;
}

int harray_seti2(harray* arr, int index, void* ptr){
    if(!arr->data){
        return HFFI_STATE_FAILED;
    }
    if(index >= arr->ele_count){
        return HFFI_STATE_FAILED;
    }
    DEF_HFFI_BASE_SWITCH(__SET_I_2, arr->hffi_t)
    switch (arr->hffi_t) {
    case HFFI_TYPE_POINTER:{
        if(!arr->data){
            arr->data = MALLOC(sizeof (void*) * arr->ele_count);
            arr->free_data = 1;
        }
        ((void**)arr->data)[index] = ptr;
    } return HFFI_STATE_OK;

    case HFFI_TYPE_HARRAY_PTR:{
        __harray_detach_child_array(arr, index);
        if(ptr == NULL){
            void** data = arr->data;
            data[index] = NULL;
            arr->ele_list[index] = NULL;
        }else{
            __harray_attach_child_array(arr, index, (harray*)ptr, 0);
            void** data = arr->data;
            data[index] = ((harray*)ptr)->data;
        }
        return HFFI_STATE_OK;
    }break;
    case HFFI_TYPE_STRUCT_PTR:{
         __harray_detach_child_struct(arr, index);

        if(ptr == NULL){
            void** data = arr->data;
            data[index] = NULL;
            arr->ele_list[index] = NULL;
        }else{
            __harray_attach_child_struct(arr, index, (struct hffi_struct*)ptr, 0);
            void** data = arr->data;
            data[index] = hffi_struct_get_data((struct hffi_struct*)ptr);
        }
        return HFFI_STATE_OK;
    }break;

    case HFFI_TYPE_HARRAY:{
        if(ptr == NULL || !((harray*)ptr)->data){
            return HFFI_STATE_FAILED;
        }   
        //check-data-size
        int target_size = ((harray*)ptr)->data_size;
        if(target_size != arr->data_size / arr->ele_count){
            return HFFI_STATE_FAILED;
        }
        //detach old and attach new
        __harray_detach_child_array(arr, index);
        __harray_attach_child_array(arr, index, (harray*)ptr, 1);
        //copy data
        void* data_ptr = arr->data + index * (arr->data_size / arr->ele_count);
        memcpy(data_ptr, ((harray*)ptr)->data, target_size);
        return HFFI_STATE_OK;
    }break;
    case HFFI_TYPE_STRUCT:{
        if(ptr == NULL || hffi_struct_get_data((struct hffi_struct*)ptr) == NULL){
            return HFFI_STATE_FAILED;
        }
        //check-data-size
        int target_size = hffi_struct_get_data_size((struct hffi_struct*)ptr);
        if(target_size != arr->data_size / arr->ele_count){
            return HFFI_STATE_FAILED;
        }
        //detach old and attach new
        __harray_detach_child_struct(arr, index);
        __harray_attach_child_struct(arr, index, (struct hffi_struct*)ptr, 1);
         //copy data
        void* data_ptr = arr->data + index * (arr->data_size / arr->ele_count);
        memcpy(data_ptr, hffi_struct_get_data((struct hffi_struct*)ptr), target_size);
        return HFFI_STATE_OK;
    }break;
    }
    return HFFI_STATE_FAILED;
}

int harray_eq(harray* arr, harray* arr2){
    if(arr == arr2){
        return HFFI_TRUE;
    }
    if(arr->hffi_t != arr2->hffi_t || arr->ele_count != arr2->ele_count
            || arr->data_size != arr2->data_size){
        return HFFI_STATE_FAILED;
    }
    if(!arr->data && !arr2->data){
        return HFFI_STATE_OK;
    }else if((!arr->data && arr2->data) || (arr->data && !arr2->data)){
        return HFFI_STATE_FAILED;
    }

    switch (arr->hffi_t) {
    case HFFI_TYPE_INT:
    case HFFI_TYPE_SINT8:
    case HFFI_TYPE_UINT8:
    case HFFI_TYPE_SINT16:
    case HFFI_TYPE_UINT16:
    case HFFI_TYPE_SINT32:
    case HFFI_TYPE_UINT32:
    case HFFI_TYPE_SINT64:
    case HFFI_TYPE_UINT64:
        if(memcmp(arr->data, arr2->data, arr->data_size) != 0){
            return HFFI_STATE_FAILED;
        }
        break;
    case HFFI_TYPE_FLOAT:
        for(int i = 0, c = arr->ele_count ; i < c ; i ++){
            if(!H_FLOAT_EQ(((float*)arr->data)[i], ((float*)arr2->data)[i])){
                return HFFI_STATE_FAILED;
            }
        }
        break;
    case HFFI_TYPE_DOUBLE:
        for(int i = 0, c = arr->ele_count ; i < c ; i ++){
            //printf("arr_eq double:  i = %d, v1 = %.5f, v2 = %.5f\n", i , ((float*)arr->data)[i], ((float*)arr2->data)[i]);
            if(!H_DOUBLE_EQ(((double*)arr->data)[i], ((double*)arr2->data)[i])){
                return HFFI_STATE_FAILED;
            }
        }
        break;
    case HFFI_TYPE_VOID:
        return HFFI_STATE_FAILED;

    case HFFI_TYPE_POINTER:
    case HFFI_TYPE_HARRAY:
    case HFFI_TYPE_STRUCT:
        if(memcmp(arr->data, arr2->data, arr->data_size) != 0){
            return HFFI_STATE_FAILED;
        }
        break;
    }
    if(arr->hffi_t == HFFI_TYPE_STRUCT_PTR || arr->hffi_t == HFFI_TYPE_HARRAY_PTR){
        for(int i = 0 ; i < arr->ele_count ; i++){
            if(arr->ele_list[i] == NULL){
                if(arr2->ele_list[i] != NULL){
                    return HFFI_STATE_FAILED;
                }
            }else{
                if(arr2->ele_list[i] == NULL){
                    return HFFI_STATE_FAILED;
                }
                if(arr->hffi_t == HFFI_TYPE_HARRAY_PTR){
                    if(harray_eq((harray*)arr->ele_list[i], (harray*)arr2->ele_list[i]) == HFFI_STATE_FAILED){
                        return HFFI_STATE_FAILED;
                    }
                }else if(arr->hffi_t == HFFI_TYPE_STRUCT_PTR){
                    if(hffi_struct_eq((struct hffi_struct*)arr->ele_list[i], (struct hffi_struct*)arr2->ele_list[i]) == HFFI_STATE_FAILED){
                        return HFFI_STATE_FAILED;
                    }
                }
            }
        }
    }
    return HFFI_STATE_OK;
}
void harray_sync_data_i(harray* arr, int index, void* child_ptr){
    if(!arr->data){
        return;
    }
    int every_size = arr->data_size / arr->ele_count;
    void* data_ptr = arr->data + index * every_size;
    int should_sync_parent = 0;
    switch (arr->hffi_t) {
    case HFFI_TYPE_STRUCT:{
        struct hffi_struct* hs = arr->ele_list[index];
        assert(hs == child_ptr);
        //1, sync target dara to self.
        memcpy(data_ptr, hffi_struct_get_data(hs), every_size);
        should_sync_parent = 1;
    }break;
    case HFFI_TYPE_HARRAY:{
        harray* hs = arr->ele_list[index];
        assert(hs == child_ptr);
        //1, sync target dara to self.
        memcpy(data_ptr, hs->data, every_size);
        should_sync_parent = 1;
    }break;
    }
    if(should_sync_parent) HFFI_SYNC_PARENT_I(arr)
}

void harray_sync_data(harray* arr, int reverse){
    if(!arr->data){
        return;
    }
    int every_size = arr->data_size / arr->ele_count;
    void* data_ptr;
    switch (arr->hffi_t) {
    case HFFI_TYPE_STRUCT:{
        for(int i = 0 ; i < arr->ele_count ; i ++){
            if(arr->ele_list[i]){
                data_ptr = arr->data + i * every_size;
                struct hffi_struct* hs = arr->ele_list[i];
                if(reverse){
                    //1, sync sub data. them sync to self.
                    hffi_struct_sync_data(hs, reverse);
                    memcpy(data_ptr, hffi_struct_get_data(hs), every_size);
                }else{
                    //1, sync self. 2 sync to sub data
                    memcpy(hffi_struct_get_data(hs), data_ptr, every_size);
                    hffi_struct_sync_data(hs, reverse);
                }
            }
        }
    }break;
    case HFFI_TYPE_HARRAY:{
        for(int i = 0 ; i < arr->ele_count ; i ++){
            if(arr->ele_list[i]){
                data_ptr = arr->data + i * every_size;
                harray* hs = arr->ele_list[i];
                if(reverse){
                    //1, sync sub data. them sync to self.
                    harray_sync_data(hs, reverse);
                    memcpy(data_ptr, hs->data, every_size);
                }else{
                    //1, sync self. 2 sync to sub data
                    memcpy(hs->data, data_ptr, every_size);
                    harray_sync_data(hs, reverse);
                }
            }
        }
    }break;
    }
}

#define harray_dump_impl(hffi_t, type, format)\
case hffi_t:{\
    for(int i = 0 ; i < arr->ele_count ; i ++){\
        if(i != 0){hstring_append(hs, ", ");}\
        hstring_appendf(hs, format, ((type*)arr->data)[i]);\
    }\
    hstring_append(hs, "]");\
    return;\
}break;

void harray_dump(harray* arr, struct hstring* hs){
#define __HARRAY_TYPE_STR(ffi_t, type)\
case ffi_t: hstring_append(hs, type); break;

    hstring_appendf(hs, "[ desc: count = %d, ele_size = %d, ele_type = ", arr->ele_count, arr->data_size / arr->ele_count);
    DEF_HFFI_SWITCH_ALL(__HARRAY_TYPE_STR, arr->hffi_t)
    hstring_append(hs, " ]\n data: [");

    DEF_HFFI_SWITCH_BASE_FORMAT(harray_dump_impl, arr->hffi_t)
    switch (arr->hffi_t) {
        case HFFI_TYPE_POINTER:{
            hstring_append(hs, "\npointer ");
        }break;
        case HFFI_TYPE_HARRAY:{
            hstring_append(hs, "\n<array> ");
            union harray_ele ele;
            for(int i = 0 ; i < arr->ele_count ; i ++){
                harray_geti(arr, i, &ele);
                if(ele._extra != NULL){
                    harray_dump((harray*)ele._extra, hs);
                }else{
                    hstring_append(hs, "null");
                }
                if(i != 0){
                    hstring_append(hs, ", ");
                }
            }
        }break;
    case HFFI_TYPE_HARRAY_PTR:{
            hstring_append(hs, "\n<array_ptr> ");
        union harray_ele ele;
        for(int i = 0 ; i < arr->ele_count ; i ++){
            harray_geti(arr, i, &ele);
            if(ele._extra != NULL){
                harray_dump((harray*)ele._extra, hs);
            }else{
                hstring_append(hs, "null");
            }
            if(i != 0){
                hstring_append(hs, ", ");
            }
        }
    }break;
    case HFFI_TYPE_STRUCT:{
        hstring_append(hs, "\n<struct> ");
        union harray_ele ele;
        for(int i = 0 ; i < arr->ele_count ; i ++){
            harray_geti(arr, i, &ele);
            if(ele._extra != NULL){
                hffi_struct_dump((struct hffi_struct*)ele._extra, hs);
            }else{
                hstring_append(hs, "null");
            }
            if(i != 0){
                hstring_append(hs, ", ");
            }
        }
    }break;
    case HFFI_TYPE_STRUCT_PTR:{
        hstring_append(hs, "\n<struct_ptr>");
        union harray_ele ele;
        for(int i = 0 ; i < arr->ele_count ; i ++){
            harray_geti(arr, i, &ele);
            if(ele._extra != NULL){
                hffi_struct_dump((struct hffi_struct*)ele._extra, hs);
            }else{
                hstring_append(hs, "null");
            }
            if(i != 0){
                hstring_append(hs, ", ");
            }
        }
    }break;
    }
    hstring_append(hs, "]");
}

int harray_set_struct_ptr(harray* arr, int index, struct hffi_struct* str){
    if(arr->hffi_t != HFFI_TYPE_STRUCT_PTR || index >= arr->ele_count){
        return HFFI_STATE_FAILED;
    }
    if(arr->ele_list[index]){
        hffi_delete_struct((struct hffi_struct*)arr->ele_list[index]);
    }
    arr->ele_list[index] = str;
    if(arr->data){
        ((void**)arr->data)[index] = str;
    }
    return HFFI_STATE_OK;
}
int harray_set_harray_ptr(harray* arr, int index, harray* str){
    if(arr->hffi_t != HFFI_TYPE_HARRAY_PTR || index >= arr->ele_count){
        return HFFI_STATE_FAILED;
    }
    if(arr->ele_list[index]){
        harray_delete((harray*)arr->ele_list[index]);
    }
    arr->ele_list[index] = str;
    if(arr->data){
        ((void**)arr->data)[index] = str;
    }
    return HFFI_STATE_OK;
}

//int* a = ...
//void* ptr = &a;
int harray_set_all(harray* arr, void* ptr){
    switch (arr->hffi_t) {

    case HFFI_TYPE_SINT8:
    case HFFI_TYPE_UINT8:
    case HFFI_TYPE_SINT16:
    case HFFI_TYPE_UINT16:
    case HFFI_TYPE_SINT32:
    case HFFI_TYPE_UINT32:
    case HFFI_TYPE_SINT64:
    case HFFI_TYPE_UINT64:
    case HFFI_TYPE_FLOAT:
    case HFFI_TYPE_DOUBLE:
    case HFFI_TYPE_INT:
    case HFFI_TYPE_HARRAY:
    case HFFI_TYPE_STRUCT:
    case HFFI_TYPE_POINTER:
    {
        if(arr->data){
            if(ptr){
                memcpy(arr->data, ptr, arr->data_size);
            }
        }else{
            //must ensure the data size match
            arr->data = ptr;
        }
        return HFFI_STATE_OK;
    }break;

    case HFFI_TYPE_HARRAY_PTR:{
        if(!arr->data){
            arr->data = MALLOC(sizeof (void*) * arr->ele_count);
            arr->free_data = 1;
        }
        void** data = arr->data;
        for(int i = 0 ; i< arr->ele_count ; i ++){
            if(arr->ele_list[i] == NULL){
                return HFFI_STATE_FAILED;
            }
            if(harray_set_all((harray*)arr->ele_list[i], ((void**)ptr)[i]) == HFFI_STATE_FAILED){
                return HFFI_STATE_FAILED;
            }
            //set pointer
            data[i] = ((harray*)arr->ele_list[i])->data;
        }
        return HFFI_STATE_OK;
    }break;

    case HFFI_TYPE_STRUCT_PTR:{
        if(!arr->data){
            arr->data = MALLOC(sizeof (void*) * arr->ele_count);
            arr->free_data = 1;
        }
        void** data = arr->data;
        for(int i = 0 ; i< arr->ele_count ; i ++){
            if(arr->ele_list[i] == NULL){
                return HFFI_STATE_FAILED;
            }
            if(hffi_struct_set_all((struct hffi_struct*)arr->ele_list[i], ((void**)ptr)[i]) == HFFI_STATE_FAILED){
               return HFFI_STATE_FAILED;
            }
            data[i] = hffi_struct_get_data((struct hffi_struct*)arr->ele_list[i]);
        }
        return HFFI_STATE_OK;
    }break;

    }
    return HFFI_STATE_FAILED;
}


