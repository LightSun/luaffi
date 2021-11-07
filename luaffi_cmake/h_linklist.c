#include <string.h>
#include "h_linklist.h"
#include "h_alloctor.h"

/* Creates a list (node) and returns it
 * Arguments: The data the list will contain or NULL to create an empty
 * list/node
 */
linklist_node* linklist_create(void *data)
{
    linklist_node *l = MALLOC(sizeof(linklist_node));
    if (l != NULL) {
        l->next = NULL;
        l->data = data;
    }

    return l;
}

/* Completely destroys a list
 * Arguments: A pointer to a pointer to a list
 */
void linklist_destroy(linklist_node **list)
{
    if (list == NULL) return;
    while (*list != NULL) {
        linklist_remove(list, *list);
    }
}

/* Creates a list node and inserts it after the specified node
 * Arguments: A node to insert after and the data the new node will contain
 */
linklist_node* linklist_insert_after(linklist_node *node, void *data)
{
    linklist_node *new_node = linklist_create(data);
    if (new_node) {
        new_node->next = node->next;
        node->next = new_node;
    }
    return new_node;
}

/* Creates a new list node and inserts it in the beginning of the list
 * Arguments: The list the node will be inserted to and the data the node will
 * contain
 */
linklist_node* linklist_insert_beginning(linklist_node *list, void *data)
{
    linklist_node *new_node = linklist_create(data);
    if (new_node != NULL) { new_node->next = list; }
    return new_node;
}

/* Creates a new list node and inserts it at the end of the list
 * Arguments: The list the node will be inserted to and the data the node will
 * contain
 */
linklist_node* linklist_insert_end(linklist_node *list, void *data)
{
    linklist_node *new_node = linklist_create(data);
    if (new_node != NULL) {
        for(linklist_node *it = list; it != NULL; it = it->next) {
            if (it->next == NULL) {
                it->next = new_node;
                break;
            }
        }
    }
    return new_node;
}

/* Removes a node from the list
 * Arguments: The list and the node that will be removed
 */
void linklist_remove(linklist_node **list, linklist_node *node)
{
    linklist_node *tmp = NULL;
    if (list == NULL || *list == NULL || node == NULL) return;

    if (*list == node) {
        *list = (*list)->next;
        FREE(node);
        node = NULL;
    } else {
        tmp = *list;
        while (tmp->next && tmp->next != node) tmp = tmp->next;
        if (tmp->next) {
            tmp->next = node->next;
            FREE(node);
            node = NULL;
        }
    }
}

/* Removes an element from a list by comparing the data pointers
 * Arguments: A pointer to a pointer to a list and the pointer to the data
 */
void linklist_remove_by_data(linklist_node **list, void *data)
{
    if (list == NULL || *list == NULL || data == NULL) return;
    linklist_remove(list, linklist_find_by_data(*list, data));
}

/* Find an element in a list by the pointer to the element
 * Arguments: A pointer to a list and a pointer to the node/element
 */
linklist_node* linklist_find_node(linklist_node *list, linklist_node *node)
{
    while (list) {
        if (list == node) break;
        list = list->next;
    }
    return list;
}

/* Finds an elemt in a list by the data pointer
 * Arguments: A pointer to a list and a pointer to the data
 */
linklist_node* linklist_find_by_data(linklist_node *list, void *data)
{
    while (list) {
        if (list->data == data) break;
        list = list->next;
    }
    return list;
}

/* Finds an element in the list by using the comparison function
 * Arguments: A pointer to a list, the comparison function and a pointer to the
 * data
 */
linklist_node* linklist_find(linklist_node *list, int(*func)(linklist_node*,void*), void *data)
{
    if (!func) return NULL;
    while(list) {
        if (func(list, data)) break;
        list = list->next;
    }
    return list;
}
