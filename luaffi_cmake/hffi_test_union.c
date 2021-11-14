
#include <stdio.h>
#include <inttypes.h>
#include "hffi.h"

/**
  web can mock union by struct in ffi.
 */

union Test_union{
    double a;
    int8_t b;
    uint64 c;
};

union Test_union test_union_i8(union Test_union u){
    u.b = 6;
    return u;
}
union Test_union test_union_double(union Test_union u){
    u.a = 6.22f;
    return u;
}
union Test_union test_union_uint64(union Test_union u){
    u.c = 1844674407370955161; // the max of uint64
    return u;
}

static void onPostInt8(hffi_value* ret){
    hffi_struct* s_ret = hffi_value_get_struct(ret);

    union Test_union* un_ret = (union Test_union*)s_ret->data;
    printf("onPostInt8 >>> int8_t val = %d\n", un_ret->b);
}
static void onPostDouble(hffi_value* ret){
    hffi_struct* s_ret = hffi_value_get_struct(ret);

    union Test_union* un_ret = (union Test_union*)s_ret->data;
    printf("onPostDouble >>> double val = %lf\n", un_ret->a);
}
static void onPostUint64(hffi_value* ret){
    hffi_struct* s_ret = hffi_value_get_struct(ret);

    union Test_union* un_ret = (union Test_union*)s_ret->data;
    //PRIu64(llu) for uint64. PRId64(ll)  for int64
    printf("onPostUint64 >>> uint64 val = %" PRIu64 "\n", un_ret->c);
}

void test_call_union1(int hffi_t,void* fn, void (*Func_post)(hffi_value*)){
    printf("----------- start test_call_union1() ------------ \n");
    char _m[128];
    char* msg[1];
    msg[0] = _m;

   // void* fn = test_union_double;

    hffi_manager* m = hffi_new_manager();

    hffi_smtype* t_double = hffi_new_smtype(hffi_t);
    hffi_manager_add_smtype(m, t_double);

    hffi_smtype* arr[2];
    arr[0] = t_double;
    arr[1] = NULL;
    //create struct to mock union
    hffi_struct* s_param = hffi_new_struct(arr, msg);
    if(s_param == NULL){
        goto failed;
    }
    hffi_manager_add_struct(m, s_param);
    hffi_struct* s_ret = hffi_new_struct(arr, msg);
    if(s_ret == NULL){
        goto failed;
    }
    hffi_manager_add_struct(m, s_ret);

    hffi_value* val_param = hffi_new_value_struct(s_param);
    hffi_value* val_ret = hffi_new_value_struct(s_ret);

    hffi_value* in[2];
    in[0] = val_param;
    in[1] = NULL;
    if(hffi_call(fn, in, val_ret, msg) == HFFI_STATE_OK){
        printf("success !!! \n");
        Func_post(val_ret);
//        hffi_struct* s_ret = hffi_value_get_struct(val_ret);
//        union Test_union* un_ret = (union Test_union*)s_ret->data;
//        printf("double val = %lf\n", un_ret->a);
    }else{
        printf("failed !!! \n");
    }
    failed:
    hffi_delete_manager(m);
}

void test_call_unions(){
    printf(">> int8 \n");
    test_call_union1(HFFI_TYPE_DOUBLE, test_union_i8, onPostInt8);
    printf(">> double \n");
    test_call_union1(HFFI_TYPE_DOUBLE, test_union_double, onPostDouble);
    printf(">> uint64 \n");
    test_call_union1(HFFI_TYPE_UINT64, test_union_uint64, onPostUint64);
}


