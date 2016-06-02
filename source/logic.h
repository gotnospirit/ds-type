#ifndef _LOGIC_H_
#define _LOGIC_H_

#include "structs.h"

int logic_hero(entity_t *, rectangle_t const *);
int logic_charge(entity_t *, rectangle_t const *);
int logic_shot(entity_t *, rectangle_t const *);

void logic_shot_hit_level(entity_t *);

#endif