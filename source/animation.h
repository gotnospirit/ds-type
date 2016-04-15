#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <stdint.h>

#include "structs.h"

int init_animations();
void shutdown_animations();

void remove_from_animations(void *);

void process_animations(uint16_t);

void add_simple_animation(void *, animation_type_t, int, int, uint16_t, animation_step_t *);
void add_loop_animation(void *, animation_type_t, int, int, uint16_t, uint8_t, animation_step_t *);

#endif