#ifndef _TEXTURES_H_
#define _TEXTURES_H_

#include "structs.h"

int init_textures();
void unload_textures();

Texture const * load_texture(const char *);

Texture const * get_texture(const char *);
Frame const * get_frame(Texture const *, int);

#endif