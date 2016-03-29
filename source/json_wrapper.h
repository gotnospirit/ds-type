#ifndef _JSON_WRAPPER_H_
#define _JSON_WRAPPER_H_

typedef void JsonWrapper;

#include "structs.h"

#ifdef __cplusplus
extern "C" {
#endif

JsonWrapper * json_create(const char *);
void json_delete(JsonWrapper *);

int load_frames(JsonWrapper *, Texture *);
int load_sprites(JsonWrapper *, Sprite **, int *);

#ifdef __cplusplus
} //end extern "C"
#endif

#endif