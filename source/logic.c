#include "logic.h"
#include "input.h"
#include "entity.h"
#include "render.h"
#include "animation.h"
#include "utils.h"

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
    uint8_t const incr = entity->velocity;

    entity->x += stick_dx() * incr;
    entity->y += stick_dy() * incr;

    // keep ship inside the camera's scope
    keep_inside(entity, camera);

    if (pressed(KEY_UP))
    {
        add_animation("rollup", entity);
    }
    else if (pressed(KEY_DOWN))
    {
        add_animation("rolldown", entity);
    }

    if (released(KEY_UP))
    {
        add_animation("rollupback", entity);
    }
    else if (released(KEY_DOWN))
    {
        add_animation("rolldownback", entity);
    }

    if (released(KEY_A))
    {
        const char * type = entity_stop_charge();

        entity_t * shot = entity_spawn_shot(type);
        if (NULL != shot)
        {
            entity_anchor(shot, entity, MIDDLE_RIGHT);
            add_animation(type, shot);
        }
    }
    return 1;
}

int logic_charge(entity_t * entity, rectangle_t const * camera)
{
    if (pressed(KEY_A))
    {
        add_to_rendering(entity->sprite);

        entity_t const * ship = entity_get("ship");
        if (NULL != ship)
        {
            entity_anchor(entity, ship, MIDDLE_RIGHT);
        }
        add_animation("charge", entity);
    }
    else if (held(KEY_A))
    {
        entity_t const * ship = entity_get("ship");
        if (NULL != ship)
        {
            entity_anchor(entity, ship, MIDDLE_RIGHT);

            charge_t * info = (charge_t *)entity->data;
            if (NULL != info)
            {
                uint8_t value = info->strength + entity->velocity;
                if (value > 100)
                {
                    value = 100;
                }

                info->strength = value;
            }
        }
    }

    if (released(KEY_A))
    {
        remove_from_rendering(entity->sprite);
        remove_from_animations(entity);
    }
    return 1;
}

int logic_shot(entity_t * entity, rectangle_t const * camera)
{
    // if outside camera -> delete entity
    if (entity->x > camera->right)
    {
        return 0;
    }

    entity->x += entity->velocity;
    return 1;
}