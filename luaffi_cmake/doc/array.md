
## array lua doc.

### what is array? what are the types which array support ?

### Create array
- 1, type with count. '(type, count [, no_data_flag]) '
	```
	local arr = ffi.array(byte, 3); // indicate create char array. which length is 3.
	```
- 2, type with values. '(type, tab ) ' 
- 3, values.  '(tab)'
- 4, string as char array. 'string'

### array methods.
- string index.
	* set
	* copy
	* eletype
	* elesize
- int index.
- int newindex.
- equals
- length
- tostring

