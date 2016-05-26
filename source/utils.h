#ifndef _UTILS_H_
#define _UTILS_H_

#include "structs.h"

char * read_file(const char *);

void entity_anchor(entity_t *, entity_t const *, anchor_t const);
void apply_anchor(anchor_t, int, int, int *, int *);

#endif