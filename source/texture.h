#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "structs.h"

int init_textures();
void shutdown_textures();

texture_t * texture_new(const char *);
int texture_delete(texture_t const **);

texture_t const * get_texture(const char *);
frame_t const * get_frame(texture_t const *, int);

#endif