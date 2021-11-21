#ifndef H_FFI_H
#define H_FFI_H
#include "hffi_common.h"

// offsets = (size_t *) &type->elements[c+1];
#define HFFI_OFFSETS(type, c)  (size_t *)&type->elements[c+1]
#define HFFI_STRUCT_TYPE_SIZE(c) (sizeof (ffi_type) + sizeof (ffi_type*) * (c + 1) + sizeof (size_t) * c)

#define hffi_new_value_auto_x_def(type) \
hffi_value* hffi_new_value_##type(type val);

#define hffi_value_get_auto_x_def(t)\
int hffi_value_get_##t(hffi_value* val, t* out_ptr);

#define hffi_get_pointer_count(n, p)\
int n = 0;\
while(p[n] != NULL){\
    n++;\
}

struct linklist_node;
struct array_list;
struct harray;

typedef struct hffi_value{
    sint8 base_ffi_type;            // see ffi.h
    sint8 pointer_base_type;        // the raw base type. eg: int** -> int
    sint8 multi_level_ptr;          // true if is multi-level-ptr. that means the data is malloc by function.
    int volatile ref;               // ref count
    void* ptr;                      // may be data-ptr/struct-ptr/harray-ptr
    ffi_type* ffi_type;             // cache ffi_type
    struct array_list* sub_types;   // mallocated sub types.
}hffi_value;

//struct member type
typedef struct hffi_smtype{
    int volatile ref;               // ref count
    sint8 ffi_type;
    void* _struct;                  //sometimes we may want put struct (hffi_struct*) directly.
    void* _harray;                  //sometimes we may want put harray(harray*) directly.
    struct array_list*  elements;   //children types. element is hffi_smtype
}hffi_smtype;

typedef struct hffi_struct{
    void* data;         // the data of struct, may be shared by parent struct.
    ffi_type * type;    // struct base info. all types with offsets.
    sint16 parent_pos;  // the parent position if need. or HFFI_STRUCT_NO_PARENT for no parent struct.
                        // HFFI_STRUCT_NO_DATA for malloc data by others.
    sint8 abi;          // which used to create
    sint8* hffi_types;  // the member base types, latter used to get value.
    int data_size;
    int count;          // member count
    int volatile ref;
    struct array_list* sub_ffi_types;
    struct array_list* children; // used to save struct_item
}hffi_struct;

typedef struct hffi_closure{
    ffi_closure* closure;
    void* func_ptr;             // function ptr
    struct array_list* in_vals;
    hffi_value* ret_val;
    int volatile ref;           // ref count
}hffi_closure;

typedef struct hffi_cif{
    ffi_cif* cif;
    struct array_list* in_vals; //element is hffi_value*
    hffi_value* out;
    void **args;
    int volatile ref;           //ref count
}hffi_cif;

typedef struct hffi_manager{
    struct linklist_node* values;
    struct linklist_node* structs;
    struct linklist_node* smtypes;
    struct linklist_node* harrays;
    struct linklist_node* hcifs;
    int harray_count;
    int value_count;
    int struct_count;
    int smtype_count;
    int hcif_count;
    struct linklist_node* list; //help list for memorys
}hffi_manager;

extern void list_travel_smtype_delete(void* d);
extern void list_travel_struct_delete(void* d);
extern void list_travel_value_delete(void* d);
extern void list_travel_hcif_delete(void* d);
extern hffi_value* hffi_get_void_value();

//error_msg can be null
ffi_type* to_ffi_type(int8_t ffi_t, char** error_msg);

//only alloc base needed memory.
hffi_value* hffi_new_value_ptr(sint8 hffi_t2);
/* create value. without data. that means the data will be allocate by an extra function. */
hffi_value* hffi_new_value_ptr_nodata(sint8 hffi_t2);
//auto alloc memory for target size
hffi_value* hffi_new_value(sint8 hffi_t1, sint8 hffi_t2, int size);
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

