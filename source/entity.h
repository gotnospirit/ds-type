#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <stdint.h>

#include "structs.h"

int init_entities();
void shutdown_entities();

entity_template_t * entity_template_new(const char *, int, texture_t const *, const char *, anchor_t, uint8_t);
hitbox_t * entity_hitbox_new(const char *, point_t *, uint8_t, hitbox_shape_t, anchor_t, rectangle_t);
shot_t * entity_shot_new(const char *, int);

void entities_logic(level_t const *, uint16_t);

entity_t const * entity_get(const char *);

const char * entity_stop_charge();

entity_t * entity_spawn_shot(const char *);

void entity_update_surface(entity_t *, uint16_t, uint16_t);

#endif