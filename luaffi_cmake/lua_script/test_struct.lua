ffi.defines()

local hs = ffi.struct{
sint8, "a";
sint8; -- align
uint16,"b";
sint32,"c";
}

hs[0]=1
hs[1]=2
hs[2]=3
hs[3]=4

print("struct: ", tostring(hs))
print("struct: ", tostring(hs.copy()))

ffi.undefines()