#include "logic.h"
#include "input.h"
#include "entity.h"
#include "animation.h"

void logic_hero(entity_t * entity)
{
    int const incr = 5;

    entity->x += stick_dx() * incr;
    entity->y += stick_dy() * incr;

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