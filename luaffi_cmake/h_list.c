#include <string.h>
#include "h_list.h"
#include "h_alloctor.h"

#define MIN_COUNT 8

static inline void __grow(array_list* list, int minCapacity){
   int oldCapacity = list->max_count;
   int newCapacity = oldCapacity + (oldCapacity >> 1);
   if (newCapacity - minCapacity < 0)
       newCapacity = minCapacity;
   //
    list->max_count = newCapacity;
    void* new_data = REALLOC(list->data, sizeof (void*) * newCapacity);
    if(new_data == NULL){
        void** old = list->data;
        list->data = MALLOC(sizeof (void*) * newCapacity);
        memcpy(list->data, old, sizeof (void*) * oldCapacity);
        FREE(old);
    }else{
        list->data = new_data;
    }
}
void array_list_ensure_capacity(array_list* list, int cap){
    if(list->max_count < cap){
        __grow(list, cap);
    }
}

array_list* array_list_new(int init_count, float factor){
    if(factor > 1 || init_count <= 0){
        return NULL;
    }
    array_list* l = MALLOC(sizeof (array_list));
    l->data = MALLOC(init_count * sizeof (void*));
    l->max_count = init_count;
    l->element_count = 0;
    l->factor = factor;
    return l;
}
void array_list_delete(array_list* list, void (*Func)(void* ud,void* ele), void* ud){
    if(Func){
        for(int i = 0; i < list->element_count; i ++){
            Func(ud, list->data[i]);
        }
    }
    FREE(list->data);
    FREE(list);
}

void array_list_delete2(array_list* list, void (*Func)(void* ele)){
    if(Func){
        for(int i = 0; i < list->element_count; i ++){
            Func(list->data[i]);
        }
    }
    FREE(list->data);
    FREE(list);
}

int array_list_size(array_list* list){
    return list->element_count;
}
void array_list_add(array_list* list, void* ele){
    list->data[list->element_count++] = ele;
    if(list->element_count >= list->factor * list->max_count){
        __grow(list, 0);
    }
}
void* array_list_get(array_list* list, int idx){
    if(idx >= list->element_count){
        return NULL;
    }
    return list->data[idx];
}
void* array_list_remove_by_index(array_list* list, int idx){
    if(idx >= list->element_count){
        return NULL;
    }
    void* old_data = list->data[idx];
    // 3,  0 -> 2  :: 3, 1 -> 1 ::3, 2 -> 0
    int size = sizeof (void*) * (list->element_count - 1 - idx);
    if(size > 0){
        memcpy(list->data + idx * sizeof (void*), list->data + (idx + 1) * sizeof (void*), size);
    }
    list->element_count -- ;
    return old_data;
}
void* array_list_remove(array_list* list, void* ele, int (*Func)(void* ud,void* rawEle, void* pEle), void* ud){
    int idx = -1;
    void* old_data = NULL;
    for(int i = list->element_count - 1; i >=0 ; i --){
        if( (Func && Func(ud, list->data[i], ele) == 0) || list->data[i] == ele){
            idx = i;
            old_data = list->data[i];
            break;
        }
    }
    //remove element
    if( idx >= 0){
        // 3,  0 -> 2  :: 3, 1 -> 1
        int size = sizeof (void*) * (list->element_count - 1 - idx);
        if(size > 0){
            memcpy(list->data + idx * sizeof (void*), list->data + (idx + 1) * sizeof (void*), size);
        }
        list->element_count -- ;
    }
    return old_data;
}
void* array_list_find(array_list* list,int (*Func)(void* ud,int size, int index,void* ele), void* ud){
    for(int i = list->element_count - 1; i >=0 ; i --){
        if( Func(ud, list->element_count, i, list->data[i]) == 0){
            return list->data[i];
        }
    }
    return NULL;
}
void array_list_find_all(array_list* list,int (*Func)(void* ud, int size, int index,void* ele), void* ud, array_list* out_list){
    for(int i = 0; i < list->element_count; i ++){
        if(Func(ud, list->element_count, i, list->data[i]) == 0){
            array_list_add(out_list, list->data[i]);
        }
    }
}
int array_list_remove_all(array_list* list, int (*Func)(void* ud, int size,int index,void* ele), void* ud, array_list* out_list){
    int old_count = list->element_count;
    int remove_count = 0;
    //remove mark
    signed char idxes[old_count];
    memset(idxes, 0, old_count * sizeof (int));
    for(int i = 0 ; i < old_count ; i ++){
        if( Func(ud, old_count, i, list->data[i]) == 0){
            if(out_list){
                array_list_add(out_list, list->data[i]);
            }
            idxes[i] = 1;
            remove_count ++;
        }
    }
    if(remove_count > 0){
        list->element_count = old_count -  remove_count;
        void** startPtr;
        for(int i = 0 ; i < old_count ; i ++){
            //move
            if(idxes[i] == 1){
                int c = 1;
                for(int j = i + 1; j < old_count ; j ++){
                    if(idxes[j] == 1){
                        c ++;
                    }else{
                        break;
                    }
                }
                startPtr = list->data + i * sizeof (void*);
                memcpy(startPtr, startPtr + c * sizeof (void*), c * sizeof (void*));
            }
        }
    }
    return remove_count;
}
void array_list_travel(array_list* list,void (*Func)(void* ud, int size, int index,void* ele), void* ud){
    for(int i = 0; i < list->element_count; i ++){
        Func(ud, list->element_count, i, list->data[i]);
    }
}

