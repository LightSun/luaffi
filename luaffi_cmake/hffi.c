
#include <stdio.h>
#include "hffi.h"
#include "atomic.h"
#include "h_linklist.h"
#include "h_alloctor.h"

#define hffi_new_value_auto_x(type) \
hffi_value* hffi_new_value_##type(hffi_type* t, type val){\
    hffi_value* v = hffi_new_value_auto(t, sizeof (type));\
    type* p = (type*)v->ptr;\
    *p = val;\
    return v;\
}

#define hffi_value_get_auto_x(t)\
int hffi_value_get_##t(hffi_value* val, t* out_ptr){\
    char _m[128];\
    char* msg[1];\
    msg[0] = _m;\
    ffi_type* ft = to_ffi_type(val->type->base_ffi_type, msg);\
    if(ft != NULL && ft->size >= sizeof(t)){\
       if(ft->size >= sizeof(t)){\
          *out_ptr = *((t*)val->ptr);\
          return 0;\
       }else{\
          strcpy(*msg, "wrong value type.");\
       }\
    }\
    return 1;\
}

ffi_type* to_ffi_type(int8_t v, char** msg){
#define hffi_TO_TYPE(t, v) case t:{ return &v;}
    switch (v) {
    hffi_TO_TYPE(FFI_TYPE_POINTER, ffi_type_pointer)
    hffi_TO_TYPE(FFI_TYPE_SINT8, ffi_type_sint8)
    hffi_TO_TYPE(FFI_TYPE_UINT8, ffi_type_uint8)
    hffi_TO_TYPE(FFI_TYPE_SINT16, ffi_type_sint16)
    hffi_TO_TYPE(FFI_TYPE_UINT16, ffi_type_uint16)
    hffi_TO_TYPE(FFI_TYPE_UINT32, ffi_type_uint32)
    hffi_TO_TYPE(FFI_TYPE_SINT32, ffi_type_sint32)
    hffi_TO_TYPE(FFI_TYPE_INT, ffi_type_sint32)
    hffi_TO_TYPE(FFI_TYPE_UINT64, ffi_type_uint64)
    hffi_TO_TYPE(FFI_TYPE_SINT64, ffi_type_sint64)
    hffi_TO_TYPE(FFI_TYPE_FLOAT, ffi_type_float)
    hffi_TO_TYPE(FFI_TYPE_DOUBLE, ffi_type_double)
    hffi_TO_TYPE(FFI_TYPE_VOID, ffi_type_void)
    }
//#undef hffi_TO_TYPE
    if(msg){
        char str[32];
        snprintf(str, 32, "unknwn ffi_type = %d", v);
        strcpy(*msg, str);
    }
    return NULL;
}
//---------------------------------------------------------------
hffi_type* hffi_new_type_base(int8_t ffi_type){
    hffi_type* ptr = MALLOC(sizeof (hffi_type));
    ptr->base_ffi_type = ffi_type;
    ptr->pointer_level = 0;
    ptr->pointer_base_type = FFI_TYPE_VOID;
    ptr->ref = 1;
    return ptr;
}
hffi_type* hffi_new_type(int8_t ffi_type, int8_t pointer_type,uint8_t pointer_level){
    hffi_type* ptr = MALLOC(sizeof (hffi_type));
    ptr->base_ffi_type = ffi_type;
    ptr->pointer_level = pointer_level;
    ptr->pointer_base_type = pointer_type;
    ptr->ref = 1;
    return ptr;
}
int hffi_type_size(hffi_type* t){
    //struct size not known here.
    if(t->base_ffi_type == FFI_TYPE_STRUCT){
        return 0;
    }
    ffi_type* ft = to_ffi_type(t->base_ffi_type, NULL);
    return ft ? ft->size : 0;
}
void hffi_delete_type(hffi_type* t){
    int old = atomic_add(&t->ref, -1);
    if(old == 1){
        FREE(t);
    }
}
//------------------------- value ------------------------
hffi_value* hffi_new_value_ptr(hffi_type* type){
    if(type->base_ffi_type != FFI_TYPE_POINTER) return NULL;
#define INIT_MEM(t, s)\
case t:{\
    val_ptr->ptr = MALLOC(s);\
}break;

    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    switch (type->pointer_base_type) {
        INIT_MEM(FFI_TYPE_SINT8, sizeof (sint8*))
        INIT_MEM(FFI_TYPE_UINT8, sizeof (uint8*))
        INIT_MEM(FFI_TYPE_SINT16, sizeof (sint16*))
        INIT_MEM(FFI_TYPE_UINT16, sizeof (uint16*))
        INIT_MEM(FFI_TYPE_SINT32, sizeof (sint32*))
        INIT_MEM(FFI_TYPE_UINT32, sizeof (uint32*))
        INIT_MEM(FFI_TYPE_SINT64, sizeof (sint64*))
        INIT_MEM(FFI_TYPE_UINT64, sizeof (uint64*))

        INIT_MEM(FFI_TYPE_INT, sizeof (sint32*))
        INIT_MEM(FFI_TYPE_FLOAT, sizeof (float*))
        INIT_MEM(FFI_TYPE_DOUBLE, sizeof (double*))
        INIT_MEM(FFI_TYPE_VOID, sizeof (void*))
    }
    val_ptr->type = type;
    val_ptr->ref = 1;
    atomic_add(&type->ref, 1);
    return val_ptr;
}
hffi_value* hffi_new_value_auto(hffi_type* type, int size){
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    val_ptr->ptr = MALLOC(size);
    val_ptr->type = type;
    val_ptr->ref = 1;
    if(type){
        atomic_add(&type->ref, 1);
    }
    return val_ptr;
}

