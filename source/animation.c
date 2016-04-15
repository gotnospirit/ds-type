#include <stdint.h>

#include "animation.h"
#include "list.h"
#include "structs.h"
#include "easing.h"

typedef int animation_find_t(animation_type_t);

static list_t * container = NULL;

static int is_roll(animation_type_t type)
{
    return (type == SHIP_ROLL_BACK || type == SHIP_ROLL_DOWN || type == SHIP_ROLL_UP)
        ? 1 : 0;
}

static animation_t * find(void * ptr, animation_find_t * fn)
{
    animation_t * animation = NULL;
    while (list_next(container, (void **)&animation))
    {
        if (ptr == animation->data && fn(animation->type))
        {
            return animation;
        }
    }
    return NULL;
}

static int process_animation(animation_t * animation, uint16_t dt)
{
    int new_value = 0;
    uint16_t elapsed = animation->elapsed;
    uint16_t duration = animation->duration;
    int start_at = animation->start;
    int end_at = animation->end;

    elapsed += dt;

    if (elapsed >= duration)
    {
        new_value = end_at;
    }
    else
    {
        float progress = linear_ease_in(elapsed, duration);
        new_value = (start_at < end_at)
            ? start_at + progress * (end_at - start_at)
            : end_at + (1 - progress) * (start_at - end_at);
    }

    animation->update(animation->data, new_value);

    if (new_value != end_at)
    {
        animation->elapsed = elapsed;
        return 1;
    }

    if (animation->loop > 0)
    {
        animation->update(animation->data, animation->start);
        animation->elapsed = 0;
        return 1;
    }
    return 0;
}

animation_t * add(void * ptr, animation_type_t type, int start, int end, uint16_t duration, animation_step_t * callback)
{
    if (start == end)
    {
        return NULL;
    }

    animation_t * animation = NULL;

    // Une animation de déplacement du vaisseau doit remplacer la précédente
    if (is_roll(type))
    {
        animation = find(ptr, is_roll);
    }

    if (NULL == animation)
    {
        animation = (animation_t *)list_alloc(container);
        if (NULL != animation)
        {
            animation->data = ptr;
        }
    }

    animation->type = type;
    animation->start = start;
    animation->end = end;
    animation->duration = duration;
    animation->elapsed = 0;
    animation->loop = 0;
    animation->update = callback;
    return animation;
}

int init_animations()
{
    container = list_new(sizeof(animation_t), 1);
    return (NULL == container)
        ? 1 : 0;
}

void shutdown_animations()
{
    list_delete(&container);
}

void remove_from_animations(void * ptr)
{
    animation_t * animation = NULL;
    while (list_next(container, (void **)&animation))
    {
        if (ptr == animation->data)
        {
            animation = list_dealloc(container, animation);
        }
    }
}

void process_animations(uint16_t dt)
{
    animation_t * animation = NULL;
    while (list_next(container, (void **)&animation))
    {
        if (!process_animation(animation, dt))
        {
            animation = list_dealloc(container, animation);
        }
    }
}

void add_simple_animation(void * ptr, animation_type_t type, int start, int end, uint16_t duration, animation_step_t * callback)
{
    add(ptr, type, start, end, duration, callback);
}

void add_loop_animation(void * ptr, animation_type_t type, int start, int end, uint16_t duration, uint8_t loop, animation_step_t * callback)
{
    animation_t * animation = add(ptr, type, start, end, duration, callback);
    if (NULL != animation)
    {
        animation->loop = loop;
    }
}