#include "logic.h"
#include "input.h"
#include "entity.h"
#include "animation.h"

#define ALIGN_RIGHT(target, ref) target->x = ref->x + ref->width;
#define VALIGN_MIDDLE(target, ref) target->y = ref->y + (ref->height - target->height) / 2;
#define STICK_TO_SHIP_NOSE(target, ref) \
ALIGN_RIGHT(target, ref) \
VALIGN_MIDDLE(target, ref)

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

static void entity_frame_step(void * entity, int value)
{
    entity_update_frame((entity_t *)entity, value);
}

static void beam_step(void * entity, int value)
{
    charge_t * info = (charge_t *)((entity_t *)entity)->data;
    info->strength = value;
}

static void rollup(entity_t * entity)
{
    animation_info_t * info = (animation_info_t *)entity->data;

    add_simple_animation(entity, SHIP_ROLL_UP, info->current_frame, info->nb_frames - 1, 300, entity_frame_step);
}

static void rolldown(entity_t * entity)
{
    animation_info_t * info = (animation_info_t *)entity->data;

    add_simple_animation(entity, SHIP_ROLL_DOWN, info->current_frame, 0, 300, entity_frame_step);
}

static void rollback(entity_t * entity)
{
    animation_info_t * info = (animation_info_t *)entity->data;

    add_simple_animation(entity, SHIP_ROLL_BACK, info->current_frame, info->nb_frames / 2, 300, entity_frame_step);
}

static void charging(entity_t * entity)
{
    charge_t * info = (charge_t *)entity->data;

    add_loop_animation(entity, CHARGING, info->current_frame, info->nb_frames - 1, 600, 1, entity_frame_step);
    add_simple_animation(entity, BEAM, 0, 100, 2000, beam_step);
}

static void fire(entity_t * entity)
{
    frame_info_t * info = (frame_info_t *)entity->data;

    add_simple_animation(entity, SHOT, info->current_frame, info->nb_frames - 1, 500, entity_frame_step);
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
        rollup(entity);
    }
    else if (pressed(KEY_DOWN))
    {
        rolldown(entity);
    }
    else if (released(KEY_UP) || released(KEY_DOWN))
    {
        rollback(entity);
    }

    if (pressed(KEY_A))
    {
        entity_t * charge = entity_start_charge();
        if (NULL != charge)
        {
            STICK_TO_SHIP_NOSE(charge, entity)
            charging(charge);
        }
    }
    else if (held(KEY_A))
    {
        entity_t * charge = entity_get_charge();
        if (NULL != charge)
        {
            STICK_TO_SHIP_NOSE(charge, entity)
        }
    }
    else if (released(KEY_A))
    {
        entity_t * shot = entity_stop_charge();
        if (NULL != shot)
        {
            STICK_TO_SHIP_NOSE(shot, entity)
            fire(shot);
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

    entity->x += 15;
    return 1;
}

#undef ALIGN_RIGHT
#undef VALIGN_MIDDLE
#undef STICK_TO_SHIP_NOSE