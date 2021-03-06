#include <stdlib.h>
#include <stdio.h>

#include <3ds.h>

#include "game.h"
#include "structs.h"
#include "input.h"
#include "texture.h"
#include "render.h"
#include "level.h"
#include "entity.h"
#include "logic.h"
#include "defer.h"

static surface_t screen;
static uint64_t last_time = 0;

static int basic_events()
{
    return (aptMainLoop() && read_inputs())
        ? 1 : 0;
}

void initialize(game_state_t * state)
{
    if (0 != init_rendering(&screen))
    {
        state->next = loading_error;
        return ;
    }

    if (0 != init_entities())
    {
        state->next = loading_error;
        return ;
    }

    if (0 != init_defers())
    {
        state->next = loading_error;
        return ;
    }

    printf("\x1b[28;10HPress Start to exit.");
    state->next = start_level;
    state->data = "level_one";
}

void loading_error(game_state_t * state)
{
    if (!basic_events())
    {
        state->next = shutdown;
        state->data = NULL;
        return ;
    }

    const char * text = 0;
    if (NULL == state->data)
    {
        text = "Loading failed!";
    }
    else
    {
        text = (const char *)state->data;
    }

    int len = strlen(text);
    int w = 0;

    if (len < 40)
    {
        w = (40 - len) / 2;
    }
    printf("\x1b[13;%dH%s", w, text);
    printf("\x1b[15;10HPress Start to exit.");
}

void start_level(game_state_t * state)
{
    const char * level_name = state->data;

    if (NULL == level_name)
    {
        state->next = loading_error;
        state->data = "No level...";
        return ;
    }

    level_t * level = level_new(level_name);
    if (NULL == level)
    {
        state->next = loading_error;
        state->data = "Level create failed...";
        return ;
    }

    state->next = run_level;
    state->data = level;
}

void run_level(game_state_t * state)
{
    if (!basic_events())
    {
        state->next = stop_level;
    }
    else
    {
        u64 current_time = osGetTime();
        if (0 == last_time)
        {
            last_time = current_time;
        }

        u16 dt = current_time - last_time;

        defer_tick(dt);

        level_logic(state->data, &screen, dt);

        entities_logic(state->data, dt);

        last_time = current_time;

        process_rendering();
    }
}

void stop_level(game_state_t * state)
{
    level_delete(state->data);

    // move to next level or credits
    state->next = shutdown;
}

void shutdown(game_state_t * state)
{
    state->next = NULL;

    shutdown_defers();
    shutdown_entities();
    shutdown_rendering();
}