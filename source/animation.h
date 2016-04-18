#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <stdint.h>

#include "structs.h"

int init_animations();
void shutdown_animations();

animation_template_t * animation_template_new(const char *, int, int, uint16_t, int, const char *);

void remove_from_animations(entity_t *);

void process_animations(uint16_t);

void frame_step(animation_t *, int);
void value_step(animation_t *, int);

void add_animation(const char *, entity_t *);

#endif