cmake_minimum_required(VERSION 3.5)

project(luaffi_app LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#cmake can build gTest
SET(gTest_enabled true)
SET(ENV_HOME true)
#SET(ext_libffi true)


if(ext_libffi)
    include_directories(E:/study/tools/libffi-3.4.2/x86_64-w64-mingw32/include)
    set(ffi_libs E:/study/tools/libffi-3.4.2/x86_64-w64-mingw32/.libs/libffi.a)
else(ext_libffi)
    set(ffi_libs ffi)
endif(ext_libffi)

if(gTest_enabled)
    if(ENV_HOME)
        #home
        include_directories(D:/study/google/googletest-release-1.11.0/googletest/include)
        SET(gTest_lib_dir D:/study/google/googletest-release-1.11.0/build_out/x64_mingw)
    else(ENV_HOME)
        #work
        include_directories(D:/study/github/thirds/googletest-release-1.11.0/googletest/include)
        set(gTest_lib_dir D:/study/github/thirds/googletest-release-1.11.0/googletest/build_out)
    endif(ENV_HOME)

    #link_directories(${gTest_lib_dir}/release/libgtest.a)
    #set(gTest_libs gtest)
    set(gTest_libs ${gTest_lib_dir}/release/libgtest.a)
endif()

add_subdirectory(lua-5.3.6)

include_directories(lua-5.3.6/src)

add_library(ffi2 STATIC ffi2.h ffi.c ffi_ext.c)

# USE_C11_ATOMICS,USE_MSC_ATOMICS,USE_GCC_ATOMICS,USE_PTHREAD_ATOMICS

SET(SRC_HFFI
    h_float_bits.h
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
    hffi_pri.h
    hffi.h
    hffi.c
    hffi_common.c
    dym_loader.h
    dym_loader.c

    hffi_lua.h
    hffi_lua.c
    lua_utils.h
    lua_utils.c

    h_array_test.c
    hffi_test.c
    hffi_test2.c
    hffi_test_struct.c
    hffi_test_union.c
    libffi_study.c
    libffi_test.c
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

target_link_libraries(ffi2 lua ${ffi_libs})
target_link_libraries(luaffi_app ffi2 lua)
if(gTest_enabled)
    target_link_libraries(luaffi_app ${gTest_libs})
endif()
#add_compile_definitions(X86_64)
add_compile_definitions(X86_WIN64)

#--------- add test lib ------
add_library(test_hffi SHARED libtest.c)
