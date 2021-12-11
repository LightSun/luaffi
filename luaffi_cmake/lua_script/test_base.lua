
print("E = ", string.byte("E"))

print("1 | 2 = ", bit32.bor(1, 2))

print("1 | 2 | 4 = ", bit32.bor(1, 2, 4))

print("1 | 2 | 4 | 8 = ", bit32.bor(1, 2, 4, 8))


local ints = require("ints")

print("ints.shift_bor('EOF ') = ",ints.shift_bor("EOF "))
