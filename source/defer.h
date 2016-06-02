#ifndef _DEFER_H_
#define _DEFER_H_

#include <stdint.h>

#include "structs.h"

int init_defers();
void shutdown_defers();

defer_t * defer_new(uint32_t, callback_t *, payload_t *);
int defer_delete(defer_t const *);
void defer_tick(uint16_t);

#endif