hffi_new_value_auto_x(sint8)
hffi_new_value_auto_x(sint16)
hffi_new_value_auto_x(sint32)
hffi_new_value_auto_x(sint64)
hffi_new_value_auto_x(uint8)
hffi_new_value_auto_x(uint16)
hffi_new_value_auto_x(uint32)
hffi_new_value_auto_x(uint64)
hffi_new_value_auto_x(int)
hffi_new_value_auto_x(float)
hffi_new_value_auto_x(double)

hffi_value_get_auto_x(sint8)
hffi_value_get_auto_x(sint16)
hffi_value_get_auto_x(sint32)
hffi_value_get_auto_x(sint64)
hffi_value_get_auto_x(uint8)
hffi_value_get_auto_x(uint16)
hffi_value_get_auto_x(uint32)
hffi_value_get_auto_x(uint64)
hffi_value_get_auto_x(int)
hffi_value_get_auto_x(float)
hffi_value_get_auto_x(double)

void hffi_delete_value(hffi_value* val){
    int old = atomic_add(&val->ref, -1);
    if(old == 1){
        //if is struct.
        if(val->type->base_ffi_type == HFFI_TYPE_STRUCT){
            hffi_struct* c = val->ptr;
            hffi_delete_struct(c);
        }else{
            if(val->ptr !=NULL){
                FREE(val->ptr);
            }
        }
        hffi_delete_type(val->type);
        FREE(val);
    }
}
hffi_value* hffi_new_value_struct(hffi_struct* c){
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    val_ptr->ptr = c;
    val_ptr->type = hffi_new_type_base(HFFI_TYPE_STRUCT);
    val_ptr->ref = 1;
    atomic_add(&c->ref, 1);
    return val_ptr;
}
hffi_value* hffi_new_value_struct_ptr(hffi_struct* c){
    hffi_type* t = hffi_new_type(HFFI_TYPE_POINTER, HFFI_TYPE_STRUCT, 1);
    hffi_value* val_ptr = MALLOC(sizeof(hffi_value));
    val_ptr->ptr = c;
    val_ptr->type = t;
    val_ptr->ref = 1;
    atomic_add(&c->ref, 1);
    return val_ptr;
}

