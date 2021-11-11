#ifndef H_ARRAY_H
#define H_ARRAY_H

#include "hffi_common.h"

struct array_list;
/**
  the base type array
  */
typedef struct harray{
    void* data;
    sint8 hffi_t;
    int data_size;
    int ele_count;  //element count
    int VOLATIE ref;
    struct array_list* ele_list; //only used for harray and struct
}harray;

union harray_ele{
    sint8 _sint8;
    uint8 _uint8;
    sint16 _sint16;
    uint16 _uint16;
    sint32 _sint32;
    uint32 _uint32;
    sint64 _sint64;
    uint64 _uint64;
    float _float;
    double _double;
    void* _extra;  //can be harray or struct.
};

harray* harray_new(sint8 hffi_t, int count);
/**
 * @brief harray_new_array: create an array. element is array-ptr.
 * @param count
 * @return
 */
harray* harray_new_array_ptr(int count);
harray* harray_new_struct_ptr(int count);
//the struct type must be the same
harray* harray_new_structs(struct array_list* structs);
//every array type must be the same
harray* harray_new_arrays(struct array_list* arrays);
/**
 * @brief harray_new_array: create an array. element is char.
 * @param count
 * @return
 */
harray* harray_new_char(int count);
void harray_delete(harray* arr);
int harray_get_count(harray* arr);
void harray_ref(harray* arr, int c);

int harray_geti(harray* arr, int index, union harray_ele* ptr);
int harray_seti(harray* arr, int index, union harray_ele* ptr);
//int harray_geti_lua(harray* arr, int index, struct lua_State* L);

#endif // H_ARRAY_H
