#ifndef _TEXTURES_H_
#define _TEXTURES_H_

#include "structs.h"

int init_textures();
void unload_textures();

Texture * prepare_texture(const char *);
Texture const * load_texture(const char *);
int unload_texture(Texture const *);

Texture const * get_texture(const char *);
Frame const * get_frame(Texture const *, int);

#endif