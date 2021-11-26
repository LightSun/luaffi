
### TODO
- 1， permit cast value - ptr(from hffi_value like int**) to direct array? ok
- 2, hffi_value for lua not done. ok
- 3, hffi_struct for lua not done. 
- 4, dym_func for lua not done.  ok
- 5, hffi_cif for lua not done. ok
- 6, struct support closure as member type.
- 7, harray and struct as member when to sync data?
- N, opt memory of h_string


### read me
- ffi: struct must use 'ffi_get_struct_offsets' to align.
- libffi 对于ffi_cif 和call 似乎有限制。 比如Lua创建一个对象ffi_cif并且准备好. 但是当调用后发现得不到正确的结果。
  纯c调用是正常的。lua绑定层是没得问题的。eg:
  ``` C
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
  ```
```

local val1 = hffi.value(sint8, 1)
local val2 = hffi.value(int, 2)
local val_ret = hffi.value(int, 0)

print("addr_val1 = ", val1.addr())
print("addr_val2 = ", val2.addr())
print("addr_val3 = ", val_ret.addr())
print("val_ret = ", val_ret)

local cif1 = hffi.cif{ret=val_ret, val1, val2}
local res = libtest["libtest_add_s8s32_s32"].call(cif1)
-- res is wrong.
```  	