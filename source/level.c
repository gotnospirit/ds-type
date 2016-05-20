#include <stdlib.h>
#include <stdio.h>

#include <3ds.h>

#include "level.h"
#include "json_wrapper.h"
#include "list.h"
#include "input.h"
#include "render.h"
#include "texture.h"
#include "utils.h"

static uint8_t show_hitbox_debug = 0;

static void update_level_tiles(list_t * container, rectangle_t const * camera)
{
    uint16_t camera_left = camera->left, camera_right = camera->right, offset_y = camera->bottom;

    tile_t * tile = NULL;
    sprite_t * sprite = NULL;
    int x = 0;
    while (list_next(container, (void **)&tile))
    {
        x = tile->x;
        sprite = tile->sprite;

        if ((x >= camera_right) || ((x + tile->width) < camera_left))
        {
            if (tile->visible)
            {
                remove_from_rendering(sprite);
                tile->visible = 0;
            }
            continue;
        }

        if (tile->visible)
        {
            sprite->x = x - camera_left;
        }
        else
        {
            sprite->x = x - camera_left;
            sprite->y = (TOP == tile->anchor)
                ? 0 : offset_y - tile->height;

            add_to_rendering(sprite);
            tile->visible = 1;
        }
    }
}

level_t * level_new(const char * name)
{
    texture_t * texture = texture_new(name);
    if (NULL == texture)
    {
        return NULL;
    }

    rectangle_t camera;
    camera.top = 0;
    camera.left = 0;
    camera.right = 0;
    camera.bottom = 0;

    level_t * level = malloc(sizeof(level_t));
    if (NULL == level)
    {
        texture_delete((texture_t const **)&texture);
        return NULL;
    }

    level->camera = camera;
    level->incr = 1;
    level->max_camera_left = 0;
    level->texture = texture;
    level->tiles = list_new(sizeof(tile_t), 1);
    level->hitboxes = list_new(sizeof(hitbox_t), 1);

    json_wrapper_t * json = json_new(name);
    if (NULL == json || 0 != parse_level(json, level, texture))
    {
        level_delete(level);
        return NULL;
    }
    json_delete(json);

    return level;
}

void level_delete(level_t * level)
{
    tile_t * tile = NULL;
    while (list_next(level->tiles, (void **)&tile))
    {
        if (tile->visible)
        {
            remove_from_rendering(tile->sprite);
        }
        free(tile->sprite);
    }
    list_delete(&level->tiles);

    hitbox_t * hitbox = NULL;
    while (list_next(level->hitboxes, (void **)&hitbox))
    {
        free(hitbox->type);
        free(hitbox->points);
    }
    list_delete(&level->hitboxes);

    texture_delete(&level->texture);

    free(level);
}

void level_logic(level_t * level, surface_t const * screen, uint16_t dt)
{
    int incr = 0;
    if (held(KEY_DRIGHT))
    {
        incr = level->incr;
    }
    else if (held(KEY_DLEFT))
    {
        incr = level->incr * -1;
    }
    else if (pressed(KEY_R))
    {
        ++level->incr;
        if (level->incr > 255)
        {
            level->incr = 255;
        }
    }
    else if (pressed(KEY_L))
    {
        --level->incr;
        if (level->incr < 1)
        {
            level->incr = 1;
        }
    }

    if (pressed(KEY_SELECT))
    {
        show_hitbox_debug = show_hitbox_debug ? 0 : 1;
    }

    int32_t camera_left = level->camera.left + incr;
    uint16_t max_camera_left = level->max_camera_left;
    if (camera_left < 0)
    {
        camera_left = 0;
    }
    else if (camera_left >= max_camera_left)
    {
        camera_left = max_camera_left;
    }
    uint16_t camera_right = camera_left + screen->width;
    uint16_t camera_bottom = screen->height;

    level->camera.left = camera_left;
    level->camera.right = camera_right;
    level->camera.top = 0;
    level->camera.bottom = camera_bottom;

    update_level_tiles(level->tiles, &level->camera);

    if (show_hitbox_debug)
    {
        select_hitboxes(level, render_level_hitbox);
    }

    printf("\x1b[0;0Hcamera: %5d %5d %5d %5d", level->camera.top, level->camera.right, level->camera.bottom, level->camera.left);
    printf("\x1b[1;0Hincr: %3d, scroll end: %5d", level->incr, max_camera_left);
}