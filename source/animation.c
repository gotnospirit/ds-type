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

static animation_t * find(entity_t * entity)
{
    animation_t * animation = NULL;
    while (list_next(animations, (void **)&animation))
    {
        if (entity == animation->entity)
        {
            return animation;
        }
    }
    return NULL;
}

static int process_animation(animation_t * animation, uint16_t dt)
{
    uint16_t elapsed = animation->elapsed;
    animation_template_t const * template = animation->tpl;
    uint16_t duration = animation->duration;
    int start_at = animation->start;
    int end_at = template->end;
    int loop_at = template->loop;

    elapsed += dt;

    if (elapsed > duration)
    {
        elapsed = duration;
    }

    float progress = animation->ease(elapsed, duration);
    int new_value = (start_at < end_at)
        ? start_at + progress * (end_at - start_at)
        : end_at + (1 - progress) * (start_at - end_at);

    if (new_value != animation->current)
    {
        animation->current = new_value;

        entity_update_sprite(animation->entity, new_value, template->anchor);
    }

    if (elapsed < duration)
    {
        animation->elapsed = elapsed;
        return 1;
    }

    if (loop_at > 0)
    {
        animation->start = loop_at;
        animation->elapsed = 0;

        if (1 == end_at - loop_at)
        {
            animation->ease = short_ease_in;
        }
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

animation_template_t * animation_template_new(const char * name, int start, int end, uint16_t duration, int loop, anchor_t anchor)
{
    if (-1 != loop && (loop < start || loop >= end))
    {
        return NULL;
    }

    animation_template_t * result = (animation_template_t *)list_alloc(templates);
    if (NULL != result)
    {
        result->name = strdup(name);
        result->start = start;
        result->end = end;
        result->loop = loop;
        result->duration = duration;
        result->anchor = anchor;
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

uint16_t animation_new(const char * type, entity_t * entity)
{
    animation_template_t * template = template_get(type);
    if (NULL == template)
    {
        return 0;
    }

    animation_t * animation = find(entity);
    int start = template->start;

    if (NULL != animation)
    {
        if (is_roll(type))
        {
            start = animation->current;
        }
    }
    else
    {
        animation = (animation_t *)list_alloc(animations);
        if (NULL == animation)
        {
            return 0;
        }
    }

    animation->start = -1 != start ? start : 0;
    animation->current = -1;
    animation->elapsed = 0;
    animation->duration = template->duration;
    animation->entity = entity;
    animation->tpl = template;
    animation->ease = linear_ease_in;
    return animation->duration;
}