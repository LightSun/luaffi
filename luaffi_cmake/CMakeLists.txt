cmake_minimum_required(VERSION 3.5)

project(luaffi_app LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#cmake can build gTest
SET(gTest_enabled true)
if(gTest_enabled)
    #work
    include_directories(D:/study/github/thirds/googletest-release-1.11.0/googletest/include)
    set(gTest_lib_dir D:/study/github/thirds/googletest-release-1.11.0/googletest/build_out)

    #home
    #include_directories(D:/study/google/googletest-release-1.11.0/googletest/include)
    #SET(gTest_lib_dir D:/study/google/googletest-release-1.11.0/build_out/x64_mingw)

    #link_directories(${gTest_lib_dir}/release/libgtest.a)
    #set(gTest_libs gtest)
    set(gTest_libs ${gTest_lib_dir}/release/libgtest.a)
endif()

add_subdirectory(lua-5.3.6)

include_directories(lua-5.3.6/src)

add_library(ffi2 STATIC ffi2.h ffi.c ffi_ext.c)

# USE_C11_ATOMICS,USE_MSC_ATOMICS,USE_GCC_ATOMICS,USE_PTHREAD_ATOMICS

SET(SRC_HFFI
    hffi_common.h
    h_alloctor.h
    h_alloctor.c
    atomic.h
    atomic.c
    h_string.h
    h_string.c
    h_array.h
    h_array.c
    h_list.h
    h_list.c
    h_linklist.h
    h_linklist.c
    hffi.h
    hffi.c
    dym_loader.h
    dym_loader.c

    hffi_lua.h
    hffi_lua.c
    lua_utils.h
    lua_utils.c

    h_array_test.c
    hffi_test.c
    hffi_test_struct.c
    hffi_test_union.c
    libffi_study.c
    main.cpp
    )

if(gTest_enabled)
    set(SRC_HFFI ${SRC_HFFI}
         hffi_test_value.cpp
         hffi_test_struct2.cpp
        )
endif()

add_executable(luaffi_app
    ${SRC_HFFI}
    )

target_link_libraries(ffi2 lua ffi)
target_link_libraries(luaffi_app ffi2 lua)
if(gTest_enabled)
    target_link_libraries(luaffi_app ${gTest_libs})
endif()
#target_compile_definitions(luaffi_app PRIVATE STRUCT_MAX_MEMBER=16)
