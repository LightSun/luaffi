
#include "hffi.h"


typedef struct Test_struct1{
    float fVal;
    uint64 ulVal;
}Test_struct1;

Test_struct1 s_struct(Test_struct1 s){
    s.fVal = 1;
    s.ulVal = 100;
    return s;
}

void test_call_struct(){
    char _m[128];
    char* msg[1];
    msg[0] = _m;

    hffi_smtype* t_float = hffi_new_smtype(HFFI_TYPE_FLOAT, NULL);
    hffi_smtype* t_ul = hffi_new_smtype(HFFI_TYPE_UINT64, NULL);

    hffi_smtype* arr[2];
    arr[0] = t_float;
    arr[1] = t_ul;
    hffi_struct* s = hffi_new_struct_simple(arr, msg);
    hffi_value* val = hffi_new_value_struct(s);
}
