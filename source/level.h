#ifndef _LEVEL_H_
#define _LEVEL_H_

#include <stdint.h>

#include "structs.h"

level_t * level_new();
void level_delete(level_t *);

void level_update(level_t *, surface_t const *, uint64_t);

#endif