ffi_type* hffi_value_get_rawtype(hffi_value* val, char** msg){
    if(val->type->base_ffi_type == HFFI_TYPE_STRUCT){
        return ((hffi_struct*)(val->ptr))->type;
    }
    return to_ffi_type(val->type->base_ffi_type, msg);
}
hffi_struct* hffi_value_get_struct(hffi_value* val){
    if(val->type->base_ffi_type == HFFI_TYPE_STRUCT){
        return (hffi_struct*)val->ptr;
    }
    if(val->type->base_ffi_type == HFFI_TYPE_POINTER && val->type->pointer_base_type == HFFI_TYPE_STRUCT){
        return (hffi_struct*)val->ptr;
    }
    return NULL;
}

//----------------------------- ------------------------------
static inline void* __get_data_ptr(hffi_value* v){
    switch (v->type->base_ffi_type) {
    case HFFI_TYPE_STRUCT:
    {
        return ((hffi_struct*)(v->ptr))->data;
    }

    case HFFI_TYPE_POINTER:
    {
        if(v->type->pointer_base_type == HFFI_TYPE_STRUCT){
            return &((hffi_struct*)(v->ptr))->data;
        }else{
            return &v->ptr;
        }
    }break;

    default:
    {
        return v->ptr;
    };
    }
}

int hffi_call(void (*fn)(void), hffi_value** in,hffi_value* out, char** msg){
    hffi_get_pointer_count(in_count, in);
    //param types with values
    ffi_type ** argTypes = NULL;
    void **args = NULL;
    if(in_count > 0){
        argTypes = alloca(sizeof(ffi_type *) *in_count);
        args = alloca(sizeof(void *) *in_count);
        for(int i = 0 ; i < in_count ; i ++){
            argTypes[i] = hffi_value_get_rawtype(in[i], msg);
            if(argTypes[i] == NULL){
                return HFFI_STATE_FAILED;
            }
            //cast param value
            args[i] = __get_data_ptr(in[i]);
        }
    }
    //prepare call
    ffi_cif cif;
    ffi_type *return_type = hffi_value_get_rawtype(out, msg);
    if(return_type == NULL){
        return HFFI_STATE_FAILED;
    }
    ffi_status s = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, (unsigned int)in_count, return_type, argTypes);
    switch (s) {
    case FFI_OK:{
        ffi_call(&cif, fn, __get_data_ptr(out), args);
        return HFFI_STATE_OK;
    }break;

    case FFI_BAD_ABI:
        if(msg){
            strcpy(*msg, "FFI_BAD_ABI");
        }
        break;
    case FFI_BAD_TYPEDEF:
        if(msg){
            strcpy(*msg, "FFI_BAD_TYPEDEF");
        }
        break;
    }
    return HFFI_STATE_FAILED;
}

