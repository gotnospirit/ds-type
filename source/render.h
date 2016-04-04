#ifndef _RENDER_H_
#define _RENDER_H_

#include "structs.h"
#include "list.h"

int init_rendering(Surface *);
void render(List const *);
void shutdown_rendering();

#endif