ffi.defines()

local hs = ffi.struct{
sint8, "a";
sint8; -- align
usint16,"b";
sint32,"c";
}

print("struct: ", tostring(hs))

ffi.undefines()