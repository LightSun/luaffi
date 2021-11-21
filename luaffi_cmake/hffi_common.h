#ifndef HFFI_COMMON_H
#define HFFI_COMMON_H

#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <ffi.h>

#if defined(_MSC_VER)
#include <malloc.h>
#define alloca _alloca
#endif

#define VOLATIE volatile

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

typedef void (*hffi_fn)(void);
#define HFFI_STRUCT_NO_PARENT -1
#define HFFI_STRUCT_NO_DATA -2

#define HFFI_STATE_OK 0
#define HFFI_STATE_FAILED -1
#define HFFI_TRUE HFFI_STATE_OK

#define HFFI_TYPE_VOID FFI_TYPE_VOID
#define HFFI_TYPE_INT FFI_TYPE_INT
#define HFFI_TYPE_FLOAT FFI_TYPE_FLOAT
#define HFFI_TYPE_DOUBLE FFI_TYPE_DOUBLE
#define HFFI_TYPE_UINT8 FFI_TYPE_UINT8
#define HFFI_TYPE_SINT8 FFI_TYPE_SINT8
#define HFFI_TYPE_UINT16 FFI_TYPE_UINT16
#define HFFI_TYPE_SINT16 FFI_TYPE_SINT16
#define HFFI_TYPE_UINT32 FFI_TYPE_UINT32
#define HFFI_TYPE_SINT32 FFI_TYPE_SINT32
#define HFFI_TYPE_UINT64 FFI_TYPE_UINT64
#define HFFI_TYPE_SINT64 FFI_TYPE_SINT64
#define HFFI_TYPE_STRUCT FFI_TYPE_STRUCT
#define HFFI_TYPE_POINTER FFI_TYPE_POINTER

//#define HFFI_TYPE_STRING (FFI_TYPE_LAST + 1)
#define HFFI_TYPE_HARRAY (FFI_TYPE_LAST + 2)
#define HFFI_TYPE_HARRAY_PTR (FFI_TYPE_LAST + 3)
#define HFFI_TYPE_STRUCT_PTR (FFI_TYPE_LAST + 4)
#define HFFI_TYPE_CLOSURE (FFI_TYPE_LAST + 5)

#define H_UNSED(x) (void)x;
#define H_UNSED2(x,y) H_UNSED(x) H_UNSED(y)

#define DEF_HFFI_BASE_SWITCH(macro, ffi_t)\
switch (ffi_t) {\
macro(HFFI_TYPE_SINT8, sint8)\
macro(HFFI_TYPE_UINT8, uint8)\
macro(HFFI_TYPE_SINT16, sint16)\
macro(HFFI_TYPE_UINT16, uint16)\
macro(HFFI_TYPE_SINT32, sint32)\
macro(HFFI_TYPE_UINT32, uint32)\
macro(HFFI_TYPE_SINT64, sint64)\
macro(HFFI_TYPE_UINT64, uint64)\
macro(HFFI_TYPE_FLOAT, float)\
macro(HFFI_TYPE_DOUBLE, double)\
macro(HFFI_TYPE_INT, sint32)\
}

#define DEF_HFFI_SWITCH_BASE_FORMAT(macro, ffi_t)\
switch (ffi_t) {\
macro(HFFI_TYPE_SINT8, sint8, "%d")\
macro(HFFI_TYPE_UINT8, uint8, "%d")\
macro(HFFI_TYPE_SINT16, sint16, "%d")\
macro(HFFI_TYPE_UINT16, uint16, "%u")\
macro(HFFI_TYPE_SINT32, sint32, "%d")\
macro(HFFI_TYPE_UINT32, uint32, "%u")\
macro(HFFI_TYPE_SINT64, sint64, "%lld")\
macro(HFFI_TYPE_UINT64, uint64, "%llu")\
macro(HFFI_TYPE_INT, sint32, "%d")\
macro(HFFI_TYPE_FLOAT, float, "%g")\
macro(HFFI_TYPE_DOUBLE, double, "%g")\
}

#define DEF_HFFI_SWITCH_ALL(macro, ffi_t)\
switch (ffi_t) {\
macro(HFFI_TYPE_SINT8, "sint8")\
macro(HFFI_TYPE_UINT8, "uint8")\
macro(HFFI_TYPE_SINT16, "sint16")\
macro(HFFI_TYPE_UINT16, "uint16")\
macro(HFFI_TYPE_SINT32, "sint32")\
macro(HFFI_TYPE_UINT32, "uint32")\
macro(HFFI_TYPE_SINT64, "sint64")\
macro(HFFI_TYPE_UINT64, "uint64")\
macro(HFFI_TYPE_FLOAT, "float")\
macro(HFFI_TYPE_DOUBLE, "double")\
macro(HFFI_TYPE_INT, "sint32")\
macro(HFFI_TYPE_VOID, "void")\
macro(HFFI_TYPE_HARRAY, "<array>")\
macro(HFFI_TYPE_HARRAY_PTR, "<array_ptr>")\
macro(HFFI_TYPE_STRUCT, "<struct>")\
macro(HFFI_TYPE_STRUCT_PTR, "<struct_ptr>")\
macro(HFFI_TYPE_CLOSURE, "<closure>")\
}

struct hffi_struct;
struct hstring;

void hffi_delete_struct(struct hffi_struct* c);

void* hffi_struct_get_data(struct hffi_struct* c);
int hffi_struct_get_data_size(struct hffi_struct* c);

/**
 * @brief hffi_struct_ref: add the struct reference count
 * @param c: the struct
 * @param ref_count: the reference count
 */
void hffi_struct_ref(struct hffi_struct* c, int ref_count);

int hffi_struct_set_all(struct hffi_struct* c, void* ptr);

struct hffi_struct* hffi_struct_copy(struct hffi_struct* src);
int hffi_struct_eq(struct hffi_struct* hs1, struct hffi_struct* hs2);
void hffi_struct_dump(struct hffi_struct* arr, struct hstring* hs);

#endif // HFFI_COMMON_H
