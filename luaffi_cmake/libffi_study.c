#include <stdio.h>
#include <ffi.h>
#include <stdlib.h>

#if defined(_MSC_VER)
#include <malloc.h>
#define alloca _alloca
#endif

//alloca 分配的内存会自动释放
int testFunc(int m, int n) {
    printf("testFunc >>> params: %d %d \n", m, n);
    return m+n;
}

int testFunc2(int** m, int n){
    printf("testFunc2 >>> params: %d\n", n);
    int* arr = (int*)malloc(sizeof(int));
    arr[0] = 1;
    *m = arr;
    return n;
}

//void __fun(ffi_cif * cif, void *ret, void *args[], lua_state*L)

// 函数实体
void calCircleArea(ffi_cif * cif,
                  float *ret,
                  void *args[],
                  FILE * ud) {
    float pi = 3.14;
    float r = **(float **)args[0];
    float area = pi * r * r;
    *ret = area;
    printf("calCircleArea >>> area:%.2f,  *ret = %.2f\n", area, *ret);
}
typedef float (*Bound_calCircleArea)(float *);
void testClosure(){
   printf("-------- testClosure --------\n");
   ffi_cif cif;
   ffi_type *args[1];
   ffi_closure *closure;

   ///声明一个函数指针,通过此指针动态调用已准备好的函数
   void* func;
   float rc = 0;

   /* Allocate closure and bound_calCircleArea */  //创建closure
   closure = ffi_closure_alloc(sizeof(ffi_closure), &func);

   if (closure) {
       /* Initialize the argument info vectors */
       args[0] = &ffi_type_pointer;
       /* Initialize the cif */  //生成函数原型 &ffi_type_float：返回值类型
       if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 1, &ffi_type_float, args) == FFI_OK) {
           /* Initialize the closure, setting stream to stdout */
               // 通过 ffi_closure 把 函数原型_cifPtr / 函数实体JPBlockInterpreter / 上下文对象self / 函数指针blockImp 关联起来
           if (ffi_prep_closure_loc(closure, &cif, calCircleArea,stdout, func) == FFI_OK) {
                   float r = 10.0;
                   //当执行了bound_calCircleArea函数时，获得所有输入参数, 后续将执行calCircleArea。
                   //动态调用calCircleArea
                   rc = ((Bound_calCircleArea)func)(&r);
                   printf("rc = %.2f\n",rc);
               }
           }
       }
   /* Deallocate both closure, and bound_calCircleArea */
   ffi_closure_free(closure);   //释放闭包
}
void testCall2(){
    printf("------- testCall2 ------ \n");
    void* functionPtr = &testFunc2;
    int argCount = 2;
    //param types
    ffi_type **ffiArgTypes = alloca(sizeof(ffi_type *) *argCount);
    ffiArgTypes[0] = &ffi_type_pointer;
    ffiArgTypes[1] = &ffi_type_sint;
    //params
    void **ffiArgs = alloca(sizeof(void *) *argCount);
    //void *arg1_ptr = alloca(ffiArgTypes[0]->size);
    int **arg1;
    ffiArgs[0] = &arg1;
    //-- arg 2
    int *arg2_ptr = (int *)alloca(ffiArgTypes[0]->size);
    *arg2_ptr = 3;
    ffiArgs[1] = arg2_ptr;
    //ffi_cif
    //生成函数原型 ffi_cfi 对象
    ffi_cif cif;
    ffi_type *returnFfiType = &ffi_type_sint;
    ffi_status ffiPrepStatus = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, (unsigned int)argCount, returnFfiType, ffiArgTypes);

    if (ffiPrepStatus == FFI_OK) {
        //生成用于保存返回值的内存
        void *returnPtr = NULL;
        if (returnFfiType->size) {
            returnPtr = alloca(returnFfiType->size);
        }
        //根据cif函数原型，函数指针，返回值内存指针，函数参数数据调用这个函数
        ffi_call(&cif, functionPtr, returnPtr, ffiArgs);

        //拿到返回值
        int returnValue = *(int *)returnPtr;
        printf("ret: %d \n", returnValue);
        printf("**m: %d \n", (*arg1)[0]);
        free(*arg1);
    }
}

void testCall() {
    printf("------------ testCall------ \n");
    testFunc(1, 2);

    //拿函数指针
    // int (*funcPointer)(int, int) = &testFunc;
    void* functionPtr = &testFunc;
    int argCount = 2;

    //参数类型数组
    ffi_type **ffiArgTypes = alloca(sizeof(ffi_type *) *argCount);
    ffiArgTypes[0] = &ffi_type_sint;
    ffiArgTypes[1] = &ffi_type_sint;

    //参数数据数组
    void **ffiArgs = alloca(sizeof(void *) *argCount);
    void *ffiArgPtr = alloca(ffiArgTypes[0]->size);
    int *argPtr = ffiArgPtr;
    *argPtr = 5;
    ffiArgs[0] = ffiArgPtr;

    void *ffiArgPtr2 = alloca(ffiArgTypes[1]->size);
    int *argPtr2 = ffiArgPtr2;
    *argPtr2 = 3;
    ffiArgs[1] = ffiArgPtr2;

    //生成函数原型 ffi_cfi 对象
    ffi_cif cif;
    ffi_type *returnFfiType = &ffi_type_sint;
    ffi_status ffiPrepStatus = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, (unsigned int)argCount, returnFfiType, ffiArgTypes);

    if (ffiPrepStatus == FFI_OK) {
        //生成用于保存返回值的内存
        void *returnPtr = NULL;
        if (returnFfiType->size) {
            returnPtr = alloca(returnFfiType->size);
        }
        //根据cif函数原型，函数指针，返回值内存指针，函数参数数据调用这个函数
        ffi_call(&cif, functionPtr, returnPtr, ffiArgs);

        //拿到返回值
        int returnValue = *(int *)returnPtr;
        printf("ret: %d \n", returnValue);
    }
}
