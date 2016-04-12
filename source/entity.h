#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <stdint.h>

#include "structs.h"

int init_entities();
void shutdown_entities();

sprite_t * sprite_new(int, int, uint16_t, uint16_t, texture_t const *, frame_t const *);
template_t * template_new(const char *, uint16_t, uint16_t, uint8_t, uint8_t, uint8_t, texture_t const *);
entity_t * entity_new(const char *);
int entity_delete(entity_t const **);

void update_sprites(rectangle_t const *);

entity_t * entity_get_ship();

entity_t * entity_spawn_shot(int, int);

#endif