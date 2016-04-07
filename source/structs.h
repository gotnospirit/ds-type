#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include <stdint.h>
#include <citro3d.h>

#include "list.h"

typedef struct Surface Surface;
typedef struct Frame Frame;
typedef struct Texture Texture;
typedef struct Template Template;
typedef struct Entity Entity;
typedef struct Sprite Sprite;
typedef struct Tile Tile;
typedef struct Level Level;
typedef struct GameState GameState;

typedef float EaseFunction(int, int);
typedef void UpdateFunction(Entity *, Surface const *, uint64_t);
typedef void StateFunction(GameState *);

struct Surface
{
    uint16_t width, height;
};

struct Frame
{
    float left, top, right, bottom;
};

struct Texture
{
    uint16_t width, height, real_width, real_height;
    uint8_t bpp;
    C3D_Tex ptr;
    char * name;
    Frame const * frames;
};

struct Template
{
    uint16_t width, height;
    char * name;
    char * texture;
    UpdateFunction * update;
    uint8_t start_frame;
    uint8_t nb_frames;
    uint8_t initial_frame;
};

struct Sprite
{
    int x, y;
    uint16_t width, height;
    Texture const * texture;
    Frame const * frame;
    // uint8_t depth
    // uint8_t flip_x, flip_y
};

struct Entity
{
    int x, y;
    uint8_t current_frame;
    uint16_t elapsed;
    Template const * tpl;
    Sprite * sprite;
};

struct Tile
{
    int x;
    uint8_t visible;
    Sprite * sprite;
};

struct Level
{
    int camera;
    // uint16_t elapsed;
    int incr;
    uint16_t max_camera;
    Texture const * texture;
    List * top_tiles;
    List * bottom_tiles;
};

struct GameState
{
    StateFunction * next;
    void * data;
};

#endif