#include <stdlib.h>
#include <stdio.h>

#include <3ds.h>

#include "game.h"
#include "json_wrapper.h"
#include "structs.h"
#include "input.h"
#include "texture.h"
#include "render.h"
#include "level.h"
#include "entity.h"
#include "logic.h"

static surface_t screen;

static uint64_t last_time = 0;

static int basic_events()
{
    return (aptMainLoop() && read_inputs())
        ? 1 : 0;
}

static void game_update(level_t * level)
{
    u64 current_time = osGetTime();
    u64 dt = current_time - last_time;

    // apply game logic
    level_logic(level, &screen, dt);

    entities_logic(&level->camera, dt);

    // collision

    // update entities' sprite's coordinates
    update_sprites(&level->camera);

    last_time = current_time;
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

    entity_t * ship = entity_get_ship();
    if (NULL == ship)
    {
        state->next = loading_error;
        return ;
    }
    else if (!add_to_rendering(ship->sprite))
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

    texture_t * texture = texture_new(level_name);
    if (NULL == texture)
    {
        state->next = loading_error;
        state->data = "Level texture failed...";
        return ;
    }

    level_t * level = level_new();
    if (NULL == level)
    {
        state->next = loading_error;
        state->data = "Level create failed...";
        return ;
    }

    level->texture = texture;

    json_wrapper_t * json = json_new(level_name);
    if (NULL == json)
    {
        level_delete(level);

        state->next = loading_error;
        state->data = "Level data not found...";
        return ;
    }
    else if (0 != parse_level(json, level, texture))
    {
        level_delete(level);

        state->next = loading_error;
        state->data = "Level init failed...";
    }
    else
    {
        state->next = run_level;
        state->data = level;
    }
    json_delete(json);
}

void run_level(game_state_t * state)
{
    if (!basic_events())
    {
        state->next = stop_level;
    }
    else
    {
        game_update(state->data);

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

    shutdown_entities();
    shutdown_rendering();
}