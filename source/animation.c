#include <stdint.h>

#include "animation.h"
#include "list.h"
#include "texture.h"

static list_t * container = NULL;

static float linear_ease_in(int t, int d)
{
    float b = 0.0f;
    float c = 1.0f;
    float result = c * t / d + b;
    if (result > 1)
    {
        result = 1;
    }
    return result;
}

static animation_t * find(entity_t const * entity, animation_type_t const a, animation_type_t const b, animation_type_t const c)
{
    animation_t * animation = NULL;
    while (list_next(container, (void **)&animation))
    {
        if ((a == animation->type || b == animation->type || c == animation->type) && entity == animation->entity)
        {
            return animation;
        }
    }
    return NULL;
}

static animation_t * animation_new(animation_type_t const type, uint16_t duration, entity_t * entity)
{
    animation_t * result = (animation_t *)list_alloc(container);
    if (NULL != result)
    {
        result->type = type;
        result->start = 0;
        result->end = 0;
        result->duration = duration;
        result->elapsed = 0;
        result->entity = entity;
    }
    return result;
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

void process_animations(uint64_t dt)
{
    int old_frame = 0, new_frame = 0;
    int start_at = 0, end_at = 0;
    uint16_t elapsed = 0;
    uint16_t duration = 0;
    sprite_t * sprite = NULL;
    entity_t * entity = NULL;

    animation_t * animation = NULL;
    while (list_next(container, (void **)&animation))
    {
        entity = animation->entity;
        old_frame = entity->current_frame;
        end_at = animation->end;

        if (old_frame != end_at)
        {
            elapsed = animation->elapsed;
            duration = animation->duration;
            start_at = animation->start;

            elapsed += dt;

            if (elapsed >= duration)
            {
                new_frame = end_at;
            }
            else
            {
                float progress = linear_ease_in(elapsed, duration);
                new_frame = (start_at < end_at)
                    ? start_at + progress * (end_at - start_at)
                    : end_at + (1 - progress) * (start_at - end_at);
            }

            if (old_frame != new_frame)
            {
                entity->current_frame = new_frame;

                sprite = entity->sprite;
                sprite->frame = get_frame(sprite->texture, entity->start_frame + new_frame);
            }

            if (new_frame == end_at)
            {
                animation = list_dealloc(container, animation);
            }
            else
            {
                animation->elapsed = elapsed;
            }
        }
    }
}

void animation_rollup(entity_t * entity)
{
    animation_t * o = find(entity, SHIP_ROLL_UP, SHIP_ROLL_DOWN, SHIP_ROLL_BACK);
    if (NULL == o)
    {
        o = animation_new(SHIP_ROLL_UP, 500, entity);
    }

    o->start = entity->current_frame;
    o->end = entity->nb_frames - 1;
    o->elapsed = 0;
}

void animation_rolldown(entity_t * entity)
{
    animation_t * o = find(entity, SHIP_ROLL_DOWN, SHIP_ROLL_UP, SHIP_ROLL_BACK);
    if (NULL == o)
    {
        o = animation_new(SHIP_ROLL_DOWN, 500, entity);
    }

    o->start = entity->current_frame;
    o->end = 0;
    o->elapsed = 0;
}

void animation_rollback(entity_t * entity)
{
    animation_t * o = find(entity, SHIP_ROLL_BACK, SHIP_ROLL_UP, SHIP_ROLL_DOWN);
    if (NULL == o)
    {
        o = animation_new(SHIP_ROLL_BACK, 500, entity);
    }

    o->start = entity->current_frame;
    o->end = entity->nb_frames / 2;
    o->elapsed = 0;
}