cmake_minimum_required(VERSION 3.5)

project(luaffi_app LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(lua-5.3.6)

include_directories(lua-5.3.6/src)

add_library(ffi2 STATIC ffi2.h ffi.c ffi_ext.c)
add_executable(luaffi_app
    ffi_def.h
    ffi_impl.c
    libffi_study.c
    main.cpp
    )

target_link_libraries(ffi2 lua ffi)
target_link_libraries(luaffi_app ffi2 lua)
