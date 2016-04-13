#include <stdlib.h>
#include <stdio.h>

#include <3ds.h>

#include "level.h"
#include "list.h"
#include "input.h"
#include "render.h"
#include "texture.h"

static void update_level_tiles(list_t * container, uint16_t camera_left, uint16_t camera_right, uint16_t offset_y)
{
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
            sprite->y = !offset_y ? 0 : offset_y - tile->height;

            add_to_rendering(sprite);
            tile->visible = 1;
        }
    }
}

static void clear_level_tiles(list_t ** list)
{
    list_t * container = *list;
    tile_t * tile = NULL;
    while (list_next(container, (void **)&tile))
    {
        if (tile->visible)
        {
            remove_from_rendering(tile->sprite);
        }
        free(tile->sprite);
    }
    list_delete(list);
}

level_t * level_new()
{
    rectangle_t camera;
    camera.top = 0;
    camera.left = 0;
    camera.right = 0;
    camera.bottom = 0;

    level_t * level = malloc(sizeof(level_t));
    level->camera = camera;
    level->incr = 1;
    level->max_camera_left = 0;
    // level->elapsed = 0;
    level->texture = NULL;
    level->top_tiles = list_new(sizeof(tile_t), 1);
    level->bottom_tiles = list_new(sizeof(tile_t), 1);
    return level;
}

void level_delete(level_t * level)
{
    clear_level_tiles(&level->top_tiles);
    clear_level_tiles(&level->bottom_tiles);

    texture_delete(&level->texture);

    free(level);
}

void level_logic(level_t * level, surface_t const * screen, uint64_t dt)
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

    update_level_tiles(level->top_tiles, camera_left, camera_right, 0);
    update_level_tiles(level->bottom_tiles, camera_left, camera_right, camera_bottom);

    printf("\x1b[0;0Hcamera: %5d %5d %5d %5d", level->camera.top, level->camera.right, level->camera.bottom, level->camera.left);
    printf("\x1b[1;0Hincr: %3d, scroll end: %5d", level->incr, max_camera_left);

    // level->elapsed += dt;
}