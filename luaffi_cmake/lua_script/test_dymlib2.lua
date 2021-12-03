--[[
typedef struct Libtest_struct1{
    float f;
    uint64 u64;
    sint16 arr[3];
    sint8 s8;
}Libtest_struct1;

typedef int (*Test_cb1)(int a, int b);

typedef struct Libtest_struct2{
    double val;
    Test_cb1 cb;
    Libtest_struct1 str;
    Libtest_struct1* str_ptr;
}Libtest_struct2;

void libtest_closure_struct(Libtest_struct2* str);
int libtest_struct_closure(int a, int b, Libtest_struct2* str);
int libtest_closure_cb(int a, int b, Test_cb1 cb);
]]--
print("-------- start test_dymlib 2---------");
hffi.defines();

-- =================================================
local libtest = hffi.loadLib("libtest_hffi")

local function createClosure()
	local param1 = hffi.value(int, 1)
	local param2 = hffi.value(int, 2)
	local result = hffi.value(int, 0)

	local _ctx = "hello_closure"
	local cb = function (ctx, tab_param)
		assert(#tab_param == 2)
		assert(ctx == _ctx)
		print("param1 : ", tab_param[1])
		print("param2 : ", tab_param[2])
		return hffi.value(int, tab_param[1].get() + tab_param[2].get())
	end

	return hffi.closure({ret = result; ctx = _ctx; param1, param2}, cb);
end

local function createStruct2()
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
	input_struct.s8 = 10
	
	local arr2 = hffi.array(sint16, 3);
	local input_struct2 = hffi.struct{
	float,"f";
	uint64,"u64";
	arr2,"arr";
	sint8,"s8";
	}
	input_struct2.f = 9.6 * 10
	input_struct2.u64 = 100 * 10
	input_struct2.arr.set(0, {100, 200, 300})
	input_struct2.s8 = 10 * 10
	local clo = createClosure()
	
	local str2 = hffi.struct{
		double, "val";
		clo, "cb";
		input_struct, "_struct";
		input_struct2, true, "_struct";
	}	
	str2.val = 1.58;
	return str2;
end

local function test_closure_callback()
	local clo = createClosure();
	local val_closure = hffi.value(clo)

	--- test 'libtest_closure_cb'
	local res = libtest.libtest_closure_cb {ret = clo.ret; clo[0], clo[1], val_closure}
	print("test_closure_callback >>> libtest_closure_cb call result >>>", res)
	assert(res == hffi.value(int, 3))
end

local function test_closure_struct()
	local struct2 = createStruct2();
	libtest.libtest_closure_struct { struct2 }
	
	--
	local param1 = hffi.value(int, 1)
	local param2 = hffi.value(int, 2)
	local result = hffi.value(int, 0)
	local res = libtest.libtest_struct_closure {ret = result; param1, param2, struct2}
	print("test_closure_struct >>> libtest_struct_closure call result >>>", res)
	assert(res == hffi.value(int, 3))
end

test_closure_callback();
test_closure_struct();

-- ===============================================
hffi.undefines();
print("-------- end test_dymlib 2---------");