
#include <stdio.h>
#include "h_array.h"


//2*2
void arr_func_1(harray* arr){
    int** a = (int**)arr->data;
    printf("arr_func_1: a[0][0] = %d\n", a[0][0]);
    printf("arr_func_1: a[0][1] = %d\n", a[0][1]);
    printf("arr_func_1: a[1][0] = %d\n", a[1][0]);
    printf("arr_func_1: a[1][1] = %d\n", a[1][1]);
}

void test_harray(){
    printf("-------- test_harray ---------- \n");
    harray* arr1 = harray_new(HFFI_TYPE_SINT32, 2);
    harray* arr2 = harray_new(HFFI_TYPE_SINT32, 2);

    union harray_ele arr1_0;
    arr1_0._sint32 = 1;
    union harray_ele arr1_1;
    arr1_1._sint32 = 2;
    harray_seti(arr1, 0, &arr1_0);
    harray_seti(arr1, 1, &arr1_1);
    //
    union harray_ele arr2_0;
    arr2_0._sint32 = 3;
    union harray_ele arr2_1;
    arr2_1._sint32 = 4;
    harray_seti(arr2, 0, &arr2_0);
    harray_seti(arr2, 1, &arr2_1);

    //total
    union harray_ele tarr_0;
    tarr_0._extra = arr1;
    union harray_ele tarr_1;
    tarr_1._extra = arr2;
    harray* tarr = harray_new_array_ptr(2);
    harray_seti(tarr, 0, &tarr_0);
    harray_seti(tarr, 1, &tarr_1);

    union harray_ele tarr_r;
    harray_geti(tarr, 0, &tarr_r);
    assert(tarr_r._extra == arr1);
    harray_geti(tarr, 1, &tarr_r);
    assert(tarr_r._extra == arr2);
    assert(harray_geti(tarr, 2, &tarr_r) == HFFI_STATE_FAILED);

    arr_func_1(tarr);
    harray_delete(tarr);
    harray_delete(arr1);
    harray_delete(arr2);
}
