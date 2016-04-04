#ifndef _GAME_STATES_H_
#define _GAME_STATES_H_

#include "structs.h"

void initialize(struct GameState *);
void loading_error(struct GameState *);
void level_one(struct GameState *);
void shutdown(struct GameState *);

#endif