
#include "gtest/gtest.h"
extern "C"{
    #include "hffi.h"
    #include "h_array.h"
}

extern "C" int hffi_test_value1(int argc,char **argv){
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}

TEST(testCase,test0){
    //EXPECT_EQ(add(2,3),5);
    //base val
   hffi_value* val = hffi_new_value_int(5);
   int int_val = 0;
   EXPECT_EQ(hffi_value_get_base(val, &int_val), HFFI_STATE_OK);
   EXPECT_EQ(int_val, 5);

   int_val= 7;
   EXPECT_EQ(hffi_value_set_base(val, &int_val), HFFI_STATE_OK);
   int_val = 8;
   EXPECT_EQ(hffi_value_get_base(val, &int_val), HFFI_STATE_OK);
   EXPECT_EQ(int_val, 7);
   hffi_delete_value(val);
   //base ptr
   val = hffi_new_value_ptr(HFFI_TYPE_INT);
   EXPECT_EQ(hffi_value_set_base(val, &int_val), HFFI_STATE_OK);
   int_val = 8;
   EXPECT_EQ(hffi_value_get_base(val, &int_val), HFFI_STATE_OK);
   EXPECT_EQ(int_val, 7);
   hffi_delete_value(val);
}
TEST(testCase,test_as_array){
    //mazy.
    hffi_value* val;
    val = hffi_new_value_ptr_no_data(HFFI_TYPE_INT);
    int* _intr = (int*)malloc(sizeof(int) *6);
    _intr[0] = 10;
    _intr[1] = 11;
    _intr[2] = 12;
    _intr[3] = 13;
    _intr[4] = 14;
    _intr[5] = 15;
    val->ptr = _intr;
    harray* arr = hffi_value_get_pointer_as_array(val, 6, 0, 1, 1);
    EXPECT_NE(arr, nullptr);
    EXPECT_EQ(arr->ele_count, 6);
    union harray_ele ele;
    EXPECT_EQ(harray_geti(arr, 4, &ele), HFFI_STATE_OK);
    EXPECT_EQ(ele._sint32, 14);
    EXPECT_EQ(harray_geti(arr, 5, &ele), HFFI_STATE_OK);
    EXPECT_EQ(ele._sint32, 15);


    hffi_delete_value(val);
    harray_delete(arr);
}
TEST(testCase,test_as_array2){
    //mazy.
    hffi_value* val;
    val = hffi_new_value_ptr_no_data(HFFI_TYPE_INT);
    int* _intr = (int*)malloc(sizeof(int) *6);
    _intr[0] = 10;
    _intr[1] = 11;
    _intr[2] = 12;
    _intr[3] = 13;
    _intr[4] = 14;
    _intr[5] = 15;
    val->ptr = _intr;
    harray* arr = hffi_value_get_pointer_as_array(val, 2, 3, 1, 1);//2
    EXPECT_NE(arr, nullptr);
    EXPECT_EQ(arr->ele_count, 2);
    //for continue mem. parent data must be ptr. sub is use shared data.
    EXPECT_EQ(arr->hffi_t, HFFI_TYPE_HARRAY_PTR);

    union harray_ele ele;
    EXPECT_EQ(harray_geti(arr, 2, &ele), HFFI_STATE_FAILED);
    EXPECT_EQ(harray_geti(arr, 1, &ele), HFFI_STATE_OK);
    EXPECT_NE(ele._extra, nullptr);

    harray* sub_arr = (harray*)ele._extra;
    EXPECT_EQ(sub_arr->hffi_t, HFFI_TYPE_INT);
    EXPECT_EQ(sub_arr->ele_count, 3);
    EXPECT_EQ(harray_geti(sub_arr, 0, &ele), HFFI_STATE_OK);
    EXPECT_EQ(ele._sint32, 13);
    EXPECT_EQ(harray_geti(sub_arr, 1, &ele), HFFI_STATE_OK);
    EXPECT_EQ(ele._sint32, 14);
    EXPECT_EQ(harray_geti(sub_arr, 2, &ele), HFFI_STATE_OK);
    EXPECT_EQ(ele._sint32, 15);

    hffi_delete_value(val);
    harray_delete(arr);
}

