#include <stdlib.h>
#include <string.h>

#include "list.h"

list_t * list_new(size_t item_size, size_t nb_items)
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

    list_t * result = malloc(sizeof(list_t));
    result->storage = storage;
    result->items = items;
    result->nb_items = 0;
    result->max_items = nb_items;
    return result;
}

void list_delete(list_t ** list)
{
    if (NULL == *list)
    {
        return ;
    }

    (*list)->max_items = 0;
    (*list)->nb_items = 0;

    free((*list)->items);

    mempool_free((*list)->storage);

    *list = NULL;
}

void * list_alloc(list_t * list)
{
    if (NULL == list)
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
            return NULL;
        }
        list->items = tmp;
    }

    void * result = mempool_get(list->storage);
    if (NULL == result)
    {
        return NULL;
    }

    list->items[list->nb_items] = result;
    ++list->nb_items;
    return result;
}

void * list_dealloc(list_t * list, void * data)
{
    if (NULL == list)
    {
        return NULL;
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
    memmove(&list->items[n], &list->items[n + 1], sizeof(void *) * (max - n));
    return 0 == n
        ? NULL : list->items[n - 1];
}

int list_next(list_t const * list, void ** ptr)
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