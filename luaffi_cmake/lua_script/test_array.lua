ffi.defines();

local arr;

arr = ffi.array(byte, 3);
arr[0]=1;
arr[1]=2;
arr[2]=3;
print("test array 1: ", tostring(arr));
assert(#arr == 3)


arr = ffi.array(int, {4, 5, 6});
print("test array 2: ", tostring(arr));

arr[1] = 100
assert(arr[1]==100)
arr[1] = {100, 101, 102}
assert(arr[1]==100)
assert(arr[2]==101)
arr.set(1, {5, 6})
assert(arr[1]==5)
assert(arr[2]==6)

print("gc: ",collectgarbage("count"))

ffi.undefines();