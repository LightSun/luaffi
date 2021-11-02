
#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

typedef struct hffi_type{
    int8_t base_ffi_type;
    int8_t pointer_base_type;
    uint8_t pointer_level;
}hffi_type;

typedef struct hffi_value{
    hffi_type type;
    void* ptr;
    union{
        signed char i8;
        unsigned char ui8;
    }BaseValue;
}hffi_value;
