#ifndef _TEXTURES_H_
#define _TEXTURES_H_

#include "structs.h"

int init_textures();
void shutdown_textures();

Texture * texture_new(const char *);
int texture_delete(Texture const **);

Texture const * get_texture(const char *);
Frame const * get_frame(Texture const *, int);

#endif