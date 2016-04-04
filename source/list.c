#include <stdlib.h>
#include <string.h>

#include "list.h"

List * list_new(size_t item_size, size_t nb_items)
{
    void ** items = malloc(sizeof(void *) * nb_items);
    if (!items)
    {
        return NULL;
    }

    mempool_t * storage = mempool_new(item_size, nb_items);
    if (!storage)
    {
        free(items);
        return NULL;
    }

    List * result = malloc(sizeof(List));
    result->storage = storage;
    result->items = items;
    result->nb_items = 0;
    result->max_items = nb_items;
    return result;
}

void list_delete(List * list)
{
    if (NULL == list)
    {
        return ;
    }

    size_t max = list->nb_items;
    for (size_t i = 0; i < max; ++i)
    {
        mempool_put(list->storage, list->items[i]);
    }

    mempool_free(list->storage);

    free(list->items);

    list->storage = NULL;
    list->items = NULL;
    list->nb_items = 0;
    list->max_items = 0;
}

void * list_alloc(List * list)
{
    if (NULL == list)
    {
        return NULL;
    }

    void * result = mempool_get(list->storage);
    if (NULL == result)
    {
        return NULL;
    }

    // realloc ?
    if (list->nb_items >= list->max_items)
    {
        list->max_items *= 2;

        void * tmp = realloc(list->items, sizeof(void *) * list->max_items);
        if (NULL == tmp)
        {
            mempool_put(list->storage, result);
            return NULL;
        }
        list->items = tmp;
    }

    list->items[list->nb_items] = result;
    ++list->nb_items;
    return result;
}

void list_dealloc(List * list, void * data)
{
    if (NULL == list)
    {
        return ;
    }

    size_t n = 0;
    size_t max = list->nb_items;

    // remove
    for (; n < max; ++n)
    {
        if (list->items[n] == data)
        {
            mempool_put(list->storage, data);
            --list->nb_items;
            --max;
            break;
        }
    }

    // arrange
    memmove(list->items[n], list->items[n + 1], sizeof(void *) * (max - n));
}

int list_next(List const * list, void ** ptr)
{
    if (NULL == list)
    {
        return 0;
    }

    size_t max = list->nb_items;

    if (NULL == *ptr)
    {
        if (!max)
        {
            return 0;
        }

        *ptr = list->items[0];
        return 1;
    }

    max -= 1;
    for (size_t i = 0; i < max; ++i)
    {
        if (list->items[i] == *ptr)
        {
            *ptr = list->items[i + 1];
            return 1;
        }
    }

    *ptr = NULL;
    return 0;
}
