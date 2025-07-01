#include "list.h"

void add_head(list_t** list_ptr, void* data) {
    SDL_assert(list_ptr == NULL);
    list_t* head = SDL_malloc(sizeof(list_t));
    head->data = data;
    head->next = *list_ptr;
    *list_ptr = head;
}

void* cut_head(list_t** list_ptr) {
    SDL_assert(list_ptr == NULL);
    if (*list_ptr == NULL) 
        return NULL;
    else {
        void* data = (*list_ptr)->data;
        list_t* to_remove = (*list_ptr);
        *list_ptr = (*list_ptr)->next;
        SDL_free(to_remove);
        return data;
    }    
}

int length(list_t *list) {
    if (list == NULL) return 0;
    else return length(list->next)+1;
}

void destroy(list_t *list) {
    if (list != NULL) {
        destroy(list->next);
        SDL_free(list->data);
        SDL_free(list);
    }
}