#ifndef _JSON_WRAPPER_H_
#define _JSON_WRAPPER_H_

typedef void json_wrapper_t;

#include "structs.h"

#ifdef __cplusplus
extern "C" {
#endif

json_wrapper_t * json_new(const char *);
void json_delete(json_wrapper_t *);

int parse_base(json_wrapper_t *, texture_t *);
int parse_level(json_wrapper_t *, level_t *, texture_t *);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif