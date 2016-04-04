#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

typedef struct mempool_chunk mempool_chunk_t;

struct mempool_chunk
{
    mempool_chunk_t * next;
    char data[0];
};

typedef union mempool_elem mempool_elem_t;

union mempool_elem
{
    mempool_elem_t * next;
    char data[0];
};

typedef struct mempool
{
    size_t elem_size;
    size_t nb_elem;
    mempool_chunk_t * chunks;
    mempool_elem_t * freelist;
} mempool_t;

mempool_t * mempool_new(size_t, size_t);
void mempool_free(mempool_t *);
void * mempool_get(mempool_t *);
void mempool_put(mempool_t *, void *);

#endif /* __MEMPOOL_H__ */