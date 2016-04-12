#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include <stdint.h>
#include <citro3d.h>

#include "list.h"

typedef struct
{
    int top, left, right, bottom;
} rectangle_t;

typedef struct
{
    uint16_t width, height;
} surface_t;

typedef struct
{
    float left, top, right, bottom;
} frame_t;

typedef struct
{
    uint16_t width, height, real_width, real_height;
    uint8_t bpp;
    C3D_Tex ptr;
    char * name;
    frame_t const * frames;
} texture_t;

typedef struct
{
    int x, y;
    int width, height;
    texture_t const * texture;
    frame_t const * frame;
    // uint8_t depth
    uint8_t flip_x, flip_y;
} sprite_t;

typedef struct
{
    char * name;
    int x, y;
    uint16_t width, height;
    uint8_t start_frame;
    uint8_t nb_frames;
    uint8_t current_frame;
    uint16_t elapsed;
    sprite_t * sprite;
} entity_t;

typedef struct
{
    int x;
    uint16_t width, height;
    uint8_t visible;
    sprite_t * sprite;
} tile_t;

typedef struct
{
    rectangle_t camera;
    // uint16_t elapsed;
    int incr;
    uint16_t max_camera_left;
    texture_t const * texture;
    list_t * top_tiles;
    list_t * bottom_tiles;
} level_t;

typedef struct GameState game_state_t;
typedef void game_state_processor_t(game_state_t *);

struct GameState
{
    game_state_processor_t * next;
    void * data;
};

#endif