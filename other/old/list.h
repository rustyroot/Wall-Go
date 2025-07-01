#ifndef LIST_H
#define LIST_H LIST_H

#include <SDL3/SDL.h>

struct list_s {
    void* data;
    struct list_s* next;
};

typedef struct list_s list_t;

void add_head (list_t** list_ptr, void* data);
void* cut_head (list_t** list_ptr);
int length (list_t* list);
void destroy (list_t* list);
bool mem(list_t* list, void* elt);

#endif