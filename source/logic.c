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

static const char * get_shot_animation_type(int strength)
{
    const char * result = "shot";

    if (strength > 90)
    {
        result = "fullshot";
    }
    else if (strength >= 60)
    {
        result = "highshot";
    }
    else if (strength >= 40)
    {
        result = "midshot";
    }
    else if (strength > 10)
    {
        result = "lowshot";
    }
    return result;
}

int logic_hero(entity_t * entity, rectangle_t const * camera)
{
    int const incr = 5;

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

    if (pressed(KEY_A))
    {
        entity_t * charge = entity_start_charge();
        if (NULL != charge)
        {
            entity_anchor(charge, entity, MIDDLE_RIGHT);
            add_animation("charge", charge);
            add_animation("beam", charge);
        }
    }
    else if (held(KEY_A))
    {
        entity_t * charge = entity_get_charge();
        if (NULL != charge)
        {
            entity_anchor(charge, entity, MIDDLE_RIGHT);
        }
    }

    if (released(KEY_A))
    {
        uint8_t strength = entity_stop_charge();

        entity_t * shot = entity_spawn_shot();
        if (NULL != shot)
        {
            entity_anchor(shot, entity, MIDDLE_RIGHT);
            add_animation(get_shot_animation_type(strength), shot);
        }
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

    entity->x += 10;
    return 1;
}