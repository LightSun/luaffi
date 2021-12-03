#ifndef DYM_LOADER_H
#define DYM_LOADER_H

#include "h_list.h"

#if defined HFFI_BUILD_AS_DLL
# define EXPORT __declspec(dllexport)
#elif defined __GNUC__
# define EXPORT __attribute__((visibility("default")))
#else
# define EXPORT
#endif

typedef struct dym_lib{
    void* lib;
    array_list* func_list;
    int volatile ref;
    int volatile total_func_ref;
}dym_lib;

typedef struct dym_func{
    dym_lib* lib;
    char* name;
    void* func_ptr;
}dym_func;

#define dym_new_lib(libname) dym_new_lib2(NULL, libname)
dym_lib* dym_new_lib2(const char* dir,const char* libname);

void dym_delete_lib(dym_lib* lib);
dym_func* dym_lib_get_function(dym_lib* lib, const char* func_name, int ref);
void dym_delete_func(dym_func* func);
void dym_delete_func_by_name(dym_lib* lib, const char* name);

void dym_func_ref(dym_func* func, int c);



#endif // DYM_LOADER_H
