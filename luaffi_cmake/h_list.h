#ifndef H_LIST_H
#define H_LIST_H

typedef struct array_list{
   void** data;
   int element_count;
   int max_count;
   float factor; //the load factor
}array_list;

#define array_list_new_simple() array_list_new(8, 0.75f)
#define array_list_new2(init_count) array_list_new(init_count, 0.75f)
array_list* array_list_new(int init_count, float factor);

/**
 * @brief array_list_delete: delete/release the list with all memory
 * @param list: the list
 * @param Func: the Func to release element
 * @param ud the userdata
 */
void array_list_delete(array_list* list, void (*Func)(void* ud,void* ele), void* ud);

void array_list_delete2(array_list* list, void (*Func)(void* ele));

void array_list_ensure_capacity(array_list* list, int cap);

int array_list_size(array_list* list);

void array_list_add(array_list* list, void* ele);
/**
 * @brief array_list_remove: remove element from array
 * @param array_list: the list
 * @param ele: the element
 * @param Func: the function to judge the element is the same or not.
 * @return the old data. or NULL if not found
 */
void* array_list_remove(array_list* list, void* ele, int (*Func)(void* ud, void* rawEle, void* pEle), void* ud);

void* array_list_remove_by_index(array_list* list, int index);

/**
 * @brief array_list_get: get element by target index
 * @param list the list
 * @param index the index
 * @return the element if found, or NULL
 */
void* array_list_get(array_list* list, int index);
/**
 * @brief array_list_remove_all: remove element from list by target function
 * @param list: the list
 * @param Func: the function ptr. return 0 if found
 * @param out_list: the remove element list. can be null
 * @return remove count
 */
int array_list_remove_all(array_list* list, int (*Func)(void* ud,int size,int index,void* ele), void* ud, array_list* out_list);
/**
 * @brief array_list_find:find e element by function.
 * @param list the list
 * @param Func: the function ptr. return 0 if found
 * @return the element if found , or NULL
 */
void* array_list_find(array_list* list,int (*Func)(void* ud, int size, int index,void* ele), void* ud);
/**
 * @brief array_list_find_all: found the all match elements
 * @param list the list
 * @param Func the function ptr. return 0 if found
 * @param out_list the out list.
 */
void array_list_find_all(array_list* list,int (*Func)(void* ud, int size, int index,void* ele), void* ud, array_list* out_list);

/**
 * @brief array_list_travel: travel all elements
 * @param list: the list
 */
void array_list_travel(array_list* list,void (*Func)(void* ud, int size, int index,void* ele), void* ud);

#endif // H_LIST_H
