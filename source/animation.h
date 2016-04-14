#ifndef _ANIMATION_H_
#define _ANIMATION_H_

#include <stdint.h>

#include "structs.h"

int init_animations();
void shutdown_animations();

void remove_from_animations(entity_t *);

void process_animations(uint16_t);

void animation_rollup(entity_t *);
void animation_rolldown(entity_t *);
void animation_rollback(entity_t *);

#endif