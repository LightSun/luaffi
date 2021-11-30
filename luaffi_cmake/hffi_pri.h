#ifndef HFFI_PRI_H
#define HFFI_PRI_H

#include "atomic.h"

#define HFFI_TAIL_INT_PTR_SET(obj, offset, val)\
do{\
    volatile int* ref_ptr = (void*)obj + offset;\
    atomic_set(ref_ptr, val);\
}while(0);

#define HFFI_TAIL_INT_PTR_ADD(obj, offset, delta)\
do{\
    volatile int* ref_ptr = (void*)obj + offset;\
    atomic_add(ref_ptr, delta);\
}while(0);

#define HFFI_TAIL_INT_PTR_ADD_X(obj, offset, delta, x)\
do{\
    volatile int* ref_ptr = (void*)obj + offset;\
    int old = atomic_add(ref_ptr, delta);\
    x;\
}while(0);


#endif // HFFI_PRI_H
