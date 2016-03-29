#ifndef _TEXTURES_H_
#define _TEXTURES_H_

#include "structs.h"

int load_textures();
void unload_textures();
Texture const * get_texture(const char *);
Frame const * get_frame(Texture const *, int);

#endif