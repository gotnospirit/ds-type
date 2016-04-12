#ifndef _ENTITY_H_
#define _ENTITY_H_

#include "structs.h"

int init_entities();
void shutdown_entities();

entity_t * entity_get(const char *);

#endif