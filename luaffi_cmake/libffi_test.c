/*
  clang -o ffibug -I/mingw64/include -I/mingw64/lib/libffi-3.2.1/include ffibug.c -L/mingw64/lib/ -lffi -g
*/

#include <stdio.h>
#include <stdlib.h>
#include <ffi.h>




typedef struct struct_of_3 {
  float a;
  float b;
  float c;
} struct_of_3;

void struct_of_3_fn(struct struct_of_3 arg)
{
  if (arg.a != 4 || arg.b != 5 || arg.c != 1 ) abort();
}

static void struct_of_3_gn(ffi_cif* cif, void* resp, void** args, void* userdata)
{
  struct struct_of_3 a0;
  a0 = *(struct struct_of_3*)(args[0]);
  struct_of_3_fn(a0);
}

typedef struct struct_of_4 {
  float a;
  float b;
  float c;
  float d;
} struct_of_4;

void struct_of_4_fn(struct struct_of_4 arg)
{
  if (arg.a != 4 || arg.b != 5 || arg.c != 1 || arg.d != 8) abort();
}

static void struct_of_4_gn(ffi_cif* cif, void* resp, void** args, void* userdata)
{
  struct struct_of_4 a0;
  a0 = *(struct struct_of_4*)(args[0]);
  struct_of_4_fn(a0);
}

typedef struct struct_of_5 {
  float a;
  float b;
  float c;
  float d;
  float e;
} struct_of_5;

void struct_of_5_fn(struct struct_of_5 arg)
{
  if (arg.a != 4 || arg.b != 5 || arg.c != 1 || arg.d != 8 || arg.e != 7) abort();
}

static void struct_of_5_gn(ffi_cif* cif, void* resp, void** args, void* userdata)
{
  struct struct_of_5 a0;
  a0 = *(struct struct_of_5*)(args[0]);
  struct_of_5_fn(a0);
}

void libffi_test_closure_struct(){
  ffi_cif cif;
  void *code;
  ffi_closure *pcl = ffi_closure_alloc(sizeof(ffi_closure), &code);
  ffi_type* cls_struct_fields0[6];
  ffi_type cls_struct_type0;
  ffi_type* dbl_arg_types[6];

  struct struct_of_3 struct3 = {4.0, 5.0, 1.0};
  struct struct_of_4 struct4 = {4.0, 5.0, 1.0, 8.0};
  struct struct_of_5 struct5 = {4.0, 5.0, 1.0, 8.0, 7.0};

  cls_struct_type0.size = 0;
  cls_struct_type0.alignment = 0;
  cls_struct_type0.type = FFI_TYPE_STRUCT;
  cls_struct_type0.elements = cls_struct_fields0;

  cls_struct_fields0[0] = &ffi_type_float;
  cls_struct_fields0[1] = &ffi_type_float;
  cls_struct_fields0[2] = &ffi_type_float;
  cls_struct_fields0[3] = NULL;
  cls_struct_fields0[4] = NULL;
  cls_struct_fields0[5] = NULL;

  dbl_arg_types[0] = &cls_struct_type0;
  dbl_arg_types[1] = NULL;


  // test 3 floats
  if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &ffi_type_void, dbl_arg_types)
    != FFI_OK) abort();

  if (ffi_prep_closure_loc(pcl, &cif, struct_of_3_gn, NULL, code)
    != FFI_OK) abort();

  ((void(*)(struct_of_3)) (code))(struct3);
  printf("works with 3 floats\n");
  fflush(stdout);

  // test 5 floats
  cls_struct_fields0[3] = &ffi_type_float;
  cls_struct_fields0[4] = &ffi_type_float;
  if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &ffi_type_void, dbl_arg_types)
    != FFI_OK) abort();

  if (ffi_prep_closure_loc(pcl, &cif, struct_of_5_gn, NULL, code)
    != FFI_OK) abort();

  ((void(*)(struct_of_5)) (code))(struct5);
   printf("works with 5 floats\n");
  fflush(stdout);

  //test 4 floats
  cls_struct_fields0[4] = NULL;
  if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &ffi_type_void, dbl_arg_types)
    != FFI_OK) abort();

  if (ffi_prep_closure_loc(pcl, &cif, struct_of_4_gn, NULL, code)
    != FFI_OK) abort();

  ((void(*)(struct_of_4)) (code))(struct4);
   printf("works with 4 floats\n");
  fflush(stdout);
}
