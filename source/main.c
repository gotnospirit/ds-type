#include <stdlib.h>

#include "structs.h"
#include "state.h"

int main(int argc, char ** argv)
{
    struct GameState state;
    state.next = initialize;
    state.data = NULL;

    while (state.next)
    {
        state.next(&state);
    }

    return 0;
}
