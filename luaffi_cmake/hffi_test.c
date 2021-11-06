
#include <stdio.h>
#include "hffi.h"

int testFunc2_0(int** m, int n){
    printf("testFunc2_0 >>> params: %d\n", n);
    int* arr = (int*)malloc(sizeof(int));
    arr[0] = 1;
    *m = arr;
    return n;
}
int testFunc2_1(int* m, int n){
    printf("testFunc2_1 >>> params: %d\n", n);
    *m =1;
    return n;
}
void hffi_test2(){
    printf("----------- hffi_test2 ---------- \n");
    hffi_type* t1 = hffi_new_type(FFI_TYPE_POINTER, FFI_TYPE_SINT32, 1);
    hffi_type* t2 = hffi_new_type_base(FFI_TYPE_SINT32);
    hffi_value* val1 = hffi_new_value_ptr(t1);
    hffi_value* val2 = hffi_new_value_int(t2, 5);
    hffi_value* out = hffi_new_value_int(t2, 0);

    void* func = testFunc2_1;

    hffi_value* arr[3];
    arr[0] = val1;
    arr[1] = val2;
    arr[2] = NULL;
    char _m[128];
    char* msg[1];
    msg[0] = _m;
    int r = hffi_call(func, arr, out, msg);
    if(r == 0){
        int out_ptr;
        int out_r = hffi_value_get_int(out, &out_ptr);
        if(out_r == 0){
            int* ptr = val1->ptr;
            printf("hffi_call: out = %d\n", out_ptr);
            printf("**m: %d \n", (*ptr));
        }else{
            printf("hffi_call: get value out failed.\n");
        }
    }else{
        printf("hffi_call: failed. >>> %s\n", msg[0]);
    }
    hffi_delete_value(val1);
    hffi_delete_value(val2);
    hffi_delete_value(out);
    hffi_delete_type(t1);
    hffi_delete_type(t2);
}
void hffi_test1(){
    printf("----------- hffi_test1 ---------- \n");
    //printf("hffi_test1: sizeof(int**) = %d, sizeof(int64*) = %d\n", sizeof(int**), sizeof(sint64*));
    hffi_type* t1 = hffi_new_type(FFI_TYPE_POINTER, FFI_TYPE_SINT32, 2);
    hffi_type* t2 = hffi_new_type_base(FFI_TYPE_SINT32);
    hffi_value* val1 = hffi_new_value_ptr(t1);
    hffi_value* val2 = hffi_new_value_int(t2, 5);
    hffi_value* out = hffi_new_value_int(t2, 0);

    void* func = testFunc2_0;

    hffi_value* arr[3];
    arr[0] = val1;
    arr[1] = val2;
    arr[2] = NULL;
    char _m[128];
    char* msg[1];
    msg[0] = _m;
    int r = hffi_call(func, arr, out, msg);
    if(r == 0){
        int out_ptr;
        int out_r = hffi_value_get_int(out, &out_ptr);
        if(out_r == 0){
            int** ptr = val1->ptr;
            printf("hffi_call: out = %d\n", out_ptr);
            printf("**m: %d \n", (*ptr)[0]);
            free(*ptr);
        }else{
            printf("hffi_call: get value out failed.\n");
        }
    }else{
        printf("hffi_call: failed. >>> %s\n", msg[0]);
    }
    hffi_delete_value(val1);
    hffi_delete_value(val2);
    hffi_delete_value(out);
    hffi_delete_type(t1);
    hffi_delete_type(t2);
}
