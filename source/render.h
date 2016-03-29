#ifndef _RENDER_H_
#define _RENDER_H_

#include "structs.h"

int init_rendering(Surface *);
void render(Sprite const *, int);
void shutdown_rendering();

#endif