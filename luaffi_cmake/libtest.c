
#include <stdlib.h>

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

typedef struct Libtest_struct1{
    float f;
    uint64 u64;
    sint16 arr[3];
    sint8 s8;
}Libtest_struct1;

int libtest_add_s8s32_s32(sint8 a, int b);

int* libtest_add_s8s32_s32p(sint8 a, int b);

int* libtest_add_s8ps32p_s32p(sint8* a, int* b);

float* libtest_add_farrfarr_farr(float a[3], float b[3]);

Libtest_struct1 libtest_struct_s_s(Libtest_struct1 s);
Libtest_struct1* libtest_struct_sp_sp(Libtest_struct1* s);

Libtest_struct1* libtest_struct_s_sp(Libtest_struct1 s);

//-----------------------------
int libtest_add_s8s32_s32(sint8 a, int b){
    return a + b;
}
int* libtest_add_s8s32_s32p(sint8 a, int b){
    int* addr = malloc(sizeof (int));
    addr[0] = a + b;
    return addr;
}
int* libtest_add_s8ps32p_s32p(sint8* a, int* b){
    int* addr = malloc(sizeof (int));
    addr[0] = *a + *b;
    return addr;
}
float* libtest_add_farrfarr_farr(float a[3], float b[3]){
    float* addr = malloc(sizeof (float) * 3);
    addr[0] = a[0] + b[0];
    addr[1] = a[1] + b[1];
    addr[2] = a[2] + b[2];
    return addr;
}
//-----------------------------
Libtest_struct1 libtest_struct_s_s(Libtest_struct1 s){
    return s;
}
Libtest_struct1* libtest_struct_sp_sp(Libtest_struct1* s){
    Libtest_struct1* str = malloc(sizeof(Libtest_struct1));
    str->f = s->f * 10;
    str->u64 = s->u64 * 10;
    str->arr[0] = s->arr[0] * 10;
    str->arr[1] = s->arr[1] * 10;
    str->arr[2] = s->arr[2] * 10;
    str->s8 = s->s8 * 10;
    return str;
}
Libtest_struct1* libtest_struct_s_sp(Libtest_struct1 s){
    Libtest_struct1* str = malloc(sizeof(Libtest_struct1));
    str->f = s.f * 10;
    str->u64 = s.u64 * 10;
    str->arr[0] = s.arr[0] * 10;
    str->arr[1] = s.arr[1] * 10;
    str->arr[2] = s.arr[2] * 10;
    str->s8 = s.s8 * 10;
    return str;
}

