
#include "hffi_common.h"

#define hffi_new_value_auto_x_def(type) \
hffi_value* hffi_new_value_##type(hffi_type* t, type val);

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

typedef struct hffi_type{
    int8_t base_ffi_type;        //see ffi.h
    int8_t pointer_base_type;    //the raw base type. eg: int** -> int
    //int8_t pointer_level;      //the pointer level. eg: int** -> 2
    int volatile ref;
}hffi_type;

typedef struct hffi_value{
    hffi_type* type;
    int volatile ref;        //ref count
    void* ptr;               //may be data-ptr/struct-ptr/harray-ptr
    ffi_type* ffi_type;      //cache ffi_type
    struct array_list* sub_types;
}hffi_value;

//struct member type
typedef struct hffi_smtype{
    int volatile ref;               // ref count
    sint8 ffi_type;
    void* _struct;                  //sometimes we may want put struct (hffi_struct*) directly.
    void* _harray;                  //sometimes we may want put harray(harray*) directly.
    struct hffi_smtype** elements;  //child type
}hffi_smtype;

typedef struct hffi_struct{
    void* data;         // the data of struct, may be shared by parent struct.
    ffi_type * type;    // struct base info. all types with offsets.
    sint16 parent_pos;  // the parent position if need. or -1 for no parent struct.
    int type_size;      // the 'type' size.
    int data_size;
    int count;          //member count
    int volatile ref;
    struct array_list* sub_ffi_types;
    struct array_list* children; // used to save struct_item
}hffi_struct;

typedef struct hffi_closure{
    ffi_closure* closure;
    void** code;             // function ptr ptr
    hffi_value** val_params;
    hffi_value* val_return;
    int volatile ref;        // ref count
}hffi_closure;

typedef struct hffi_cif{
    ffi_cif* cif;
    struct array_list* in_vals; //element is hffi_value*
    hffi_value* out;
    void **args;
    int volatile ref;        // ref count
}hffi_cif;

typedef struct hffi_manager{
    struct linklist_node* values;
    struct linklist_node* types;
    struct linklist_node* structs;
    struct linklist_node* smtypes;
    struct linklist_node* hcifs;
    int value_count;
    int type_count;
    int struct_count;
    int smtype_count;
    int hcif_count;
    struct linklist_node* list; //help list for memorys
}hffi_manager;


//error_msg can be null
ffi_type* to_ffi_type(int8_t ffi_t, char** error_msg);

hffi_type* hffi_new_type(int8_t ffi_type, int8_t ptr_base_type);
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

void hffi_delete_value(hffi_value* val);
void hffi_value_ref(hffi_value* val, int count);
void hffi_value_set_ptr_base_type(hffi_value* val, sint8 base);

ffi_type* hffi_value_get_rawtype(hffi_value* val, char** msg);

hffi_struct* hffi_value_get_struct(hffi_value* c);
struct harray* hffi_value_get_harray(hffi_value* val);

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
#define hffi_new_smtype_base(t) hffi_new_smtype(t, NULL)
hffi_smtype* hffi_new_smtype(sint8 ffi_type, hffi_smtype** member_types);
hffi_smtype* hffi_new_smtype_struct(hffi_struct* _struct);
hffi_smtype* hffi_new_smtype_struct_ptr(hffi_struct* _struct);
hffi_smtype* hffi_new_smtype_harray(struct harray* array);
hffi_smtype* hffi_new_smtype_harray_ptr(struct harray* array);
void hffi_delete_smtype(hffi_smtype* type);

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

/**
 * @brief hffi_new_struct_base:create struct by base types. except struct or its' pointer.
 * @param types: the base types. see HFFI_TYPE_<X>
 * @param count: the type count
 * @return the struct ptr.
 */
hffi_struct* hffi_new_struct_base(sint8* types, int count);

void hffi_delete_structs(hffi_struct** cs, int count);
hffi_struct* hffi_struct_copy(hffi_struct* _hs);

//----------------- manager -------------------

hffi_manager* hffi_new_manager();
void hffi_delete_manager(hffi_manager*);
int hffi_manager_add_value(hffi_manager*, hffi_value* v);
int hffi_manager_add_type(hffi_manager*, hffi_type* v);
int hffi_manager_add_smtype(hffi_manager*, hffi_smtype* v);
int hffi_manager_add_struct(hffi_manager*, hffi_struct* v);
int hffi_manager_add_cif(hffi_manager* hm,hffi_cif* v);
/**
 * @brief hffi_manager_alloc: allocate a memory for temp use. the memory will be released by call 'hffi_delete_manager()'
 * @param size the memory size
 * @return the data ptr.
 */
void* hffi_manager_alloc(hffi_manager*, int size);

//------------------- closure --------------
/**
 * @brief hffi_new_closure: create closure with concat function proxy.
 * @param codeloc  often is the function-ptr
 * @param fun_proxy  the function proxy as function-ptr
 * @param param_types the parameter values. must end with null.
 * @param return_type the return type
 * @param ud the user data pass to function-proxy
 * @param msg the output error msg. can be null.
 * @return the struct of closure. or null if failed.
 */
hffi_closure* hffi_new_closure(void* codeloc, void (*fun_proxy)(ffi_cif*,void* ret,void** args,void* ud),
                               hffi_value** param_types, hffi_value* return_type, void* ud, char** msg);
void hffi_delete_closure(hffi_closure* c);

//---------------------- cif ------------------------
hffi_cif* hffi_new_cif(int abi, struct array_list* in_vals, hffi_value* out, char** msg);
void hffi_delete_cif(hffi_cif* hcif);
void hffi_cif_call(hffi_cif* hcif, void* fun);
hffi_value* hffi_cif_get_result_value(hffi_cif* hcif);
hffi_value* hffi_cif_get_param_value(hffi_cif* hcif, int index);
int hffi_cif_get_param_count(hffi_cif* hcif);

//load lib can like: https://github.com/chfoo/callfunc/blob/ef79a3985be728c42914320ddfc30f9e764f838e/src/c/callfunc.c
