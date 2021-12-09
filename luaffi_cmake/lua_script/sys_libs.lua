
-- latter support other platforms.
local FFI_WIN64 = 1;
local _c_runtime = hffi.loadLib("msvcrt.dll")

c_runtime = {};
local c_runtime_meta = {
__index = function(tab, index)
	--print("index = ", index)
	local index_m = function (...)
		local _input = {...}
		assert(type(_input[1])=='table')
		_input[1]["abi"] = FFI_WIN64	
		return hffi.call(_c_runtime, tostring(index), table.unpack(_input))
	end
	return index_m;
end,
__tostring = function(tab)
	return tostring(_c_runtime)
end
};
setmetatable(c_runtime, c_runtime_meta)


print("-------- start test syslib")
hffi.defines();

c_runtime.printf{var_count = 1; "test syslib >>> val = %d\n", hffi.value(int, 10086)}
local res = c_runtime.pow{ret = hffi.value(double, 0); hffi.value(double, 2), hffi.value(double, 3)}
print("pow(x, y): ",res)

hffi.undefines();
print("-------- end test syslib")