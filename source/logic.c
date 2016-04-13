#include "logic.h"
#include "input.h"
#include "entity.h"
#include "animation.h"

static void keep_inside(entity_t * entity, rectangle_t const * camera)
{
    int y = entity->y;
    int height = entity->height;
    int camera_top = camera->top;
    int camera_bottom = camera->bottom;

    if (y < camera_top)
    {
        entity->y = camera_top;
    }
    else if ((y + height) > camera_bottom)
    {
        entity->y = camera_bottom - height;
    }

    int x = entity->x;
    int width = entity->width;
    int camera_left = camera->left;
    int camera_right = camera->right;

    if (x < camera_left)
    {
        entity->x = camera_left;
    }
    else if ((x + width) > camera_right)
    {
        entity->x = camera_right - width;
    }
}

int logic_hero(entity_t * entity, rectangle_t const * camera)
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

    // keep ship inside the camera's scope
    keep_inside(entity, camera);
    return 1;
}

int logic_shot(entity_t * entity, rectangle_t const * camera)
{
    // if outside camera -> delete entity
    if (entity->x > camera->right)
    {
        return 0;
    }

    entity->x += 15;
    return 1;
}

int logic_charge(entity_t * entity, rectangle_t const * camera)
{
    return 1;
}