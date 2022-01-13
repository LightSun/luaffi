
local m = require("core.util.strings")

local a = "a/n/c.d"
assert(m.endsWidth(a, "c.d"));
assert(m.endsWidth(a, "n/c.d"));
assert(not m.endsWidth(a, "n"));
assert(m.startsWidth(a, "a"))
assert(m.startsWidth(a, "a/n"))
assert(m.startsWidth(a, "a/n/c"))
assert(not m.startsWidth(a, "n/c"))


--[[print(string.gsub(" 1 2 3 ", " ", ""))
print(string.gsub(" 1 2 3 ", " ", "", 1))
print(string.gsub(" 1 2 3 ", " ", "", 2))]]


local str = "1234,389, abc 6767,888";
local list = m.split(str, ",");
for i = 1, #list
do
    str = string.format("index %d: value = %s", i, list[i]);
    print(str);
end