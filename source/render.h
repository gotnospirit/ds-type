#ifndef _RENDER_H_
#define _RENDER_H_

#include "structs.h"

int init_rendering(Surface *);
void shutdown_rendering();

void do_render();

int add_to_rendering(Sprite *);
int remove_from_rendering(Sprite *);

#endif