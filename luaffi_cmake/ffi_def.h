
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include <ffi.h>

union PriValue{
    signed char i8;
    unsigned char ui8;
    short int s16;
    unsigned short int us16;
    int s32;
    unsigned int us32;
    long long s64;
    unsigned long long us64;
    float f32;
    double f64;
};

typedef struct hffi_type{
    int8_t base_ffi_type;       //see ffi.h
    int8_t pointer_base_type;   //the raw base type. eg: int** -> int
    uint8_t pointer_level;      //the pointer level. eg: int** -> 2
}hffi_type;

typedef struct hffi_value{
    hffi_type type;
    union PriValue base;
    int ref;        //ref count
    void* ptr;
}hffi_value;

hffi_type* hffi_new_type(int8_t base_ffi_type, int8_t pointer_base_type,uint8_t pointer_level);
hffi_value* hffi_new_value(hffi_type* type, union PriValue val, void* ptr_val);
void hffi_delete_value(hffi_value* type);
int hffi_call(void (*fn)(void), hffi_value** in, unsigned in_count,hffi_value* out, char** msg);






