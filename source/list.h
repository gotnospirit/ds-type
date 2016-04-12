#ifndef _LIST_H_
#define _LIST_H_

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "mempool.h"

typedef struct
{
    mempool_t * storage;
    void ** items;
    size_t nb_items;
    size_t max_items;
} list_t;

list_t * list_new(size_t, size_t);
void list_delete(list_t **);

void * list_alloc(list_t *);
void * list_dealloc(list_t *, void *);

int list_next(list_t const *, void **);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif