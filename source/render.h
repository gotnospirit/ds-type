#ifndef _RENDER_H_
#define _RENDER_H_

#include "structs.h"

int init_rendering(surface_t *);
void shutdown_rendering();

void process_rendering();

int add_to_rendering(sprite_t *);
int remove_from_rendering(sprite_t *);

void render_level_hitbox(hitbox_t const *, rectangle_t const *);
void render_entity_hitbox(hitbox_t const *, entity_t const *, rectangle_t const *);

#endif