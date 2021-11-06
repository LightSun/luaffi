#ifndef DYM_LOADER_H
#define DYM_LOADER_H

#include "hlist.h"

#if defined HFFI_BUILD_AS_DLL
# define EXPORT __declspec(dllexport)
#elif defined __GNUC__
# define EXPORT __attribute__((visibility("default")))
#else
# define EXPORT
#endif

typedef struct dym_func{
    char* name;
    void* func_ptr;
}dym_func;

typedef struct dym_lib{
    void* lib;
    list_node* func_nodes;
}dym_lib;

dym_lib* dym_new_lib(const char* libname);
void dym_delete_lib(dym_lib* lib);
dym_func* dym_lib_get_function(dym_lib* lib, const char* func_name);

#endif // DYM_LOADER_H
