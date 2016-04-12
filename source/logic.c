#include "logic.h"
#include "input.h"
#include "animation.h"

void logic_hero(entity_t * entity)
{
    int const incr = 5;

    float const x_incr = stick_dx() * incr;
    float const y_incr = stick_dy() * incr;

    entity->x += x_incr;
    entity->y += y_incr;

    if (pressed(KEY_UP))
    {
        animation_rollup(entity);
    }
    else if (pressed(KEY_DOWN))
    {
        animation_rolldown(entity);
    }
    else if (released(KEY_UP) || released(KEY_DOWN))
    {
        animation_rollback(entity);
    }
}