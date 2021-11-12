
#include <stdio.h>
#include "h_array.h"
#include "atomic.h"
#include "h_alloctor.h"
#include "h_list.h"
#include "h_string.h"

typedef struct _Data{
    int index;
    void* _extra;
}_Data;

static inline _Data* new_data(int index, void* d){
    _Data* _d = MALLOC(sizeof (_Data));
    _d->index = index;
    _d->_extra = d;
    return _d;
}

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
    arr->ele_count = count;
    arr->data_size = count * every_size;
    arr->data = MALLOC(arr->data_size);
    arr->ref = 1;
    arr->hffi_t = HFFI_TYPE_STRUCT;
    arr->ele_list = array_list_new2(count * 4 / 3 + 1);
    //copy data
    int startPos = 0;
    struct hffi_struct* _item;
    for(int i = 0 ; i < count ; i ++){
        _item = array_list_get(structs, i);
        memcpy(arr->data + startPos, hffi_struct_get_data(_item), every_size);
        startPos += every_size;
        //ref to ele list
        hffi_struct_ref(_item, 1);
        array_list_add(arr->ele_list,_item);
    }
    return arr;
}
harray* harray_new_arrays(struct array_list* arrays){
    int count = array_list_size(arrays);
    if(count == 0) return NULL;
    int every_size = ((harray*)array_list_get(arrays, 0))->data_size;
    harray* arr = MALLOC(sizeof (harray));
    arr->ele_count = count;
    arr->data_size = count * every_size;
    arr->data = MALLOC(arr->data_size);
    arr->ref = 1;
    arr->hffi_t = HFFI_TYPE_HARRAY;
    arr->ele_list = NULL;
    //copy data
    int startPos = 0;
    harray* _item;
    for(int i = 0 ; i < count ; i ++){
        _item = array_list_get(arrays, i);
        memcpy(arr->data + startPos, _item->data, every_size);
        startPos += every_size;
        //ref to ele list
        harray_ref(_item, 1);
        array_list_add(arr->ele_list,_item);
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
    arr->ele_list = NULL;
    arr->hffi_t = hffi_t;
    arr->data = MALLOC(ele_size * count);
    arr->data_size = ele_size * count;
    arr->ele_count = count;
    arr->ref = 1;
    return arr;
}
//default as string
harray* harray_new_char(int count){
    harray* arr = harray_new(HFFI_TYPE_SINT8, count);
    ((char*)arr->data)[count - 1] = '\0';
    return arr;
}
static void __delete_ele(void* ud,void* ele){
    int hffi_t = *((int*)ud);
    _Data* _d  = ele;
    if(hffi_t == HFFI_TYPE_HARRAY || hffi_t == HFFI_TYPE_HARRAY_PTR){
        harray_delete((harray*)_d->_extra);
    }else if(hffi_t == HFFI_TYPE_STRUCT || hffi_t == HFFI_TYPE_STRUCT_PTR){
        hffi_delete_struct((struct hffi_struct*)_d->_extra);
    }
    FREE(_d);
}
void harray_delete(harray* arr){
    if(atomic_add(&arr->ref, -1) == 1){
        int hffi_t = arr->hffi_t;
        if(arr->ele_list){
            array_list_delete(arr->ele_list, __delete_ele, &hffi_t);
        }
        FREE(arr->data);
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

int __find_ele_by_index(void* ud,int size, int index,void* ele){
    int* id_ptr = ud;
    _Data* _d  = ele;
    return _d->index == *id_ptr ? 0 : -1 ;
}

int harray_geti(harray* arr, int index, union harray_ele* ptr){
    if(index >= arr->ele_count){
        return HFFI_STATE_FAILED;
    }
    switch (arr->hffi_t) {
        __GET_I(HFFI_TYPE_SINT8, sint8)
        __GET_I(HFFI_TYPE_UINT8, uint8)
        __GET_I(HFFI_TYPE_SINT16, sint16)
        __GET_I(HFFI_TYPE_UINT16, uint16)
        __GET_I(HFFI_TYPE_SINT32, sint32)
        __GET_I(HFFI_TYPE_UINT32, uint32)
        __GET_I(HFFI_TYPE_SINT64, sint64)
        __GET_I(HFFI_TYPE_UINT64, uint64)
        __GET_I(HFFI_TYPE_FLOAT, float)
        __GET_I(HFFI_TYPE_DOUBLE, double)
        __GET_I(HFFI_TYPE_INT, sint32)

        case HFFI_TYPE_STRUCT_PTR:
        case HFFI_TYPE_HARRAY_PTR:{
            if(arr->ele_list == NULL){
                return HFFI_STATE_FAILED;
            }
            void *data = array_list_find(arr->ele_list, __find_ele_by_index, &index);
            if(data){
                ptr->_extra = ((_Data*)data)->_extra;
                //printf("get harray: index = %d, arr_addr = %p\n", index, ptr->_extra);
                return HFFI_STATE_OK;
            }
        }break;

        case HFFI_TYPE_HARRAY:{
            //find old
            void *data =  array_list_find(arr->ele_list, __find_ele_by_index, &index);
            if(data){
                ptr->_extra = ((_Data*)data)->_extra;
                //must provide an array to copy daya.
                int target_size = ((harray*)ptr->_extra)->data_size;
                void* data_ptr = arr->data + index * (arr->data_size / arr->ele_count);
                memcpy(((harray*)ptr->_extra)->data, data_ptr, target_size);
                return HFFI_STATE_OK;
            }
        }break;

        case HFFI_TYPE_STRUCT:{
            //find old
            void *data = array_list_find(arr->ele_list, __find_ele_by_index, &index);
            if(data){
                ptr->_extra = ((_Data*)data)->_extra;
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

int harray_seti(harray* arr, int index, union harray_ele* ptr){
    //for harray or struct we need make data-connected
    if(index >= arr->ele_count){
        return HFFI_STATE_FAILED;
    }
    switch (arr->hffi_t) {
    __SET_I(HFFI_TYPE_SINT8, sint8)
    __SET_I(HFFI_TYPE_UINT8, uint8)
    __SET_I(HFFI_TYPE_SINT16, sint16)
    __SET_I(HFFI_TYPE_UINT16, uint16)
    __SET_I(HFFI_TYPE_SINT32, sint32)
    __SET_I(HFFI_TYPE_UINT32, uint32)
    __SET_I(HFFI_TYPE_SINT64, sint64)
    __SET_I(HFFI_TYPE_UINT64, uint64)

    __SET_I(HFFI_TYPE_FLOAT, float)
    __SET_I(HFFI_TYPE_DOUBLE, double)
    __SET_I(HFFI_TYPE_INT, sint32)

    case HFFI_TYPE_HARRAY_PTR:{
        if(ptr->_extra == NULL){
            return HFFI_STATE_FAILED;
        }
        //printf("put harray: index = %d, arr_addr = %p\n", index, ptr->_extra);
        //set data and ref
        harray_ref((harray*)ptr->_extra, 1);
        void** data = arr->data;
        data[index] = ((harray*)ptr->_extra)->data;
        //TODO multi thread ?
        if(arr->ele_list == NULL){
            arr->ele_list = array_list_new(4, 0.75);
        }
        array_list_add(arr->ele_list, new_data(index, ptr->_extra));
        return HFFI_STATE_OK;
    }break;
    case HFFI_TYPE_STRUCT_PTR:{
        if(ptr->_extra == NULL){
            return HFFI_STATE_FAILED;
        }
        hffi_struct_ref((struct hffi_struct*)ptr->_extra, 1);
        void** data = arr->data;
        data[index] = hffi_struct_get_data((struct hffi_struct*)ptr->_extra);
        if(arr->ele_list == NULL){
            arr->ele_list = array_list_new(4, 0.75);
        }
        array_list_add(arr->ele_list, new_data(index, ptr->_extra));
        return HFFI_STATE_OK;
    }break;

    case HFFI_TYPE_HARRAY:{
        if(ptr->_extra == NULL){
            return HFFI_STATE_FAILED;
        }
        //must provide an array to copy daya.
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
