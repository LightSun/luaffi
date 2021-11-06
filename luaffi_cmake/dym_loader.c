
#ifdef _WIN32
#include <windows.h>
#else
#include <errno.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "dym_loader.h"

/* architectures */
#if defined _WIN32 && defined UNDER_CE
# define OS_CE
#elif defined _WIN32
# define OS_WIN
#elif defined __APPLE__ && defined __MACH__
# define OS_OSX
#elif defined __linux__
# define OS_LINUX
#elif defined __FreeBSD__ || defined __OpenBSD__ || defined __NetBSD__
# define OS_BSD
#elif defined unix || defined __unix__ || defined __unix || defined _POSIX_VERSION || defined _XOPEN_VERSION
# define OS_POSIX
#endif

/* architecture */
#if defined __i386__ || defined _M_IX86
# define ARCH_X86
#elif defined __amd64__ || defined _M_X64
# define ARCH_X64
#elif defined __arm__ || defined __ARM__ || defined ARM || defined __ARM || defined __arm || defined __aarch64__
# define ARCH_ARM
#elif defined __powerpc64__
# define ARCH_PPC64
#else
# error
#endif

#ifdef _WIN32
#   ifdef UNDER_CE
        static void* DoLoadLibraryA(const char* name) {
          wchar_t buf[MAX_PATH];
          int sz = MultiByteToWideChar(CP_UTF8, 0, name, -1, buf, 512);
          if (sz > 0) {
            buf[sz] = 0;
            return LoadLibraryW(buf);
          } else {
            return NULL;
          }
        }
#       define LoadLibraryA DoLoadLibraryA
#   else
#       define GetProcAddressA GetProcAddress
#   endif

#   define LIB_FORMAT_1 "%s.dll"
#   define AllocPage(size) VirtualAlloc(NULL, size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE)
#   define FreePage(data, size) VirtualFree(data, 0, MEM_RELEASE)
#   define EnableExecute(data, size) do {DWORD old; VirtualProtect(data, size, PAGE_EXECUTE, &old); FlushInstructionCache(GetCurrentProcess(), data, size);} while (0)
#   define EnableWrite(data, size) do {DWORD old; VirtualProtect(data, size, PAGE_READWRITE, &old);} while (0)

#else
#ifdef OS_OSX
#   define LIB_FORMAT_1 "%s.dylib"
#   define LIB_FORMAT_2 "lib%s.dylib"
#else
#   define LIB_FORMAT_1 "%s.so"
#   define LIB_FORMAT_2 "lib%s.so"
#endif
#   define LoadLibraryA(name) dlopen(name, RTLD_LAZY | RTLD_GLOBAL)
#   define GetProcAddressA(lib, name) dlsym(lib, name)
#   define AllocPage(size) mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, -1, 0)
#   define FreePage(data, size) munmap(data, size)
#   define EnableExecute(data, size) mprotect(data, size, PROT_READ|PROT_EXEC)
#   define EnableWrite(data, size) mprotect(data, size, PROT_READ|PROT_WRITE)
#endif

#if defined ARCH_X86 || defined ARCH_X64
#define ALLOW_MISALIGNED_ACCESS
#endif
//------------------------------------------
dym_lib* dym_new_lib(const char* libname){
    void* lib;
    lib = LoadLibraryA(libname);

    #ifdef LIB_FORMAT_1
        if (!lib) {
            char buf[128];
            snprintf(buf, 128, LIB_FORMAT_1, libname);
            lib = LoadLibraryA(buf);
    #ifdef _WIN32
            if(!lib){
                printf("load lib failed: %s, code = %d\r\n", libname, (int)GetLastError());
            }
        }
    #endif
    #endif

    #ifdef LIB_FORMAT_2
        if (!lib) {
            char buf[128];
            snprintf(buf, 128, LIB_FORMAT_2, libname);
            lib = LoadLibraryA(buf);
        }
    #endif
    if(lib == NULL){
        return NULL;
    }
    dym_lib* dymlib = malloc(sizeof (dym_lib));
    dymlib->lib = lib;
    dymlib->func_nodes = NULL;
    return dymlib;
}
void dym_delete_lib(dym_lib* lib){
    //TODO
#ifdef _WIN32
    FreeLibrary((HANDLE)lib->lib);
#else
    dlclose(lib);
#endif
    free(lib);
}
dym_func* dym_lib_get_function(dym_lib* lib, const char* func_name){
    void* sym = GetProcAddressA(lib->lib, func_name);
    if(sym == NULL){
        return NULL;
    }
    dym_func* func = malloc(sizeof(dym_func));
    func->name = malloc(strlen(func_name) + 1);
    func->func_ptr = sym;

    if(lib->func_nodes){
        lib->func_nodes = list_insert_beginning(lib->func_nodes, func);
    }else{
        lib->func_nodes = list_create(func);
    }
    return func;
}

