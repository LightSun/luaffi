local print=print
local ffi = require('ffi')

print("ffi.char: ", ffi.char)
local libc, div_t do
  local _ENV=ffi
  div_t = struct {
    sint, "quot",
    sint, "rem",
  }
  --libc.so
  libc = loadlib('advapi32.dll', {
   --[[
    printf = cif {ret = sint; pointer},
    div = cif {ret = div_t; sint, sint},
    memcpy = cif {ret = pointer; pointer, pointer, size_t},
    strcat = cif {ret = pointer; pointer, pointer},
    strlen = cif {ret = size_t; pointer},
	]]--
	GetFileAttributes = cif{ret=uint64; pointer},
  })
end

for k, v in pairs(libc) do
  print(k, v)
end

do
  local _ENV=libc
  local attr = GetFileAttributes("E:/Qt/Qt5.12.9/Tools/mingw730_64/bin/libbz2-1.dll");
  print("attr: ", attr);
  --[[
  local buf = ffi.alloc(ffi.char, 128)
  print(buf)

  memcpy(buf, "hello, ", 8)
  strcat(buf, "world")
  printf("strlen(\"%s\") == %d\n", buf, strlen(buf))

  local r = div(7654321, 1234567)
  printf("%d %d\n", r.quot, r.rem)
  ]]--
end

-- vim: ts=2:sw=2:et