hffi_value_get_auto_x_def(sint8)
hffi_value_get_auto_x_def(sint16)
hffi_value_get_auto_x_def(sint32)
hffi_value_get_auto_x_def(sint64)
hffi_value_get_auto_x_def(uint8)
hffi_value_get_auto_x_def(uint16)
hffi_value_get_auto_x_def(uint32)
hffi_value_get_auto_x_def(uint64)
hffi_value_get_auto_x_def(int)
hffi_value_get_auto_x_def(float)
hffi_value_get_auto_x_def(double)

hffi_value* hffi_new_value_raw_type(sint8 type);
hffi_value* hffi_new_value_raw_type2(sint8 type, void* val_ptr);
hffi_value* hffi_new_value_ptr2(sint8 type, void* val_ptr);

/**
 * @brief hffi_new_value_struct: pack the struct as value
 * @param c the struct addr
 * @return the value
 */
hffi_value* hffi_new_value_struct(hffi_struct* c);

/**
 * @brief hffi_new_value_struct_ptr: pack the struct ptr as value
 * @param c the struct addr
 * @return the value
 */
hffi_value* hffi_new_value_struct_ptr(hffi_struct* c);

hffi_value* hffi_new_value_harray(struct harray* arr);
hffi_value* hffi_new_value_harray_ptr(struct harray* arr);
hffi_value* hffi_new_value_closure(hffi_closure* closure);

hffi_value* hffi_value_copy(hffi_value* val);

void hffi_delete_value(hffi_value* val);
void hffi_value_ref(hffi_value* val, int count);

ffi_type* hffi_value_get_rawtype(hffi_value* val, char** msg);

/** get value as base value. like int/char/in* /char* /... etc. */
int hffi_value_get_base(hffi_value* val, void* out_ptr);
hffi_struct* hffi_value_get_struct(hffi_value* c);
struct harray* hffi_value_get_harray(hffi_value* val);
/** only used for pointer.,  continueMemory: sometimes, some memory malloc split for array-array. */
struct harray* hffi_value_get_pointer_as_array(hffi_value* val, int rows, int cols, int continue_mem, int share_mem);

//ext: the ext infos for 'int rows, int cols, int continue_mem, int share_mem'.
int hffi_value_set_any(hffi_value* val, void* val_ptr);
//--------- for test ----------
int hffi_value_set_base(hffi_value* val, void* in_ptr);

/**
 * @brief hffi_call: do call ffi.
 * @param in :  the input parameters. must end with NULL
 * @param out : the output(return) value proxy.
 * @param msg : the error msg ,if need. or NULL
 * @return HFFI_STATE_SUCCESS for success. HFFI_STATE_FAILED failed
 */
int hffi_call(void (*fn)(void), hffi_value** in, hffi_value* out, char** msg);

/**
 * @brief hffi_call_abi
 * @param abi: the target abi
 * @param in : the in values.
 * @param out: the out values
 * @param msg: the msg
 * @return HFFI_STATE_SUCCESS for success. HFFI_STATE_FAILED failed
 */
int hffi_call_abi(int abi, void (*fn)(void), hffi_value** in, hffi_value* out, char** msg);

//-------------------- sm type -------------------------------

hffi_smtype* hffi_new_smtype(sint8 ffi_type);
//members as struct members
hffi_smtype* hffi_new_smtype_members(struct array_list* member_types);

hffi_smtype* hffi_new_smtype_struct(hffi_struct* _struct);
hffi_smtype* hffi_new_smtype_struct_ptr(hffi_struct* _struct);
hffi_smtype* hffi_new_smtype_harray(struct harray* array);
hffi_smtype* hffi_new_smtype_harray_ptr(struct harray* array);
void hffi_delete_smtype(hffi_smtype* type);

hffi_smtype* hffi_smtype_cpoy(hffi_smtype* src);
void hffi_smtype_ref(hffi_smtype* src, int c);

//---------------- struct -----------------
/**
 * @brief hffi_new_struct: create struct with member types.
 * @param member_types: the member types. must ends with NULL elements.
 * @param parent_pos: the position of parent struct.
 * @param msg: the error msg if create struct failed.
 * @return the struct pointer
 */
hffi_struct* hffi_new_struct(hffi_smtype** member_types, char** msg);
hffi_struct* hffi_new_struct_abi(int abi,hffi_smtype** member_types, char** msg);

