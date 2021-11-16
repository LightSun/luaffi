
#include "gtest/gtest.h"
extern "C"{
    #include "hffi.h"
}

extern "C" int hffi_test_value1(int argc,char **argv){
    testing::InitGoogleTest(&argc,argv);
    return RUN_ALL_TESTS();
}

TEST(testCase,test0){
    //EXPECT_EQ(add(2,3),5);
   hffi_value* val = hffi_new_value_int(5);
   int int_val = 0;
   EXPECT_EQ(hffi_value_get_base(val, &int_val), HFFI_STATE_OK);
   EXPECT_EQ(int_val, 5);
}
