#ifndef _LEVEL_H_
#define _LEVEL_H_

#include <stdint.h>

#include "structs.h"

Level * level_new();
void level_delete(Level *);

void level_update(Level *, Surface const *, uint64_t);

#endif