/**
 * @brief hffi_new_struct_from_list: create struct with member types list.
 * @param abi: the abi to match.
 * @param member_types: the member types. must ends with NULL elements.
 * @param parent_pos: the position of parent struct.
 * @param msg: the error msg if create struct failed.
 * @return the struct pointer
 */
hffi_struct* hffi_new_struct_from_list2(int abi,struct array_list* member_types, char** msg);
hffi_struct* hffi_new_struct_from_list(struct array_list* member_types, char** msg);
hffi_struct* hffi_new_struct_from_list_nodata(int abi,struct array_list* member_types, char** msg);
/**
 * @brief hffi_new_struct_base:create struct by base types. except struct or its' pointer.
 * @param types: the base types. see HFFI_TYPE_<X>
 * @param count: the type count
 * @return the struct ptr.
 */
hffi_struct* hffi_new_struct_base(sint8* types, int count);
hffi_struct* hffi_new_struct_base_abi(int abi,sint8* types, int count);

void hffi_delete_structs(hffi_struct** cs, int count);

int hffi_struct_is_pointer(hffi_struct* hs, int index);
/**
 * get base value. if base val is simple ptr.you must assign target_hffi.
 * @return HFFI_STATE_OK for success.
 */
int hffi_struct_get_base(hffi_struct* hs, int index, sint8 target_hffi, void* ptr);
int hffi_struct_set_base(hffi_struct* hs, int index, sint8 target_hffi, void* ptr);
hffi_struct* hffi_struct_get_struct(hffi_struct* hs, int index);
struct harray* hffi_struct_get_harray(hffi_struct* hs, int index);
struct harray* hffi_struct_get_as_array(hffi_struct* hs, int index, sint8 hffi_t,int rows, int cols,
                                 int continue_mem, int share_memory);

int hffi_struct_set_all(struct hffi_struct* c, void* ptr);
//----------------- manager -------------------

hffi_manager* hffi_new_manager();
void hffi_delete_manager(hffi_manager*);
int hffi_manager_add_value(hffi_manager*, hffi_value* v);
int hffi_manager_add_smtype(hffi_manager*, hffi_smtype* v);
int hffi_manager_add_struct(hffi_manager*, hffi_struct* v);
int hffi_manager_add_cif(hffi_manager* hm,hffi_cif* v);
int hffi_manager_add_harray(hffi_manager* hm,struct harray* v);
/**
 * @brief hffi_manager_alloc: allocate a memory for temp use. the memory will be released by call 'hffi_delete_manager()'
 * @param size the memory size
 * @return the data ptr.
 */
void* hffi_manager_alloc(hffi_manager*, int size);

//------------------- closure --------------
/**
 * @brief hffi_new_closure: create closure with concat function proxy.
 * @param fun_proxy  the function proxy as function-ptr
 * @param param_types the parameter values. must end with null.
 * @param return_type the return type
 * @param ud the user data pass to function-proxy
 * @param msg the output error msg. can be null.
 * @return the struct of closure. or null if failed.
 */
hffi_closure* hffi_new_closure(void (*fun_proxy)(ffi_cif*,void* ret,void** args,void* ud),
                               struct array_list* in_vals, hffi_value* return_type, void* ud, char** msg);
int hffi_delete_closure(hffi_closure* c);
hffi_closure* hffi_closure_copy(hffi_closure* c);
void hffi_closure_ref(hffi_closure* hc, int c);

//---------------------- cif ------------------------
hffi_cif* hffi_new_cif(int abi, struct array_list* in_vals, int var_count,hffi_value* out, char** msg);
void hffi_delete_cif(hffi_cif* hcif);
void hffi_cif_ref(hffi_cif* hcif, int c);
void hffi_cif_call(hffi_cif* hcif, void* fun);
hffi_value* hffi_cif_get_result_value(hffi_cif* hcif);
hffi_value* hffi_cif_get_param_value(hffi_cif* hcif, int index);
int hffi_cif_get_param_count(hffi_cif* hcif);

#endif
//load lib can like: https://github.com/chfoo/callfunc/blob/ef79a3985be728c42914320ddfc30f9e764f838e/src/c/callfunc.c
