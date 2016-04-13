#include "logic.h"
#include "input.h"
#include "entity.h"
#include "animation.h"

int logic_hero(entity_t * entity, rectangle_t const * camera, uint16_t dt)
{
    int const incr = 5;

    entity->x += stick_dx() * incr;
    entity->y += stick_dy() * incr;

    if (pressed(KEY_UP))
    {
        animation_rollup(entity);
    }
    else if (pressed(KEY_DOWN))
    {
        animation_rolldown(entity);
    }
    else if (released(KEY_UP) || released(KEY_DOWN))
    {
        animation_rollback(entity);
    }

    if (pressed(KEY_A))
    {
        entity_t * shot = entity_spawn_shot(entity->x, entity->y);
        if (NULL != shot)
        {
            shot->x += entity->width;
            shot->y += (entity->height + shot->height) / 2;
        }
    }
    return 1;
}

int logic_shot(entity_t * entity, rectangle_t const * camera, uint16_t dt)
{
    // if outside camera -> delete entity
    if (entity->x > camera->right)
    {
        return 0;
    }

    entity->x += 15;
    return 1;
}