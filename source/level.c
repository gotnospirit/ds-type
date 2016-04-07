#include <stdlib.h>
#include <stdio.h>

#include <3ds.h>

#include "level.h"
#include "list.h"
#include "input.h"
#include "render.h"
#include "texture.h"

static void update_level_tiles(List * container, uint16_t camera_left, uint16_t camera_right, uint16_t offset_y)
{
    Tile * tile = NULL;
    Sprite * sprite = NULL;
    int x = 0;
    while (list_next(container, (void **)&tile))
    {
        x = tile->x;
        sprite = tile->sprite;

        if ((x >= camera_right) || ((x + sprite->width) < camera_left))
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
            sprite->y = !offset_y ? 0 : offset_y - sprite->height;

            add_to_rendering(sprite);
            tile->visible = 1;
        }
    }
}

static void clear_level_tiles(List ** list)
{
    List * container = *list;
    Tile * tile = NULL;
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

Level * level_new()
{
    Level * level = malloc(sizeof(Level));
    level->camera = 0;
    level->incr = 1;
    level->max_camera = 0;
    // level->elapsed = 0;
    level->texture = NULL;
    level->top_tiles = list_new(sizeof(Tile), 1);
    level->bottom_tiles = list_new(sizeof(Tile), 1);
    return level;
}

void level_delete(Level * level)
{
    unload_texture(level->texture);

    clear_level_tiles(&level->top_tiles);
    clear_level_tiles(&level->bottom_tiles);

    free(level);
}

void level_update(Level * level, Surface const * screen, uint64_t dt)
{
    int incr = 0;
    if (keypressed(KEY_DRIGHT))
    {
        incr = level->incr;
    }
    else if (keypressed(KEY_DLEFT))
    {
        incr = level->incr * -1;
    }
    else if (keypressed(KEY_R))
    {
        ++level->incr;
        if (level->incr > 255)
        {
            level->incr = 255;
        }
    }
    else if (keypressed(KEY_L))
    {
        --level->incr;
        if (level->incr < 1)
        {
            level->incr = 1;
        }
    }

    level->camera += incr;
    if (level->camera < 0)
    {
        level->camera = 0;
    }
    else if (level->camera >= level->max_camera)
    {
        level->camera = level->max_camera;
    }

    uint16_t camera_left = level->camera;
    uint16_t camera_right = camera_left + screen->width;

    update_level_tiles(level->top_tiles, camera_left, camera_right, 0);
    update_level_tiles(level->bottom_tiles, camera_left, camera_right, screen->height);

    printf("\x1b[0;0H %3d, cam: %5d %5d, end: %5d", level->incr, (int)camera_left, (int)camera_right, level->max_camera);

    // level->elapsed += dt;
}