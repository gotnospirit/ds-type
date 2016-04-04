#ifndef _LIST_H_
#define _LIST_H_

#include <stdlib.h>

#include "mempool.h"

typedef struct
{
    mempool_t * storage;
    void ** items;
    size_t nb_items;
    size_t max_items;
} List;

List * list_new(size_t, size_t);
void list_delete(List *);

void * list_alloc(List *);
void list_dealloc(List *, void *);

int list_next(List const *, void **);

#endif