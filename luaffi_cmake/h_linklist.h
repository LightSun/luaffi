#ifndef _H_LINKLIST_H
#define _H_LINKLIST_H

//impl of link list
typedef struct linklist_node {
    void *data;
    struct linklist_node *next;
} linklist_node;

/* linked list */
linklist_node* linklist_create(void *data);
void linklist_destroy(linklist_node **list);
linklist_node* linklist_insert_after(linklist_node *node, void *data);
linklist_node* linklist_insert_beginning(linklist_node *list, void *data);
linklist_node* linklist_insert_end(linklist_node *list, void *data);
void linklist_remove(linklist_node **list, linklist_node *node);
void linklist_remove_by_data(linklist_node **list, void *data);
linklist_node* linklist_find_node(linklist_node *list, linklist_node *node);
linklist_node* linklist_find_by_data(linklist_node *list, void *data);
linklist_node* linklist_find(linklist_node *list, int(*func)(linklist_node*,void*), void *data);

#endif // _H_LINKLIST_H
