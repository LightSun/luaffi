--[[
typedef struct Libtest_struct1{
    float f;
    uint64 u64;
    sint16 arr[3];
    sint8 s8;
}Libtest_struct1;

int libtest_add_s8s32_s32(sint8 a, int b);

int* libtest_add_s8s32_s32p(sint8 a, int b);

int* libtest_add_s8ps32p_s32p(sint8* a, int* b);

float* libtest_add_farrfarr_farr(float a[3], float b[3]);

Libtest_struct1 libtest_struct_s_s(Libtest_struct1 s);
Libtest_struct1* libtest_struct_sp_sp(Libtest_struct1* s);

Libtest_struct1* libtest_struct_s_sp(Libtest_struct1 s);
]]--
hffi.defines();

local val1 = hffi.value(sint8, 1)
local val2 = hffi.value(int, 2)
local val_ret = hffi.value(int, 0)

local libtest = hffi.loadLib("libtest_hffi")

-- test 1. 'libtest_add_s8s32_s32'
local res = libtest.libtest_add_s8s32_s32({ret = val_ret, val1, val2});
print("call method 'libtest_add_s8s32_s32' result: ", res)
assert(res.addr() == val_ret.addr())
assert(val_ret == hffi.value(int, 3))

-- test 2. 'libtest_add_s8s32_s32p'
val_ret = hffi.value(pointer, int);
local res = libtest.libtest_add_s8s32_s32p({ret = val_ret, val1, val2});
print("call method 'libtest_add_s8s32_s32p' result: ", res)
assert(val_ret == hffi.value(pointer, int, 3))

-- test 3 , 'libtest_add_s8ps32p_s32p'
val1 = hffi.value(pointer, sint8, 1)
val2 = hffi.value(pointer, int, 2)
val_ret = hffi.value(pointer, int);
local res = libtest.libtest_add_s8ps32p_s32p({ret = val_ret, val1, val2});
print("call method 'libtest_add_s8ps32p_s32p' result: ", res)
assert(val_ret == hffi.value(pointer, int, 3))

-- test 4. 'libtest_add_farrfarr_farr'
local arr1 = hffi.array(float, {1.1,2.2, 3.3});
local arr2 = hffi.array(float, {11.1,12.2, 13.3});
local arr_ret = hffi.array(float, 3, true); -- create an array with no data.
local val_ret = hffi.value(arr_ret, true); -- create a value(array-pointer-type) which reference a float array. length is 3
local input_val1 = hffi.value(arr1);
local input_val2 = hffi.value(arr2);
local res = libtest.libtest_add_farrfarr_farr {ret = val_ret, input_val1, input_val2}
print("call method 'libtest_add_farrfarr_farr' result: ", res)
assert(res == hffi.value(hffi.array(float, {1.1 + 11.1, 2.2+12.2, 3.3+13.3}), true))

-- test 5, 'libtest_struct_s_s'
local arr = hffi.array(sint16, 3);
local input_struct = hffi.struct{
float,"f";
uint64,"u64";
arr,"arr";
sint8,"s8";
}
input_struct.f = 9.6
input_struct.u64 = 100
input_struct.arr.set(0, {10, 20, 30})
input_struct.s8 = 127

local out_struct = hffi.struct{
float,"f";
uint64,"u64";
hffi.array(sint16, 3),"arr";
sint8,"s8";
}

local res = libtest.libtest_struct_s_s {ret=out_struct, input_struct}
print("call method 'libtest_struct_s_s' result: ", res)
assert(out_struct == input_struct)

hffi.undefines();