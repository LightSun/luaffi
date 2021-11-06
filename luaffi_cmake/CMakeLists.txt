cmake_minimum_required(VERSION 3.5)

project(luaffi_app LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lua-5.3.6)

include_directories(lua-5.3.6/src)

add_library(ffi2 STATIC ffi2.h ffi.c ffi_ext.c)

# USE_C11_ATOMICS,USE_MSC_ATOMICS,USE_GCC_ATOMICS,USE_PTHREAD_ATOMICS
add_executable(luaffi_app
    atomic.h
    atomic.c
    hlist.h
    hlist.c
    hffi.h
    hffi.c
    hffi_test.c
    hffi_test_struct.c
    hffi_test_union.c
    libffi_study.c
    main.cpp
    )

target_link_libraries(ffi2 lua ffi)
target_link_libraries(luaffi_app ffi2 lua)
target_compile_definitions(luaffi_app PRIVATE STRUCT_MAX_MEMBER=16)
