#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include <stdint.h>
#include <citro3d.h>

#include "list.h"

typedef struct
{
    int x, y;
} point_t;

typedef struct
{
    int top, left, right, bottom;
} rectangle_t;

typedef enum
{
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    MIDDLE_LEFT,
    MIDDLE_CENTER,
    MIDDLE_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT,
    TOP,
    BOTTOM
} anchor_t;

typedef enum
{
    RECTANGLE,
    POLYGON,
    CIRCLE
} hitbox_shape_t;

typedef struct Entity entity_t;
typedef int logic_t(entity_t *, rectangle_t const *);

typedef struct Animation animation_t;
typedef void animation_step_t(animation_t *, int);

typedef struct GameState game_state_t;
typedef void game_state_processor_t(game_state_t *);

typedef float easing_t(int, int);

typedef struct
{
    uint16_t width, height;
} surface_t;

typedef struct
{
    float left, top, right, bottom;
    uint16_t width, height;
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
    texture_t const * texture;
    frame_t const * frame;
    // uint8_t depth
    uint8_t flip_x, flip_y;
} sprite_t;

typedef struct
{
    char * name;
    int start, end, loop;
    uint16_t duration;
    animation_step_t * update;
} animation_template_t;

typedef struct
{
    int x;
    uint16_t width, height;
    uint8_t visible;
    anchor_t anchor;
    sprite_t * sprite;
} tile_t;

typedef struct
{
    rectangle_t camera;
    int incr;
    uint16_t max_camera_left;
    texture_t const * texture;
    list_t * tiles;
    list_t * hitboxes;
} level_t;

typedef struct
{
    int strength;
} charge_t;

typedef struct
{
    char * name;
    uint8_t threshold;
} shot_t;

typedef struct
{
    char * name;
    texture_t const * texture;
    frame_t const * frame;
    logic_t * logic;
    anchor_t anchor;
    uint8_t velocity;
} entity_template_t;

typedef struct
{
    char * name;
    hitbox_shape_t shape;
    point_t * points;
    uint8_t nb_points;
    anchor_t anchor;
} hitbox_t;

struct Animation
{
    int start, current;
    uint16_t elapsed, duration;
    entity_t * entity;
    animation_template_t const * tpl;
    easing_t * ease;
};

struct Entity
{
    const char * type;
    int x, y;
    uint16_t width, height;
    sprite_t * sprite;
    logic_t * logic;
    void * data; // charge_t, ...
    anchor_t anchor;
    uint8_t velocity;
    uint8_t newly;
    hitbox_t const * hitbox;
};

struct GameState
{
    game_state_processor_t * next;
    void * data; // const char *, level_t, ...
};

#endif