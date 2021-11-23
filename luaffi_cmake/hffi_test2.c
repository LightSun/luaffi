
#include <stdio.h>
#include "hffi.h"
#include "dym_loader.h"

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
