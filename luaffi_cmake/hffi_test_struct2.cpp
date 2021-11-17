
#include "gtest/gtest.h"
extern "C"{
    #include "hffi.h"
    #include "h_array.h"
    #include "h_list.h"
}

struct Test_struct2{
    sint16 a;
    void* b;
    char c[3];
};

Test_struct2* func1(Test_struct2 ts){
    Test_struct2* out = (Test_struct2*)malloc(sizeof (struct Test_struct2));
    out->a = ts.a;
    out->b = &out->a;
    out->c[0] = ts.c[0] * 10;
    out->c[1] = ts.c[1] * 10;
    out->c[2] = ts.c[2] * 10;
    return out;
}

TEST(testCase,test_struct1){

    harray* arr = harray_new_char(3);
    int ival = 1;
    harray_seti2(arr, 0, &ival);
    ival = 2;
    harray_seti2(arr, 1, &ival);
    ival = 3;
    harray_seti2(arr, 2, &ival);
    harray* arr2 = harray_copy(arr);

    array_list* list = array_list_new2(6);
    hffi_smtype* t1 = hffi_new_smtype(HFFI_TYPE_SINT16);
    hffi_smtype* t2 = hffi_new_smtype(HFFI_TYPE_POINTER);
    hffi_smtype* t3 = hffi_new_smtype_harray(arr);
    array_list_add(list, t1);
    array_list_add(list, t2);
    array_list_add(list, t3);

    hffi_struct* hs = hffi_new_struct_from_list(list, NULL);
    sint16 hs_val1 = 5;
    EXPECT_EQ(hffi_struct_set_base(hs, 0, 0, &hs_val1), HFFI_STATE_OK);
    //build func desc
    hffi_struct* hs_ret = hffi_new_struct_from_list_no_data(FFI_DEFAULT_ABI, list, NULL);
    hffi_fn fn = (hffi_fn)func1;
    hffi_value* p1 = hffi_new_value_struct(hs);
    hffi_value* ret = hffi_new_value_struct_ptr(hs_ret);

    hffi_value* in_vals[2];
    in_vals[0] = p1;
    in_vals[1] = NULL;
    EXPECT_EQ(hffi_call(fn, in_vals, ret, NULL), HFFI_STATE_OK);
    EXPECT_EQ(hffi_value_get_struct(ret), hs_ret);
    //return value.
    sint16 ret_hs_val1 = 0;
    EXPECT_EQ(hffi_struct_get_base(hs_ret, 0, 0, &ret_hs_val1), HFFI_STATE_OK);
    EXPECT_EQ(ret_hs_val1, hs_val1);
    EXPECT_EQ(hffi_struct_get_harray(hs_ret, 1), arr);
    //TODO EXPECT_EQ(hffi_struct_get_harray(hs_ret, 1), arr);

    hffi_delete_value(ret);
    hffi_delete_value(p1);
    hffi_delete_struct(hs_ret);
    hffi_delete_struct(hs);
    harray_delete(arr);
    harray_delete(arr2);
    array_list_delete2(list, NULL);
}
