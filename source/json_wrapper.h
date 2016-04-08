#ifndef _JSON_WRAPPER_H_
#define _JSON_WRAPPER_H_

typedef void JsonWrapper;

#include "structs.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

JsonWrapper * json_new(const char *);
void json_delete(JsonWrapper *);

int parse_base(JsonWrapper *, List *, Texture *);
int parse_level(JsonWrapper *, Level *, Texture *);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif