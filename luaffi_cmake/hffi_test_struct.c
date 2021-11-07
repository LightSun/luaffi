
#include <stdio.h>
#include "hffi.h"


typedef struct Test_struct1{
    float fVal;
    uint64 ulVal;
}Test_struct1;

Test_struct1 method_struct(Test_struct1 s){
    s.fVal = 1.25f;
    s.ulVal = 100;
    return s;
}

void hffi_test_struct(){
    printf("----------- start hffi_test_struct() ------------ \n");
    char _m[128];
    char* msg[1];
    msg[0] = _m;

    void* fn = method_struct;

    hffi_manager* m = hffi_new_manager();

    hffi_smtype* t_float = hffi_new_smtype(HFFI_TYPE_FLOAT, NULL);
    hffi_smtype* t_ul = hffi_new_smtype(HFFI_TYPE_UINT64, NULL);
    hffi_manager_add_smtype(m, t_float);
    hffi_manager_add_smtype(m, t_ul);

    hffi_smtype* arr[3];
    arr[0] = t_float;
    arr[1] = t_ul;
    arr[2] = NULL;
    hffi_struct* s_param = hffi_new_struct_simple(arr, msg);
    if(s_param == NULL){
        printf("%s\n", msg);
        goto failed;
    }
    hffi_manager_add_struct(m, s_param);
    hffi_struct* s_return = hffi_new_struct_simple(arr, msg);
    if(s_return == NULL){
        printf("%s\n", msg);
        goto failed;
    }
    hffi_manager_add_struct(m, s_return);

    hffi_value* param1 = hffi_new_value_struct(s_param);
    hffi_value* retVal = hffi_new_value_struct(s_return);
    hffi_manager_add_value(m, param1);
    hffi_manager_add_value(m, retVal);

    hffi_value* input[2];
    input[0] = param1;
    input[1] = NULL;
    if(hffi_call(fn, input, retVal, msg) == HFFI_STATE_OK){
        printf("test call 'Test_struct1 method_struct(Test_struct1 s)' success.\n");
        hffi_struct* ret_s = hffi_value_get_struct(retVal);
        Test_struct1* ts1 = ret_s->data;
        printf("result: fVal = %.2f, ulVal = %lld \n", ts1->fVal, ts1->ulVal);
    }else{
        printf("test call 'Test_struct1 method_struct(Test_struct1 s)' failed.\n");
        printf("%s\n", msg);
    }
    failed:
    hffi_delete_manager(m);
}
//----------------------- demo2 ----------------------------------
Test_struct1 method_struct2(Test_struct1* s){
    s->fVal = 1.25f;
    s->ulVal = 100;
    return *s;
}
void hffi_test_struct2(){
    printf("----------- start hffi_test_struct2() ------------ \n");
    char _m[128];
    char* msg[1];
    msg[0] = _m;

    void* fn = method_struct2;

    hffi_manager* m = hffi_new_manager();

    hffi_smtype* t_float = hffi_new_smtype(HFFI_TYPE_FLOAT, NULL);
    hffi_smtype* t_ul = hffi_new_smtype(HFFI_TYPE_UINT64, NULL);
    hffi_manager_add_smtype(m, t_float);
    hffi_manager_add_smtype(m, t_ul);

    hffi_smtype* arr[3];
    arr[0] = t_float;
    arr[1] = t_ul;
    arr[2] = NULL;
    hffi_struct* s_param = hffi_new_struct_simple(arr, msg);
    if(s_param == NULL){
        printf("%s\n", msg);
        goto failed;
    }
    hffi_manager_add_struct(m, s_param);
    hffi_struct* s_return = hffi_new_struct_simple(arr, msg);
    if(s_return == NULL){
        printf("%s\n", msg);
        goto failed;
    }
    hffi_manager_add_struct(m, s_return);

    hffi_value* param1 = hffi_new_value_struct_ptr(s_param);
    hffi_value* retVal = hffi_new_value_struct(s_return);
    hffi_manager_add_value(m, param1);
    hffi_manager_add_value(m, retVal);

    hffi_value* input[2];
    input[0] = param1;
    input[1] = NULL;
    if(hffi_call(fn, input, retVal, msg) == HFFI_STATE_OK){
        printf("test call 'Test_struct1 method_struct(Test_struct1* s)' success.\n");
        hffi_struct* param_s = hffi_value_get_struct(param1);
        Test_struct1* ps1 = ((Test_struct1*)param_s->data);
        printf("param: fVal = %.2f, ulVal = %lld \n", ps1->fVal, ps1->ulVal);

        hffi_struct* ret_s = hffi_value_get_struct(retVal);
        Test_struct1* ts1 = ret_s->data;
        printf("result: fVal = %.2f, ulVal = %lld \n", ts1->fVal, ts1->ulVal);
    }else{
        printf("test call 'Test_struct1 method_struct(Test_struct1* s)' failed.\n");
        printf("%s\n", msg);
    }
    failed:
    hffi_delete_manager(m);
}

//----------------------- demo3 ----------------------------------
Test_struct1* method_struct3(Test_struct1* s){
    s->fVal = 1.25f;
    s->ulVal = 100;
    return s;
}
void hffi_test_struct3(){
    printf("----------- start hffi_test_struct3() ------------ \n");
    char _m[128];
    char* msg[1];
    msg[0] = _m;

    void* fn = method_struct3;

    hffi_manager* m = hffi_new_manager();

    hffi_smtype* t_float = hffi_new_smtype(HFFI_TYPE_FLOAT, NULL);
    hffi_smtype* t_ul = hffi_new_smtype(HFFI_TYPE_UINT64, NULL);
    hffi_manager_add_smtype(m, t_float);
    hffi_manager_add_smtype(m, t_ul);

    hffi_smtype* arr[3];
    arr[0] = t_float;
    arr[1] = t_ul;
    arr[2] = NULL;
    hffi_struct* s_param = hffi_new_struct_simple(arr, msg);
    if(s_param == NULL){
        printf("%s\n", msg);
        goto failed;
    }
    hffi_manager_add_struct(m, s_param);
    hffi_struct* s_return = hffi_new_struct_simple(arr, msg);
    if(s_return == NULL){
        printf("%s\n", msg);
        goto failed;
    }
    hffi_manager_add_struct(m, s_return);

    hffi_value* param1 = hffi_new_value_struct_ptr(s_param);
    hffi_value* retVal = hffi_new_value_struct_ptr(s_return);
    hffi_manager_add_value(m, param1);
    hffi_manager_add_value(m, retVal);

    hffi_value* input[2];
    input[0] = param1;
    input[1] = NULL;
    if(hffi_call(fn, input, retVal, msg) == HFFI_STATE_OK){
        printf("test call 'Test_struct1* method_struct(Test_struct1* s)' success.\n");
        hffi_struct* param_s = hffi_value_get_struct(param1);
        Test_struct1* ps1 = ((Test_struct1*)param_s->data);
        printf("param: fVal = %.2f, ulVal = %lld \n", ps1->fVal, ps1->ulVal);

        hffi_struct* ret_s = hffi_value_get_struct(retVal);
        Test_struct1* ts1 = ret_s->data;
        printf("result: fVal = %.2f, ulVal = %lld \n", ts1->fVal, ts1->ulVal);
    }else{
        printf("test call 'Test_struct1* method_struct(Test_struct1* s)' failed.\n");
        printf("%s\n", msg);
    }
    failed:
    hffi_delete_manager(m);
}


