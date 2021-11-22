
### hffi object types.
- value: represent a type with its' value. 
- smtype: represent a member type of struct. 'struct member type'
- struct: represent a struct of C.
- array: represet c array.
- closure: represet function callback of c.
- cif: represet ffi cif. which used to prepare and call c funtion from lib.
- dymlib: represet a dynamic lib. 
- dymfunc: represet a funtion of lib

### hffi types represent c types. 
- bases and its' ptr. : sint8/uint8/sint16/uint16/sint32/uint32/sint64/uint64/float/double.
	*alias: bool = byte = sint8, short = sint16, int = sint32, uint = uint32. long = sint64
	
- array and array-ptr.
- struct and struct-ptr.
- closure as function callback.