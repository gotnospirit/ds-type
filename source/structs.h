#ifndef _STRUCTS_H_
#define _STRUCTS_H_

#include <citro3d.h>

typedef struct
{
    int width, height;
} Surface;

typedef struct
{
    float left, top, right, bottom;
} Frame;

typedef struct
{
    int width, height, real_width, real_height, bpp;
    C3D_Tex ptr;
    Frame const * frames;
} Texture;

typedef struct
{
    int x, y, width, height;
    char * name;
    char * method;
    char * texture;
    uint8_t start_frame;
    uint8_t nb_frames;
    uint8_t current_frame;
} Sprite;

struct GameState;

typedef void StateFunction(struct GameState *);

struct GameState
{
    StateFunction * next;
};

#endif