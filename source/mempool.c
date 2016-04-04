#include <stdlib.h>

#include "mempool.h"

static inline void mempool_init(mempool_t * mempool, size_t elem_size, size_t nb_elem)
{
    if (elem_size < sizeof(mempool_elem_t))
    {
        elem_size = sizeof(mempool_elem_t);
    }

    mempool->elem_size = (elem_size + sizeof(int) - 1) & ~(sizeof(int) - 1);
    // mempool->elem_size = elem_size;
    mempool->nb_elem = nb_elem;
    mempool->chunks = NULL;
    mempool->freelist = NULL;
}

static inline void mempool_wipe(mempool_t * mempool)
{
    mempool_chunk_t * i;
    mempool_chunk_t * n;

    i = mempool->chunks;
    while (i)
    {
        n = i->next;
        free(i);
        i = n;
    }

    mempool->chunks = NULL;
    mempool->freelist = NULL;
}

static inline void mempool_alloc(mempool_t * mempool)
{
    size_t size;
    mempool_chunk_t * chunk;
    int i;
    mempool_elem_t * data;

    size = mempool->nb_elem * mempool->elem_size + sizeof(mempool_chunk_t);
    chunk = (mempool_chunk_t *)malloc(size);
    data = (mempool_elem_t *)chunk->data;

    for (i = 0; i < mempool->nb_elem; ++i)
    {
        data->next = mempool->freelist;
        mempool->freelist = data;

        data = (mempool_elem_t *)(data->data + mempool->elem_size);
    }

    chunk->next = mempool->chunks;
    mempool->chunks = chunk;
}

mempool_t * mempool_new(size_t elem_size, size_t nb_elem)
{
    mempool_t * mempool = (mempool_t *)malloc(sizeof(mempool_t));
    if (mempool)
    {
        mempool_init(mempool, elem_size, nb_elem);
    }
    return mempool;
}

void mempool_free(mempool_t * mempool)
{
    if (!mempool)
    {
        return ;
    }

    mempool_wipe(mempool);
    free(mempool);
}

void * mempool_get(mempool_t * mempool)
{
    if (!mempool)
    {
        return NULL;
    }

    mempool_elem_t * data;

    data = mempool->freelist;
    if (data == NULL)
    {
        mempool_alloc(mempool);
        data = mempool->freelist;
    }

    if (data != NULL)
    {
        mempool->freelist = data->next;
    }
    return data;
}

void mempool_put(mempool_t * mempool, void * data)
{
    if (!mempool)
    {
        return ;
    }

    mempool_elem_t * elem;

    elem = (mempool_elem_t *)data;

    elem->next = mempool->freelist;
    mempool->freelist = elem;
}