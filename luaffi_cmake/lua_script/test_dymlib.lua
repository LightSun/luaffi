--[[

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

-- test 1
local res = libtest["libtest_add_s8s32_s32"].call({ret = val_ret, val1, val2});
print("call method 'libtest_add_s8s32_s32' result: ", res)

-- test 2
val_ret = hffi.value(pointer, int);
local res = libtest["libtest_add_s8s32_s32p"].call({ret = val_ret, val1, val2});
print("call method 'libtest_add_s8s32_s32p' result: ", res)

-- test 3
val1 = hffi.value(pointer, sint8, 1)
val2 = hffi.value(pointer, int, 2)
val_ret = hffi.value(pointer, int);
local res = libtest["libtest_add_s8ps32p_s32p"].call({ret = val_ret, val1, val2});
print("call method 'libtest_add_s8ps32p_s32p' result: ", res)

-- test 4


hffi.undefines();