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

static void add_animation(void * target, animation_type_t type, int start, int end, uint16_t duration)
{
    animation_t * o = NULL;

    // Il ne peut y avoir qu'une seul animation pour le déplacement du vaisseau
    if (type == SHIP_ROLL_BACK || type == SHIP_ROLL_DOWN || type == SHIP_ROLL_UP)
    {
        animation_t * ptr = NULL;
        while (list_next(container, (void **)&ptr))
        {
            if (target == ptr->target)
            {
                o = ptr;
                break;
            }
        }
    }

    if (NULL == o)
    {
        o = (animation_t *)list_alloc(container);
        o->target = target;
    }

    o->type = type;
    o->start = start;
    o->current = start;
    o->end = end;
    o->duration = duration;
    o->elapsed = 0;
}

static void apply_to_target(animation_t const * animation)
{
    animation_type_t type = animation->type;

    if (type == SHIP_ROLL_BACK || type == SHIP_ROLL_DOWN || type == SHIP_ROLL_UP)
    {
        entity_t * entity = (entity_t *)animation->target;
        sprite_t * sprite = entity->sprite;
        int new_frame = animation->current;

        entity->current_frame = new_frame;
        sprite->frame = get_frame(sprite->texture, entity->start_frame + new_frame);
    }
}

static int process_animation(animation_t * animation, uint64_t dt)
{
    int old_value = animation->current, new_value = 0;
    int start_at = 0, end_at = animation->end;
    uint16_t elapsed = 0, duration = 0;

    if (old_value != end_at)
    {
        elapsed = animation->elapsed;
        duration = animation->duration;
        start_at = animation->start;

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

        if (old_value != new_value)
        {
            animation->current = new_value;
            apply_to_target(animation);
        }

        if (new_value != end_at)
        {
            animation->elapsed = elapsed;
            return 1;
        }
    }
    return 0;
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

void remove_from_animations(void * target)
{
    animation_t * animation = NULL;
    while (list_next(container, (void **)&animation))
    {
        if (target == animation->target)
        {
            animation = list_dealloc(container, animation);
        }
    }
}

void process_animations(uint64_t dt)
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

void animation_rollup(entity_t * entity)
{
    add_animation(
        entity,
        SHIP_ROLL_UP,
        entity->current_frame,
        entity->nb_frames - 1,
        300
    );
}

void animation_rolldown(entity_t * entity)
{
    add_animation(
        entity,
        SHIP_ROLL_DOWN,
        entity->current_frame,
        0,
        300
    );
}

void animation_rollback(entity_t * entity)
{
    add_animation(
        entity,
        SHIP_ROLL_BACK,
        entity->current_frame,
        entity->nb_frames / 2,
        300
    );
}