hffi_smtype* hffi_new_smtype(sint8 ffi_type, hffi_smtype** member_types){
    hffi_smtype* ptr = MALLOC(sizeof (hffi_smtype));
    ptr->ffi_type = ffi_type;
    ptr->ref = 1;
    ptr->elements = member_types;
    if(member_types != NULL){
        int i = 0;
        while(member_types[i] != NULL){
            atomic_add(&member_types[i]->ref, 1);
            i++;
        }
    }
    return ptr;
}
void hffi_delete_smtype(hffi_smtype* type){
    int old = atomic_add(&type->ref, -1);
    if(old == 1){
        if(type->elements != NULL){
            int count = 0;
            while(type->elements[count] != NULL){
                hffi_delete_smtype(type->elements[count]);
                count++;
            }
        }
        FREE(type);
    }
}
//--------------------------------------------------------------
hffi_struct* hffi_new_struct_base(sint8* types, int count){
    hffi_smtype* smtypes[count +1];
    for(int i = 0 ; i < count; i ++){
        smtypes[i] = hffi_new_smtype(types[i], NULL);
    }
    smtypes[count] = NULL;
    hffi_struct* c = hffi_new_struct(smtypes, -1, NULL);
    for(int i = 0 ; i < count; i ++){
        hffi_delete_smtype(smtypes[i]);
    }
    return c;
}
static inline void setChildrenDataPtrs(hffi_struct* p){
    size_t * offsets = (size_t *) &p->type->elements[p->count+1];
    for(int i = 0 ; i < p->count ; i ++){
        if(p->children[i] != NULL){
            p->children[i]->data = p->data + offsets[p->children[i]->parent_pos];
            setChildrenDataPtrs(p->children[i]);
        }
    }
}
hffi_struct* hffi_new_struct(hffi_smtype** member_types, sint16 parent_pos, char** msg){
    if(member_types == NULL){
        if(msg) strcpy(*msg, "member_types can't be null.");
        return NULL;
    }
    int count = 0;
    while(member_types[count] != NULL){
        count++;
    }
    //child structs
    hffi_struct** children = MALLOC(sizeof (hffi_struct*) * count);
    memset(children, 0, sizeof (hffi_struct*) * count);

    //struct data
    ffi_type *type;
    size_t *offsets;
    //raw_type, elements, offsets.
    type = (ffi_type *) MALLOC(sizeof(*type) + sizeof (ffi_type *) * (count+1) + sizeof (offsets[0]) * count);
    type->size = type->alignment = 0;
    type->type = FFI_TYPE_STRUCT;
    type->elements = (ffi_type **) (type + 1);
    ffi_type* tmp_type;
    for(int i = count -1 ; i >=0 ; i --){
        //struct.
        if(member_types[i]->ffi_type == HFFI_TYPE_STRUCT){
            //create sub struct. latter will set data-memory-pointer
            children[i] = hffi_new_struct(member_types[i]->elements, i, msg);
            if(children[i] == NULL){
                goto failed;
            }
            tmp_type = children[i]->type;
        }else{
            tmp_type = to_ffi_type(member_types[i]->ffi_type, msg);
            if(tmp_type == NULL){
                goto failed;
            }
        }
        type->elements[i] = tmp_type;
    }
    type->elements[count] = NULL;
    offsets = (size_t *) &type->elements[count+1];
    ffi_status status = ffi_get_struct_offsets(FFI_DEFAULT_ABI, type, offsets);
    if (status != FFI_OK){
        if(msg){
            strcpy(*msg, "ffi_get_struct_offsets() failed.");
        }
        goto failed;
    }
    //compute total size after align. make total_size align of 8
    int total_size = offsets[count  - 1] +  type->elements[count  - 1]->size;//TODO need align?

    //create hffi_struct to manage struct.
    hffi_struct* ptr = MALLOC(sizeof(hffi_struct));
    ptr->type = type;
    ptr->count = count;
    ptr->data_size = total_size;
    //of create by parent struct . never need MALLOC memory.
    if(parent_pos < 0){
        ptr->data = MALLOC(total_size);
    }else{
        ptr->data = NULL;
    }
    ptr->children = children;
    ptr->ref = 1;
    ptr->parent_pos = parent_pos;
    //handle sub structs' data-pointer.
    setChildrenDataPtrs(ptr);
    return ptr;

failed:
    FREE(type);
    hffi_delete_structs(children, count);
    return NULL;
}
void hffi_delete_structs(hffi_struct** cs, int count){
    if(cs){
        for(int i = count - 1; i >=0 ; i--){
             if(cs[i]){
                 hffi_delete_struct(cs[i]);
             }
        }
        FREE(cs);
    }
}
void hffi_delete_struct(hffi_struct* val){
    int old = atomic_add(&val->ref, -1);
    if(old == 1){
        if(val->type){
            FREE(val->type);
        }
        //children
        if(val->children != NULL){
            hffi_delete_structs(val->children, val->count);
        }
        // MALLOC data by self
        if(val->parent_pos < 0 && val->data){
            FREE(val->data);
        }
        //self
        FREE(val);
    }
}
void hffi_struct_ref(hffi_struct* c, int ref_count){
    atomic_add(&c->ref, ref_count);
}


//--------------------------------------------
//func_data(old->data);
#define RELEASE_LINK_LIST(list, x) \
if(list){\
    linklist_node* node = list;\
    linklist_node* old;\
    do{\
        old = node;\
        node = node->next;\
        {x;}\
        FREE(old);\
    }while(node != NULL);\
}
#define ADD_VAL_TO_MANAGER(vals, c)\
if(hm->vals == NULL){\
    hm->vals = linklist_create(v);\
}else{\
    hm->vals = linklist_insert_beginning(hm->values, v);\
}\
hm->c++;\
return hm->c;

