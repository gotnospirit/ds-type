#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include <stdint.h>
#include <citro3d.h>

typedef struct Surface Surface;
typedef struct Frame Frame;
typedef struct Texture Texture;
typedef struct Template Template;
typedef struct Sprite Sprite;
typedef struct GameState GameState;

typedef float EaseFunction(int, int);
typedef void UpdateFunction(Sprite *, uint8_t, uint32_t, uint32_t, uint64_t);
typedef void StateFunction(GameState *);

struct Surface
{
    int width, height;
};

struct Frame
{
    float left, top, right, bottom;
};

struct Texture
{
    int width, height, real_width, real_height, bpp;
    C3D_Tex ptr;
    char * name;
    Frame const * frames;
};

struct Template
{
    int width, height;
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
    uint8_t current_frame;
    uint64_t timestamp;
    Template const * tpl;
};

struct GameState
{
    StateFunction * next;
};

#endif