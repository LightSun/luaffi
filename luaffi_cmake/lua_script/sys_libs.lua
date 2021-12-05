
-- latter support other platforms.
local FFI_WIN64 = 1;
_c_runtime = hffi.loadLib("msvcrt.dll")

c_runtime = {};
local c_runtime_meta = {
__index = function(tab, index)
	local index_m = function (tab, ...)
		local _input = {...}
		_input[1]["abi"] = FFI_WIN64	
		return _c_runtime.index(table.unpack(_input))		
	end
	return index_m;
end,
__tostring = function(tab)
	return tostring(_c_runtime)
end
};
setmetatable(c_runtime, c_runtime_meta)