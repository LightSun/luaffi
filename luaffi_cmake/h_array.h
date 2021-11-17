#ifndef H_ARRAY_H
#define H_ARRAY_H

#include "hffi_common.h"

struct array_list;
struct hstring;
/**
  the base type array
  */
typedef struct harray{
    void* data;
    sint8 hffi_t;
    sint8 free_data;
    int data_size;
    int ele_count;  //element count
    int VOLATIE ref;
    void** ele_list; //only used for harray* and hffi_struct*
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
harray* harray_new_chars(const char* str);

//create array as fix length. and copy str to it.
harray* harray_new_chars2(const char* str, int len);

//free_data: true to free data on recycle.
harray* harray_new_from_data(sint8 hffi_t, void* data, int data_size, int ele_count, sint8 free_data);

harray* harray_copy(harray* src);
void harray_delete(harray* arr);

int harray_get_count(harray* arr);
void harray_ref(harray* arr, int c);

int harray_geti(harray* arr, int index, union harray_ele* ptr);
int harray_seti(harray* arr, int index, union harray_ele* ptr);
int harray_seti2(harray* arr, int index, void* ptr);

int harray_eq(harray* arr, harray* arr2);

void harray_dump(harray* arr, struct hstring* hs);

#endif // H_ARRAY_H
