#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include <stdint.h>
#include <citro3d.h>

#include "list.h"

typedef struct Rectangle Rectangle;
typedef struct Surface Surface;
typedef struct Frame Frame;
typedef struct Texture Texture;
typedef struct Sprite Sprite;
typedef struct Entity Entity;
typedef struct Tile Tile;
typedef struct Level Level;
typedef struct GameState GameState;

typedef void StateFunction(GameState *);

struct Rectangle
{
    int top, left, right, bottom;
};

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

struct Sprite
{
    int x, y;
    int width, height;
    Texture const * texture;
    Frame const * frame;
    // uint8_t depth
    uint8_t flip_x, flip_y;
};

struct Entity
{
    char * name;
    int x, y;
    uint16_t width, height;
    uint8_t start_frame;
    uint8_t nb_frames;
    uint8_t current_frame;
    uint16_t elapsed;
    Sprite * sprite;
};

struct Tile
{
    int x;
    uint16_t width, height;
    uint8_t visible;
    Sprite * sprite;
};

struct Level
{
    Rectangle camera;
    // uint16_t elapsed;
    int incr;
    uint16_t max_camera_left;
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