TEST(testCase,test_as_array3){
    hffi_value* val;
    val = hffi_new_value_ptr_no_data(HFFI_TYPE_INT);
    void** data = (void**)malloc(sizeof(void*) * 2);
    int* intr1 = (int*)malloc(sizeof(int) *3);
    int* intr2 = (int*)malloc(sizeof(int) *3);
    data[0]=intr1;
    data[1]=intr2;

    intr1[0] = 10;
    intr1[1] = 11;
    intr1[2] = 12;

    intr2[0] = 13;
    intr2[1] = 14;
    intr2[2] = 15;
    val->ptr = data;
    //memory is not continue, and share data with 'val'.
    harray* arr = hffi_value_get_pointer_as_array(val, 2, 3, 0, 1);//2
    EXPECT_NE(arr, nullptr);
    EXPECT_EQ(arr->ele_count, 2);
    //for continue mem. parent data must be ptr. sub is use shared data.
    EXPECT_EQ(arr->hffi_t, HFFI_TYPE_HARRAY_PTR);

    union harray_ele ele;
    EXPECT_EQ(harray_geti(arr, 2, &ele), HFFI_STATE_FAILED);
    EXPECT_EQ(harray_geti(arr, 1, &ele), HFFI_STATE_OK);
    EXPECT_NE(ele._extra, nullptr);

    harray* sub_arr = (harray*)ele._extra;
    EXPECT_EQ(sub_arr->hffi_t, HFFI_TYPE_INT);
    EXPECT_EQ(sub_arr->ele_count, 3);
    EXPECT_EQ(harray_geti(sub_arr, 0, &ele), HFFI_STATE_OK);
    EXPECT_EQ(ele._sint32, 13);
    EXPECT_EQ(harray_geti(sub_arr, 1, &ele), HFFI_STATE_OK);
    EXPECT_EQ(ele._sint32, 14);
    EXPECT_EQ(harray_geti(sub_arr, 2, &ele), HFFI_STATE_OK);
    EXPECT_EQ(ele._sint32, 15);

    hffi_delete_value(val);
    harray_delete(arr);
}

TEST(testCase,test_as_array4){
    hffi_value* val;
    val = hffi_new_value_ptr_no_data(HFFI_TYPE_INT);
    void** data = (void**)malloc(sizeof(void*) * 2);
    int* intr1 = (int*)malloc(sizeof(int) *3);
    int* intr2 = (int*)malloc(sizeof(int) *3);
    data[0]=intr1;
    data[1]=intr2;

    intr1[0] = 10;
    intr1[1] = 11;
    intr1[2] = 12;

    intr2[0] = 13;
    intr2[1] = 14;
    intr2[2] = 15;
    val->ptr = data;
    //memory is not continue, and not share data with 'val'.
    harray* arr = hffi_value_get_pointer_as_array(val, 2, 3, 0, 0);//2
    EXPECT_NE(arr, nullptr);
    EXPECT_EQ(arr->ele_count, 2);
    //for continue mem. parent data must be ptr. sub is use shared data.
    EXPECT_EQ(arr->hffi_t, HFFI_TYPE_HARRAY_PTR);

    union harray_ele ele;
    EXPECT_EQ(harray_geti(arr, 2, &ele), HFFI_STATE_FAILED);
    EXPECT_EQ(harray_geti(arr, 1, &ele), HFFI_STATE_OK);
    EXPECT_NE(ele._extra, nullptr);

    harray* sub_arr = (harray*)ele._extra;
    EXPECT_EQ(sub_arr->hffi_t, HFFI_TYPE_INT);
    EXPECT_EQ(sub_arr->ele_count, 3);
    EXPECT_EQ(harray_geti(sub_arr, 0, &ele), HFFI_STATE_OK);
    EXPECT_EQ(ele._sint32, 13);
    EXPECT_EQ(harray_geti(sub_arr, 1, &ele), HFFI_STATE_OK);
    EXPECT_EQ(ele._sint32, 14);
    EXPECT_EQ(harray_geti(sub_arr, 2, &ele), HFFI_STATE_OK);
    EXPECT_EQ(ele._sint32, 15);

    hffi_delete_value(val);
    harray_delete(arr);
}
