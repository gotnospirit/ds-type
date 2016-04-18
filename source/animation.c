#include <stdint.h>

#include "animation.h"
#include "texture.h"
#include "entity.h"
#include "list.h"
#include "structs.h"
#include "easing.h"

static list_t * templates = NULL;
static list_t * animations = NULL;

static animation_template_t * template_get(const char * name)
{
    animation_template_t * template = NULL;
    while (list_next(templates, (void **)&template))
    {
        if (0 == strcmp(template->name, name))
        {
            return template;
        }
    }
    return NULL;
}

static int is_roll(const char * type)
{
    return (0 == strncmp(type, "rollback", 8) || 0 == strncmp(type, "rolldown", 8) || 0 == strncmp(type, "rollup", 6))
        ? 1 : 0;
}

static animation_t * find_roll(entity_t * entity)
{
    animation_t * animation = NULL;
    while (list_next(animations, (void **)&animation))
    {
        if (entity == animation->entity && is_roll(animation->tpl->name))
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
    animation_template_t const * template = animation->tpl;
    uint16_t duration = animation->duration;
    int start_at = animation->start;
    int end_at = template->end;
    int loop_at = template->loop;
    int diff = end_at - start_at;

    elapsed += dt;

    if (elapsed > duration)
    {
        elapsed = duration;
    }

    if (1 == diff)
    {
        new_value = (elapsed > (duration / 2))
            ? start_at : end_at;
    }
    else
    {
        float progress = linear_ease_in(elapsed, duration);
        new_value = (start_at < end_at)
            ? start_at + progress * (end_at - start_at)
            : end_at + (1 - progress) * (start_at - end_at);
    }

    template->update(animation, new_value);

    if (elapsed < duration)
    {
        animation->elapsed = elapsed;
        return 1;
    }

    if (loop_at > 0)
    {
        animation->start = loop_at;
        animation->elapsed = 0;
        // animation->duration = template->duration / (end_at - template->start + 1) * (end_at - loop_at);
        return 1;
    }
    return 0;
}

int init_animations()
{
    templates = list_new(sizeof(animation_template_t), 1);
    if (NULL == templates)
    {
        return 1;
    }

    animations = list_new(sizeof(animation_t), 1);
    return (NULL == animations)
        ? 2 : 0;
}

void shutdown_animations()
{
    list_delete(&animations);

    animation_template_t * template = NULL;
    while (list_next(templates, (void **)&template))
    {
        free(template->name);
    }
    list_delete(&templates);
}

animation_template_t * animation_template_new(const char * name, int start, int end, uint16_t duration, int loop, const char * update_method)
{
    if (-1 != loop && (loop < start || loop >= end))
    {
        return NULL;
    }

    animation_step_t * callback = NULL;

    if (0 == strncmp(update_method, "frame", 5))
    {
        callback = frame_step;
    }
    else if (0 == strncmp(update_method, "value", 5))
    {
        callback = value_step;
    }
    else
    {
        return NULL;
    }

    animation_template_t * result = (animation_template_t *)list_alloc(templates);
    if (NULL != result)
    {
        result->name = strdup(name);
        result->start = start;
        result->end = end;
        result->duration = duration;
        result->loop = loop;
        result->update = callback;
    }
    return result;
}

void remove_from_animations(entity_t * entity)
{
    animation_t * animation = NULL;
    while (list_next(animations, (void **)&animation))
    {
        if (entity == animation->entity)
        {
            animation = list_dealloc(animations, animation);
        }
    }
}

void process_animations(uint16_t dt)
{
    animation_t * animation = NULL;
    while (list_next(animations, (void **)&animation))
    {
        if (!process_animation(animation, dt))
        {
            animation = list_dealloc(animations, animation);
        }
    }
}

void frame_step(animation_t * animation, int value)
{
    if (value != animation->current)
    {
        animation->current = value;

        entity_t * entity = (entity_t *)animation->entity;
        sprite_t * sprite = entity->sprite;
        if (NULL != sprite)
        {
            frame_t const * frame = get_frame(sprite->texture, value);
            if (NULL != frame)
            {
                sprite->frame = frame;
                entity_update_surface(entity, frame->width, frame->height);
            }
        }
    }
}

void value_step(animation_t * animation, int value)
{
    if (value != animation->current)
    {
        animation->current = value;

        entity_t * entity = (entity_t *)animation->entity;
        charge_t * info = (charge_t *)entity->data;
        if (NULL != info)
        {
            info->strength = value;
        }
    }
}

void add_animation(const char * type, entity_t * entity)
{
    animation_template_t * template = template_get(type);
    if (NULL == template)
    {
        return ;
    }

    animation_t * animation = NULL;
    int start = -1;

    // Une animation de déplacement du vaisseau doit remplacer la précédente
    if (is_roll(type))
    {
        animation = find_roll(entity);
        if (NULL != animation)
        {
            start = animation->current;
        }
    }

    if (NULL == animation)
    {
        animation = (animation_t *)list_alloc(animations);
        if (NULL == animation)
        {
            return ;
        }
        start = template->start;
    }

    animation->start = -1 != start ? start : 0;
    animation->current = -1;
    animation->elapsed = 0;
    animation->duration = template->duration;
    animation->entity = entity;
    animation->tpl = template;
}