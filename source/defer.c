#include "list.h"
#include "defer.h"

static list_t * defers = NULL;

int init_defers()
{
    defers = list_new(sizeof(defer_t), 1);
    if (NULL == defers)
    {
        return 1;
    }
    return 0;
}

void shutdown_defers()
{
    list_delete(&defers);
}

defer_t * defer_new(uint32_t delay, callback_t * callback, payload_t * payload)
{
    defer_t * defer = (defer_t *)list_alloc(defers);
    if (NULL != defer)
    {
        defer->delay = delay;
        defer->callback = callback;
        defer->payload = payload;
        return defer;
    }
    return NULL;
}

int defer_delete(defer_t const * defer)
{
    defer_t * ptr = NULL;
    while (list_next(defers, (void **)&ptr))
    {
        if (ptr == defer)
        {
            list_dealloc(defers, ptr);
            return 1;
        }
    }
    return 0;
}

void defer_tick(uint16_t dt)
{
    defer_t * defer = NULL;
    while (list_next(defers, (void **)&defer))
    {
        defer->delay -= dt;

        if (defer->delay <= 0)
        {
            defer->callback(defer->payload);
            defer = list_dealloc(defers, defer);
        }
    }
}