#ifndef FLOAT_BITS_H
#define FLOAT_BITS_H

#include <stdlib.h>
//#include <stdio.h>

#ifndef sint32
typedef signed int sint32;
#endif

#ifndef sint64
typedef signed long long sint64;
#endif

#define H_FLOAT_EQ(v1, v2) (abs(hffi_float2Bits(v1) - hffi_float2Bits(v2)) < 10)
#define H_DOUBLE_EQ(v1, v2) (llabs(hffi_double2Bits(v1) - hffi_double2Bits(v2)) < 10)
//#define H_FLOAT_EQ(v1, v2) ((float)fabs(v1- v2) <= 0.000001f)

union Hffi_FloatIntUnion {
    float   val;
    sint32 ival;
};
union Hffi_DoubleInt64Union {
    double   val;
    sint64 ival;
};
// Helper to see a float as its bit pattern (w/o aliasing warnings)
static inline sint32 hffi_float2Bits(float x) {
    union Hffi_FloatIntUnion data;
    data.val = x;
   // printf("hffi_float2Bits: x = %.5f, ival = %d\n", x, data.ival);
    return data.ival;
}
// Helper to see a bit pattern as a float (w/o aliasing warnings)
static inline float hffi_bits2float(sint32 bits) {
    union Hffi_FloatIntUnion data;
    data.ival = bits;
    return data.val;
}
static inline sint64 hffi_double2Bits(double x) {
    union Hffi_DoubleInt64Union data;
    data.val = x;
    return data.ival;
}
static inline double hffi_bits2double(sint64 bits) {
    union Hffi_DoubleInt64Union data;
    data.ival = bits;
    return data.val;
}
static inline int hffi_float_isnan(float x) {
    return !(x == x);
}

#endif // FLOAT_BITS_H
