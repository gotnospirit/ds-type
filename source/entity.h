#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <stdint.h>

#include "structs.h"

int init_entities();
void shutdown_entities();

entity_template_t * entity_template_new(const char *, uint8_t, texture_t const *, const char *);

void entities_logic(rectangle_t const *, uint16_t);
void sprites_update(rectangle_t const *);

entity_t * entity_start_charge();
entity_t * entity_get_charge();
uint8_t entity_stop_charge();

entity_t * entity_spawn_shot();

#endif