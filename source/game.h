#ifndef _GAME_STATES_H_
#define _GAME_STATES_H_

#include "structs.h"

void initialize(GameState *);
void loading_error(GameState *);
void start_level(GameState *);
void run_level(GameState *);
void stop_level(GameState *);
void shutdown(GameState *);

#endif