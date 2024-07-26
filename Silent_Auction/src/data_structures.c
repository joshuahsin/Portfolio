#include "data_structures.h"

void user_tDeleter(void* val_ref)
{   
    if (val_ref == NULL) {return;}
    //free(((user_t*)val_ref)->username);
    //free(((user_t*)val_ref)->password);
    removeAllFromList(((user_t*)val_ref)->auctions_watching);
    removeAllFromList(((user_t*)val_ref)->auctions_won);
    removeAllFromList(((user_t*)val_ref)->auctions_created);
    free(val_ref);
    return;
}

void auction_tDeleter(void* val_ref)
{
    if (val_ref == NULL) {return;}
    //free(((auction_t*)val_ref)->creator);
    removeAllFromList(((auction_t*)val_ref)->user_list);
    free(val_ref);
    return;
}

void job_tDeleter(void* val_ref)
{
    if (val_ref == NULL) {return;}
    //free(((job_t*)val_ref)->message);
    //free(((job_t*)val_ref)->p_head);
    free(val_ref);
    return;
}

// Linked list

list_t* CreateList (void (*delete)(void*))
{
    list_t* list = malloc(sizeof(list_t));
    list->deleter = delete;
    list->length = 0;
    list->head = NULL;
    return list;
}

void InsertAtHead(list_t* list, void* val_ref)
{
    if(list == NULL || val_ref == NULL)
        return;
    if (list->length == 0) list->head = NULL;

    node_t** head = &(list->head);
    node_t* new_node;
    new_node = malloc(sizeof(node_t));

    new_node->data = val_ref;

    new_node->next = *head;

    // moves list head to the new node
    *head = new_node;
    list->length++;
}

void InsertAtTail(list_t* list, void* val_ref) {
    if (list == NULL || val_ref == NULL)
        return;
    if (list->length == 0) {
        InsertAtHead(list, val_ref);
        return;
    }

    node_t* head = list->head;
    node_t* current = head;
    while (current->next != NULL) {
        current = current->next;
    }

    current->next = malloc(sizeof(node_t));
    current->next->data = val_ref;
    current->next->next = NULL;
    list->length++;
}

void* removeFront(list_t* list)
{
     node_t** head = &(list->head);
    void* retval = NULL;
    node_t* next_node = NULL;

    if (list->length == 0) {
        return NULL;
    }

    next_node = (*head)->next;
    retval = (*head)->data;
    list->length--;

    node_t* temp = *head;
    *head = next_node;
    free(temp);

    return retval;
}

void* removeRear(list_t* list)
{
    if (list->length == 0) {
        return NULL;
    } else if (list->length == 1) {
        return removeFront(list);
    }

    void* retval = NULL;
    node_t* head = list->head;
    node_t* current = head;

    while (current->next->next != NULL) { 
        current = current->next;
    }

    retval = current->next->data;
    free(current->next);
    current->next = NULL;

    list->length--;

    return retval;
}

void* removeByIndex(list_t* list, int index) {
    if (list->length <= index) {
        return NULL;
    }

    node_t** head = &(list->head);
    void* retval = NULL;
    node_t* current = *head;
    node_t* prev = NULL;
    int i = 0;

    if (index == 0) {
        retval = (*head)->data;
        
		node_t* temp = *head;
        *head = current->next;
        free(temp);
        
		list->length--;
        return retval;
    }

    while (i++ != index) {
        prev = current;
        current = current->next;
    }

    prev->next = current->next;
    retval = current->data;
    free(current);

    list->length--;

    return retval;
}

void deleteList(list_t* list)
{
    if (list->length == 0)
        return;
    while (list->head != NULL){
        void* val_ref = removeFront(list);
        list->deleter(val_ref);
    }
    list->length = 0;
    free(list);
}

void removeAllFromList(list_t* list)
{
    if (list->length == 0)
        return;
    while (list->head != NULL){
        void* val_ref = removeFront(list);
    }
    list->length = 0;
    free(list);
}