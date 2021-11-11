---
-- @function: 打印table的内容，递归
-- @param: tbl 要打印的table
-- @param: level 递归的层数，默认不用传值进来
-- @param: filteDefault 是否过滤打印构造函数，默认为是
-- @return: return
local function PrintTable( tbl , level, filteDefault)
  local msg = ""
  filteDefault = filteDefault or true --默认过滤关键字（DeleteMe, _class_type）
  level = level or 1
  local indent_str = ""
  for i = 1, level do
    indent_str = indent_str.."  "
  end

  print(indent_str .. "{")
  for k,v in pairs(tbl) do
    if filteDefault then
      if k ~= "_class_type" and k ~= "DeleteMe" then
        local item_str = string.format("%s%s = %s", indent_str .. " ",tostring(k), tostring(v))
        print(item_str)
        if type(v) == "table" then
          PrintTable(v, level + 1)
        end
      end
    else
      local item_str = string.format("%s%s = %s", indent_str .. " ",tostring(k), tostring(v))
      print(item_str)
      if type(v) == "table" then
        PrintTable(v, level + 1)
      end
    end
  end
  print(indent_str .. "}")
end

local arr = { a, "type"; b, "timestamp";};
PrintTable(arr);
print("#arr: ", #arr)

sint8 = "sint8"
print("sint8 = ",_G.sint8);

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