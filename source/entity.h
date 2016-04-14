#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <stdint.h>

#include "structs.h"

int init_entities();
void shutdown_entities();

void entities_logic(rectangle_t const *, uint16_t);
void sprites_update(rectangle_t const *);

template_t * template_new(const char *, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t, texture_t const *, const char *);

entity_t * entity_spawn_shot(int, int);

#endif