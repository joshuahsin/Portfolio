
#ifndef DATAS_H
#define DATAS_H
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "protocol.h"



// linked list
typedef struct node {
    void* data;
    struct node* next;
} node_t;

typedef struct list {
    node_t* head;
//    int (*comparator)(void*, void*);
//    void (*printer)(void*);
    void (*deleter)(void*);
    int length;
} list_t;

// ((auction_t*)(head->data))->auction_id = 1;

list_t* CreateList (void (*delete)(void*));
void InsertAtHead(list_t* list, void* val_ref);
void InsertAtTail(list_t* list, void* val_ref);
void* removeFront(list_t* list);
void* removeRear(list_t* list);
void* removeByIndex(list_t* list, int index);
void deleteList(list_t* list);
void removeAllFromList(list_t* list);



#define AUCTION_ID 1

typedef struct user{
	char* username;
    char* password;
    int file_descriptor;
    int isActive;
    list_t* auctions_watching;
    list_t* auctions_won;
    list_t* auctions_created;
} user_t;


typedef struct auction{
	int auction_id;
    char* creator;
    char* winner;
    char* item_name;
    int remaining_ticks;
    int isActive;
    int currentBid;
    int bin;
    int num_watchers;
    list_t* user_list;
} auction_t;

typedef struct job{
    uint8_t msg_type;
    char* message;
    int client_fd;
    //petr_header* p_head;
} job_t;


void user_tDeleter(void* val_ref);
void auction_tDeleter(void* val_ref);
void job_tDeleter(void* val_ref);

// typedef struct auction_list{
// 		auction event;
//     struct auction_list *next;
// } auction_list;

// typedef struct user_list{
// 		user person;
//     struct user_list *next;
// } user_list;



#endif