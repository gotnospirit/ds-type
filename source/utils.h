#ifndef _UTILS_H_
#define _UTILS_H_

#include "structs.h"

char * read_file(const char *);

int get_hitbox_surface(hitbox_t const *, surface_t *);

void apply_anchor(anchor_t, int, int, int *, int *);

#endif