hffi_manager* hffi_new_manager(){
    hffi_manager* ptr = MALLOC(sizeof (hffi_manager));
    memset(ptr, 0, sizeof (hffi_manager));
    return ptr;
}
void hffi_delete_manager(hffi_manager* ptr){
    RELEASE_LINK_LIST(ptr->values, hffi_delete_value((hffi_value*)old->data))
    RELEASE_LINK_LIST(ptr->types, hffi_delete_type((hffi_type*)old->data))
    RELEASE_LINK_LIST(ptr->structs, hffi_delete_struct((hffi_struct*)old->data))
    RELEASE_LINK_LIST(ptr->smtypes, hffi_delete_smtype((hffi_smtype*)old->data))
    RELEASE_LINK_LIST(ptr->hcifs, hffi_delete_cif((hffi_cif*)old->data))

    RELEASE_LINK_LIST(ptr->list, FREE(old->data));
    FREE(ptr);
}
int hffi_manager_add_value(hffi_manager* hm,hffi_value* v){
    ADD_VAL_TO_MANAGER(values, value_count)
}
int hffi_manager_add_type(hffi_manager* hm,hffi_type* v){
    ADD_VAL_TO_MANAGER(types, type_count)
}
int hffi_manager_add_smtype(hffi_manager* hm,hffi_smtype* v){
    ADD_VAL_TO_MANAGER(smtypes, smtype_count)
}
int hffi_manager_add_struct(hffi_manager* hm,hffi_struct* v){
   ADD_VAL_TO_MANAGER(structs, struct_count)
}
int hffi_manager_add_cif(hffi_manager* hm,hffi_cif* v){
   ADD_VAL_TO_MANAGER(hcifs, hcif_count)
}
void* hffi_manager_alloc(hffi_manager* hm,int size){
    void* data = MALLOC(size);
    if(hm->list == NULL){
        hm->list = linklist_create(data);
    }else{
        hm->list = linklist_insert_beginning(hm->list, data);
    }
    return data;
}

