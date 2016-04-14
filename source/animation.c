#include <stdint.h>

#include "animation.h"
#include "list.h"
#include "structs.h"
#include "texture.h"

typedef int animation_find_t(animation_type_t);

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

static int is_roll(animation_type_t type)
{
    return (type == SHIP_ROLL_BACK || type == SHIP_ROLL_DOWN || type == SHIP_ROLL_UP)
        ? 1 : 0;
}

static int is_charge(animation_type_t type)
{
    return (type == SHOT_CHARGE)
        ? 1 : 0;
}

static animation_t * find(entity_t * entity, animation_find_t * fn)
{
    animation_t * animation = NULL;
    while (list_next(container, (void **)&animation))
    {
        if (entity == animation->entity && fn(animation->type))
        {
            return animation;
        }
    }
    return NULL;
}

static void add(entity_t * entity, animation_type_t type, int start, int end, uint16_t duration)
{
    animation_t * animation = NULL;

    if (is_roll(type))
    {
        // Il ne peut y avoir qu'une seul animation pour le déplacement du vaisseau
        animation = find(entity, is_roll);
    }
    else if (is_charge(type))
    {
        // Il ne peut y avoir qu'une seul animation de charge
        animation = find(entity, is_charge);
    }

    if (NULL == animation)
    {
        animation = (animation_t *)list_alloc(container);
        if (NULL != animation)
        {
            animation->entity = entity;
        }
    }

    animation->type = type;
    animation->start = start;
    animation->current = start;
    animation->end = end;
    animation->duration = duration;
    animation->elapsed = 0;
}

static void apply_to_target(animation_t const * animation, int new_value)
{
    int old_value = animation->current;
    entity_t * entity = (entity_t *)animation->entity;

    if (old_value != new_value)
    {
        frame_info_t * info = (frame_info_t *)entity->data;
        info->current_frame = new_value;
        uint8_t start_frame = info->start_frame;

        sprite_t * sprite = entity->sprite;
        sprite->frame = get_frame(sprite->texture, start_frame + new_value);
    }

    if (SHOT_CHARGE == animation->type)
    {
        charge_t * info = (charge_t *)entity->data;

        uint8_t strength = info->strength;
        info->strength = (strength + 2 < 100)
            ? strength + 2 : 100;
    }
}

static int on_animation_end(animation_t * animation)
{
    if (SHOT_CHARGE == animation->type)
    {
        // infinite charge
        animation->current = animation->start;
        animation->elapsed = 0;
        return 1;
    }
    return 0;
}

static int process_animation(animation_t * animation, uint16_t dt)
{
    int new_value = 0;
    int start_at = 0, end_at = animation->end;
    uint16_t elapsed = 0, duration = 0;

    if (animation->current != end_at)
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

        apply_to_target(animation, new_value);

        if (new_value != end_at)
        {
            animation->elapsed = elapsed;
            return 1;
        }
    }
    return on_animation_end(animation);
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

void animation_rollup(entity_t * entity)
{
    frame_info_t * info = (frame_info_t *)entity->data;

    add(
        entity,
        SHIP_ROLL_UP,
        info->current_frame,
        info->nb_frames - 1,
        300
    );
}

void animation_rolldown(entity_t * entity)
{
    frame_info_t * info = (frame_info_t *)entity->data;

    add(
        entity,
        SHIP_ROLL_DOWN,
        info->current_frame,
        0,
        300
    );
}

void animation_rollback(entity_t * entity)
{
    frame_info_t * info = (frame_info_t *)entity->data;

    add(
        entity,
        SHIP_ROLL_BACK,
        info->current_frame,
        info->nb_frames / 2,
        300
    );
}

void animation_charge(entity_t * entity)
{
    frame_info_t * info = (frame_info_t *)entity->data;

    add(
        entity,
        SHOT_CHARGE,
        info->current_frame,
        info->nb_frames - 1,
        600
    );
}

void animation_shot(entity_t * entity)
{
    frame_info_t * info = (frame_info_t *)entity->data;

    add(
        entity,
        SHOT_FIRED,
        info->current_frame,
        info->nb_frames - 1,
        500
    );
}