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

#define HFFI_STATE_OK 0
#define HFFI_STATE_FAILED -1

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

#define HFFI_TYPE_STRING (FFI_TYPE_LAST + 1)
#define HFFI_TYPE_HARRAY (FFI_TYPE_LAST + 2)
#define HFFI_TYPE_HARRAY_PTR (FFI_TYPE_LAST + 3)
#define HFFI_TYPE_STRUCT_PTR (FFI_TYPE_LAST + 4)

#define H_UNSED(x) (void)x;
#define H_UNSED2(x,y) H_UNSED(x) H_UNSED(y)

struct hffi_struct;

void hffi_delete_struct(struct hffi_struct* c);

void* hffi_struct_get_data(struct hffi_struct* c);
int hffi_struct_get_data_size(struct hffi_struct* c);

/**
 * @brief hffi_struct_ref: add the struct reference count
 * @param c: the struct
 * @param ref_count: the reference count
 */
void hffi_struct_ref(struct hffi_struct* c, int ref_count);

struct hffi_struct* hffi_struct_copy(struct hffi_struct* src);

#endif // HFFI_COMMON_H
