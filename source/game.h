#ifndef _GAME_STATES_H_
#define _GAME_STATES_H_

#include "structs.h"

void initialize(game_state_t *);
void loading_error(game_state_t *);
void start_level(game_state_t *);
void run_level(game_state_t *);
void stop_level(game_state_t *);
void shutdown(game_state_t *);

#endif