#include <stdint.h>

#include "animation.h"
#include "list.h"
#include "structs.h"
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

static void add_animation(entity_t * entity, animation_type_t type, int start, int end, uint16_t duration)
{
    animation_t * animation = NULL;

    // Il ne peut y avoir qu'une seul animation pour le déplacement du vaisseau
    if (type == SHIP_ROLL_BACK || type == SHIP_ROLL_DOWN || type == SHIP_ROLL_UP)
    {
        animation_t * ptr = NULL;
        while (list_next(container, (void **)&ptr))
        {
            if (entity == ptr->entity)
            {
                animation = ptr;
                break;
            }
        }
    }

    if (NULL == animation)
    {
        animation = (animation_t *)list_alloc(container);
        animation->entity = entity;
    }

    animation->type = type;
    animation->start = start;
    animation->current = start;
    animation->end = end;
    animation->duration = duration;
    animation->elapsed = 0;
}

static void apply_to_target(animation_t const * animation)
{
    animation_type_t type = animation->type;

    if (type == SHIP_ROLL_BACK || type == SHIP_ROLL_DOWN || type == SHIP_ROLL_UP)
    {
        int new_frame = animation->current;

        entity_t * entity = (entity_t *)animation->entity;
        ship_t * ship = (ship_t *)entity->data;
        ship->current_frame = new_frame;

        sprite_t * sprite = entity->sprite;
        sprite->frame = get_frame(sprite->texture, ship->start_frame + new_frame);
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

void remove_from_animations(entity_t * entity)
{
    animation_t * animation = NULL;
    while (list_next(container, (void **)&animation))
    {
        if (entity == animation->entity)
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
    ship_t * ship = (ship_t *)entity->data;

    add_animation(
        entity,
        SHIP_ROLL_UP,
        ship->current_frame,
        ship->nb_frames - 1,
        300
    );
}

void animation_rolldown(entity_t * entity)
{
    ship_t * ship = (ship_t *)entity->data;

    add_animation(
        entity,
        SHIP_ROLL_DOWN,
        ship->current_frame,
        0,
        300
    );
}

void animation_rollback(entity_t * entity)
{
    ship_t * ship = (ship_t *)entity->data;

    add_animation(
        entity,
        SHIP_ROLL_BACK,
        ship->current_frame,
        ship->nb_frames / 2,
        300
    );
}