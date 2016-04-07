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

int load_frames(JsonWrapper *, Texture *);
int load_templates(JsonWrapper *, List *, const char *);
int load_level(JsonWrapper *, Level *, Texture *);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif