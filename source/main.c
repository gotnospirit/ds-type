#include <stdlib.h>

#include "structs.h"
#include "game.h"

int main(int argc, char ** argv)
{
    game_state_t state;
    state.next = initialize;
    state.data = NULL;

    while (state.next)
    {
        state.next(&state);
    }

    return 0;
}