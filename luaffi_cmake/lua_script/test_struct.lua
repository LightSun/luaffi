hffi.defines()

local hs = hffi.struct{
sint8, "a";
sint8; -- align
uint16,"b";
sint32,"c";
}

hs[0]=1
hs[1]=2
hs[2]=3
hs[3]=4

print("struct 1: ", tostring(hs))
print("struct 2: ", tostring(hs.copy()))
print("hs.c = ", hs.c)
-- member name as newindex.
hs.a=10
print("struct 3: ", tostring(hs))
assert(hs[0] == 10)


hffi.undefines()