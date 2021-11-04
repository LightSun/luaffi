
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <ffi.h>

#if defined(_MSC_VER)
#include <malloc.h>
#define alloca _alloca
#endif

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

#define HFFI_TYPE_VOID FFI_TYPE_VOID
#define HHFFI_TYPE_INT HFFI_TYPE_INT
#define HHFFI_TYPE_FLOAT HFFI_TYPE_FLOAT
#define HHFFI_TYPE_DOUBLE HFFI_TYPE_DOUBLE
#define HHFFI_TYPE_UINT8 FFI_TYPE_UINT8
#define HHFFI_TYPE_SINT8 FFI_TYPE_SINT8
#define HHFFI_TYPE_UINT16 FFI_TYPE_UINT16
#define HHFFI_TYPE_SINT16 FFI_TYPE_SINT16
#define HHFFI_TYPE_UINT32 FFI_TYPE_UINT32
#define HHFFI_TYPE_SINT32 FFI_TYPE_SINT32
#define HHFFI_TYPE_UINT64 FFI_TYPE_UINT64
#define HHFFI_TYPE_SINT64 FFI_TYPE_SINT64
#define HHFFI_TYPE_STRUCT FFI_TYPE_STRUCT
#define HHFFI_TYPE_POINTER FFI_TYPE_POINTER

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

typedef struct hffi_closure{
    ffi_closure* closure;
    void** code;             // function ptr ptr
    sint8 param_types[15];
    sint8 return_type;
    int param_count;
    int volatile ref;        // ref count
}hffi_closure;

typedef struct hffi_struct{
    ffi_type * data; //struct base
   // void* data;     //the data if struct.
    int count;
    int volatile ref;
}hffi_struct;

//error_msg can be null
extern ffi_type* to_ffi_type(int8_t ffi_t, char** error_msg);

hffi_type* hffi_new_type(int8_t ffi_type, int8_t pointer_type,uint8_t pointer_level);
hffi_type* hffi_new_type_base(int8_t ffi_type);
int hffi_type_size(hffi_type* val);
void hffi_delete_type(hffi_type* t);

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

/**
 * @brief hffi_new_closure: create closure with concat function proxy.
 * @param codeloc  often is the function-ptr
 * @param fun_proxy  the function proxy as function-ptr
 * @param p_types the parameter types of function-ptr
 * @param p_count the parameter count of function-ptr
 * @param return_type the return type of function-ptr
 * @param ud the user data pass to function-proxy
 * @param msg the output error msg. can be null.
 * @return the struct of closure. or null if failed.
 */
hffi_closure* hffi_new_closure(void* codeloc, void (*fun_proxy)(ffi_cif*,void* ret,void** args,void* ud),
                               sint8* p_types, int p_count, sint8 return_type, void* ud, char** msg);
void hffi_delete_closure(hffi_closure* c);

hffi_struct* hffi_new_struct(sint8* p_types, int p_count, char** msg);
void hffi_delete_struct(hffi_struct* c);
//void hffi_struct_getfield();

//load lib can like: https://github.com/chfoo/callfunc/blob/ef79a3985be728c42914320ddfc30f9e764f838e/src/c/callfunc.c