//------------------------- closure impl ---------------------------------
hffi_closure* hffi_new_closure(void* func_ptr, void (*fun_proxy)(ffi_cif*,void* ret,void** args,void* ud),
                               hffi_value** val_params, hffi_value* val_return, void* ud, char** msg){
    hffi_get_pointer_count(in_count, val_params);
    hffi_closure* ptr = MALLOC(sizeof(hffi_closure));
    memset(ptr, 0, sizeof(hffi_closure));
    ptr->ref = 1;
    ptr->code = &func_ptr;
    ptr->val_params = val_params;
    ptr->val_return = val_return;
    //mark reference
    for(int i = 0 ; i < in_count ; i ++){
        atomic_add(&val_params[i]->ref, 1);
    }
    atomic_add(&val_return->ref, 1);

    //create closure, prepare
    ptr->closure = ffi_closure_alloc(sizeof(ffi_closure), &func_ptr);
    if(ptr->closure == NULL){
        hffi_delete_closure(ptr);
        if(msg){
            strcpy(*msg, "create closure failed!");
        }
        return NULL;
    }

    //param
    ffi_type ** argTypes = NULL;
    if(in_count > 0){
        argTypes = alloca(sizeof(ffi_type *) *in_count);
        for(int i = 0 ; i < in_count ; i ++){
            argTypes[i] = hffi_value_get_rawtype(val_params[i], msg);
            if(argTypes[i] == NULL){
                hffi_delete_closure(ptr);
                return NULL;
            }
        }
    }
    //return type
    ffi_type *return_type = hffi_value_get_rawtype(val_return, msg);
    if(return_type == NULL){
        hffi_delete_closure(ptr);
        return NULL;
    }
    //prepare
    ffi_cif cif;
    if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, in_count, return_type, argTypes) == FFI_OK) {
        //concat closure with proxy.
        if (ffi_prep_closure_loc(ptr->closure, &cif, fun_proxy, ud, func_ptr) == FFI_OK) {
            return ptr; //now we can call func
        }else{
            hffi_delete_closure(ptr);
            if(msg){
                strcpy(*msg, "concat closure with function proxy failed!");
            }
        }
    }else{
        hffi_delete_closure(ptr);
        if(msg){
            strcpy(*msg, "ffi_prep_cif(...) failed!");
        }
    }
    return NULL;
}
void hffi_delete_closure(hffi_closure* val){
    int old = atomic_add(&val->ref, -1);
    if(old == 1){
        if(val->closure){
            ffi_closure_free(val->closure);
        }
        if(val->val_params){
            int n = 0;
            while(val->val_params[n] != NULL){
                hffi_delete_value(val->val_params[n]);
                n++;
            }
        }
        if(val->val_return){
            hffi_delete_value(val->val_return);
        }
        FREE(val);
    }
}
//------------------------------------
hffi_cif* hffi_new_cif(hffi_value** in, hffi_value* out, char** msg){
    hffi_get_pointer_count(in_count, in);
    //add ref
    for(int i = 0 ; i < in_count ; i ++){
        atomic_add(&in[i]->ref, 1);
    }
    atomic_add(&out->ref, 1);

    ffi_type ** argTypes = NULL;
    void **args = NULL;
    if(in_count > 0){
        argTypes = alloca(sizeof(ffi_type *) *in_count);
        args = MALLOC(sizeof(void *) *in_count);
        for(int i = 0 ; i < in_count ; i ++){
            argTypes[i] = hffi_value_get_rawtype(in[i], msg);
            if(argTypes[i] == NULL){
                return NULL;
            }
            //cast param value
            args[i] = __get_data_ptr(in[i]);
        }
    }
    //prepare call
    ffi_type *return_type = hffi_value_get_rawtype(out, msg);
    if(return_type == NULL){
        return NULL;
    }
    ffi_cif* cif = MALLOC(sizeof (ffi_cif));

    ffi_status s = ffi_prep_cif(cif, FFI_DEFAULT_ABI, (unsigned int)in_count, return_type, argTypes);
    switch (s) {
    case FFI_OK:{
       // ffi_call(&cif, fn, __get_data_ptr(out), args);
        //prepare
        hffi_cif* hcif = MALLOC(sizeof (hffi_cif));
        hcif->cif = cif;
        hcif->args = args;
        hcif->in = in;
        hcif->out = out;
        hcif->ref = 1;
        hcif->in_count = in_count;
        return hcif;
    }break;

    case FFI_BAD_ABI:
        if(msg){
            strcpy(*msg, "FFI_BAD_ABI");
        }
        break;
    case FFI_BAD_TYPEDEF:
        if(msg){
            strcpy(*msg, "FFI_BAD_TYPEDEF");
        }
        break;
    }
    FREE(cif);
    //ref
    for(int i = 0 ; i < in_count ; i ++){
        hffi_delete_value(in[i]);
    }
    hffi_delete_value(out);
    FREE(args);
    return NULL;
}
void hffi_delete_cif(hffi_cif* hcif){
    if(atomic_add(&hcif->ref, -1) == 1){
        FREE(hcif->cif);
        FREE(hcif->args);
        for(int i = 0 ; i < hcif->in_count ; i ++){
            hffi_delete_value(hcif->in[i]);
        }
        hffi_delete_value(hcif->out);
        FREE(hcif);
    }
}
void hffi_cif_call(hffi_cif* hcif, void* fn){
    ffi_call(hcif->cif, fn, __get_data_ptr(hcif->out), hcif->args);
}
hffi_value* hffi_cif_get_result_value(hffi_cif* hcif){
    return hcif->out;
}
hffi_value* hffi_cif_get_param_value(hffi_cif* hcif, int index){
    if(hcif->in_count == 0 || index >= hcif->in_count){
        return NULL;
    }
    return hcif->in[index];
}
int hffi_cif_get_param_count(hffi_cif* hcif){
    return hcif->in_count;
}
