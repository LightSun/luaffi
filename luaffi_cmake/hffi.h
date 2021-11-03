
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <ffi.h>

#ifndef sint8
typedef signed char sint8;
#endif
#ifndef uint8
typedef unsigned char uint8;
#endif

#ifndef sint16
typedef signed short sint16;
#endif
#ifndef uint16
typedef unsigned short uint16;
#endif

#ifndef sint32
typedef signed int sint32;
#endif
#ifndef uint32
typedef unsigned int uint32;
#endif

#ifndef sint64
typedef signed long long sint64;
#endif
#ifndef uint64
typedef unsigned long long uint64;
#endif

#define hffi_new_value_auto_x_def(type) \
hffi_value* hffi_new_value_##type(hffi_type* t, type val);

#define hffi_get_value_auto_x_def(t)\
int hffi_get_value_##t(hffi_value* val, t* out_ptr);

typedef struct hffi_type{
    int8_t base_ffi_type;       //see ffi.h
    int8_t pointer_base_type;   //the raw base type. eg: int** -> int
    uint8_t pointer_level;      //the pointer level. eg: int** -> 2
    int volatile ref;
}hffi_type;

typedef struct hffi_value{
    hffi_type* type;
    int volatile ref;        //ref count
    void* ptr;
}hffi_value;

//error_msg can be null
extern ffi_type* to_ffi_type(int8_t ffi_t, char** error_msg);

hffi_type* hffi_new_type(int8_t ffi_type, int8_t pointer_type,uint8_t pointer_level);
hffi_type* hffi_new_type_base(int8_t ffi_type);
int hffi_type_size(hffi_type* val);
void hffi_free_type(hffi_type* t);

//only alloc base needed memory.
hffi_value* hffi_new_value_ptr(hffi_type* type);
//auto alloc memory for target size
hffi_value* hffi_new_value_auto(hffi_type* type, int size);
hffi_new_value_auto_x_def(sint8)
hffi_new_value_auto_x_def(sint16)
hffi_new_value_auto_x_def(sint32)
hffi_new_value_auto_x_def(sint64)
hffi_new_value_auto_x_def(uint8)
hffi_new_value_auto_x_def(uint16)
hffi_new_value_auto_x_def(uint32)
hffi_new_value_auto_x_def(uint64)
hffi_new_value_auto_x_def(int)
hffi_new_value_auto_x_def(float)
hffi_new_value_auto_x_def(double)

hffi_get_value_auto_x_def(sint8)
hffi_get_value_auto_x_def(sint16)
hffi_get_value_auto_x_def(sint32)
hffi_get_value_auto_x_def(sint64)
hffi_get_value_auto_x_def(uint8)
hffi_get_value_auto_x_def(uint16)
hffi_get_value_auto_x_def(uint32)
hffi_get_value_auto_x_def(uint64)
hffi_get_value_auto_x_def(int)
hffi_get_value_auto_x_def(float)
hffi_get_value_auto_x_def(double)

void hffi_delete_value(hffi_value* val);

//return 0 for success. 1 failed
int hffi_call(void (*fn)(void), hffi_value** in, int in_count,hffi_value* out, char** msg);




