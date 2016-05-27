#include "logic.h"
#include "input.h"
#include "entity.h"
#include "render.h"
#include "animation.h"
#include "utils.h"

static void keep_inside(entity_t * entity, rectangle_t const * camera, float * dx, float * dy)
{
    int y = entity->y;
    int height = entity->height;
    int camera_top = camera->top;
    int camera_bottom = camera->bottom;

    if ((y + *dy) < camera_top)
    {
        *dy = camera_top - y;
    }
    else if ((y + *dy + height) > camera_bottom)
    {
        *dy = camera_bottom - (y + height);
    }

    int x = entity->x;
    int width = entity->width;
    int camera_left = camera->left;
    int camera_right = camera->right;

    if ((x + *dx) < camera_left)
    {
        *dx = camera_left - x;
    }
    else if ((x + *dx + width) > camera_right)
    {
        *dx = camera_right - (x + width);
    }
}

int logic_hero(entity_t * entity, rectangle_t const * camera)
{
    uint8_t const incr = entity->velocity;

    float dx = stick_dx() * incr;
    float dy = stick_dy() * incr;

    if (dx != 0 || dy != 0)
    {
        // don't let the ship going outside the camera's scope
        keep_inside(entity, camera, &dx, &dy);

        if (dx != 0 || dy != 0)
        {
            entity_move_ship(dx, dy);
        }
    }

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
        entity_spawn_shot();
    }
    return 1;
}

int logic_charge(entity_t * entity, rectangle_t const * camera)
{
    if (pressed(KEY_A))
    {
        entity_start_charge();
    }
    else if (held(KEY_A))
    {
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

    if (released(KEY_A))
    {
        entity_stop_charge();
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