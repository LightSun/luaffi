
#include <stdio.h>
#include "hffi.h"
#include "dym_loader.h"
#include "h_float_bits.h"

int test_add_s8s32_s32(sint8 a, int b){
    return a + b;
}

void test_dymlib(){
    printf("-------- test_dymlib -------\n");
    dym_lib* lib = dym_new_lib("libtest_hffi");
    dym_func* func = dym_lib_get_function(lib, "libtest_add_s8s32_s32", 1);

    int int_val1 = 1;
    int int_val2 = 2;
    int int_val3 = 0;
    hffi_value* val1 = hffi_new_value_raw_type2(HFFI_TYPE_SINT8, &int_val1);
    hffi_value* val2 = hffi_new_value_raw_type2(HFFI_TYPE_INT, &int_val2);
    hffi_value* val_ret = hffi_new_value_raw_type2(HFFI_TYPE_INT, &int_val3);
    array_list* list = array_list_new2(4);
    array_list_add(list, val1);
    array_list_add(list, val2);
    //as libffi has some limit of use ffi_cif.
    hffi_cif* cif = hffi_new_cif(FFI_DEFAULT_ABI, list, 0, val_ret, NULL);
    hffi_cif_call(cif, func->func_ptr);

    int out = 0;
    hffi_value_get_int(val_ret, &out);
    printf("test_dymlib result: %d\n", out);

    hffi_value_get_base(cif->out, &out);
    printf("test_dymlib result 2: %d\n", out);

    hffi_delete_cif(cif);
    hffi_delete_value(val_ret);
    array_list_delete2(list, list_travel_value_delete);
    dym_delete_lib(lib);
    dym_delete_func(func);
}

void hffi_test_closure();
static void test_hffi_closure2();

typedef float (*Func_calCircleArea)(float*);

static float __hffi_call_cb(float* a, Func_calCircleArea cb){
    float val = cb(a);
    return val;
}

static void __hffi_calCircleArea(ffi_cif * cif,
                  void *ret,
                  void **args,
                  void * ud) {
    //input: float*
    //args[0]: float** --- float*
    float pi = 3.14;
    float r = **(float **)args[0];
    float area = pi * r * r;
    *(float*)ret = area;
    printf("__hffi_calCircleArea >>> ud = %d,area:%.2f,  *ret = %.2f\n",*(int*)ud,area, *(float*)ret);
}

void test_hffi_closure(){
    printf("-------- hffi_test_closure -------- \n");
    int ud = 10086;
    ffi_cif cif;
    void *code;
    ffi_closure * pcl = ffi_closure_alloc(sizeof(ffi_closure), &code);

    ffi_type* types[6];
    types[0] = &ffi_type_pointer;
    types[1] = NULL;
    if(ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &ffi_type_float, types) != FFI_OK){
        abort();
    }
    if(ffi_prep_closure_loc(pcl, &cif, __hffi_calCircleArea, &ud, code) != FFI_OK){
        abort();
    }
    float val = 10;
    float result;

    result = ((Func_calCircleArea)code)(&val);
    assert(H_FLOAT_EQ(result, 3.14 * val * val));
    val = 5;
    result = __hffi_call_cb(&val, (Func_calCircleArea)code);
    assert(H_FLOAT_EQ(result, 3.14 * val * val));

    ffi_closure_free(pcl);
    printf("-------- end hffi_test_closure -------- \n");

    test_hffi_closure2();
}

static void test_hffi_closure2(){
    printf("-------- hffi_test_closure2 -------- \n");
    array_list* list = array_list_new2(4);
    float val = 10;
    hffi_value* hv_input = hffi_new_value_ptr2(HFFI_TYPE_FLOAT, &val);
    hffi_value* hv_ret = hffi_new_value_float(0);
    array_list_add(list, hv_input);
    int ud = 10086;
    hffi_closure* closure = hffi_new_closure(FFI_DEFAULT_ABI, __hffi_calCircleArea, list, hv_ret, &ud, NULL);
    //hffi_closure_prepare(closure, &code);

    float result;
    result = ((Func_calCircleArea)closure->func_ptr)(&val);
    assert(H_FLOAT_EQ(result, 3.14 * val * val));
    val = 5;
    result = ((Func_calCircleArea)closure->func_ptr)(&val);
    assert(H_FLOAT_EQ(result, 3.14 * val * val));

    result = __hffi_call_cb(&val, (Func_calCircleArea)closure->func_ptr);
    assert(H_FLOAT_EQ(result, 3.14 * val * val));
    //post execute
    array_list_delete2(list, list_travel_value_delete);
    hffi_delete_value(hv_ret);
    hffi_delete_closure(closure);

    printf("-------- end hffi_test_closure2 -------- \n");
}
