#include <stdlib.h>
#include <stdio.h>

#include <3ds.h>

#include "game.h"
#include "json_wrapper.h"
#include "list.h"
#include "structs.h"
#include "input.h"
#include "texture.h"
#include "render.h"
#include "level.h"
#include "update.h"

static surface_t screen;

static list_t * entities = NULL;
static entity_t * ship = NULL;

static uint64_t last_time = 0;

static int basic_events()
{
    return (aptMainLoop() && read_inputs())
        ? 0 : 1;
}

static entity_t * get_entity(const char * name)
{
    entity_t * entity = NULL;
    while (list_next(entities, (void **)&entity))
    {
        if (0 == strcmp(entity->name, name))
        {
            return entity;
        }
    }
    return NULL;
}

static int load_base()
{
    texture_t * texture = texture_new("base");
    if (NULL == texture)
    {
        return 1;
    }

    json_wrapper_t * json = json_new("base");
    if (NULL == json)
    {
        return 2;
    }
    else if (0 != parse_base(json, entities, texture))
    {
        json_delete(json);
        return 3;
    }
    json_delete(json);

    ship = get_entity("ship");
    if (NULL == ship)
    {
        return 4;
    }
    return !add_to_rendering(ship->sprite)
        ? 5 : 0;
}

static void keep_inside(entity_t * entity, rectangle_t const * camera)
{
    int y = entity->y;
    int height = entity->height;
    int camera_top = camera->top;
    int camera_bottom = camera->bottom;

    if (y < camera_top)
    {
        entity->y = camera_top;
    }
    else if ((y + height) > camera_bottom)
    {
        entity->y = camera_bottom - height;
    }

    int x = entity->x;
    int width = entity->width;
    int camera_left = camera->left;
    int camera_right = camera->right;

    if (x < camera_left)
    {
        entity->x = camera_left;
    }
    else if ((x + width) > camera_right)
    {
        entity->x = camera_right - width;
    }
}

static void update_sprite_position(entity_t * entity, rectangle_t const * camera)
{
    sprite_t * sprite = entity->sprite;
    sprite->x = entity->x - camera->left;
    sprite->y = entity->y - camera->top;
}

static void game_update(level_t * level)
{
    u64 current_time = osGetTime();
    u64 dt = current_time - last_time;

    // logic
    level_update(level, &screen, dt);

    update_hero(level, ship, dt);

    // collisions
    keep_inside(ship, &level->camera);

    // global to local
    update_sprite_position(ship, &level->camera);

    last_time = current_time;
}

void initialize(game_state_t * state)
{
    if (0 != init_rendering(&screen))
    {
        state->next = loading_error;
        return ;
    }

    entities = list_new(sizeof(entity_t), 1);
    if (NULL == entities)
    {
        state->next = loading_error;
        return ;
    }

    if (0 != load_base())
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
    if (0 != basic_events())
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
    if (0 != basic_events())
    {
        state->next = stop_level;
    }
    else
    {
        game_update((level_t *)state->data);

        process_rendering();
    }
}

void stop_level(game_state_t * state)
{
    level_delete(state->data);
    state->data = NULL;

    // move to next level or credits
    state->next = shutdown;
}

void shutdown(game_state_t * state)
{
    state->next = NULL;

    shutdown_rendering();

    entity_t * entity = NULL;
    while (list_next(entities, (void **)&entity))
    {
        remove_from_rendering(entity->sprite);
        free(entity->name);
        free(entity->sprite);
    }
    list_delete(&entities);
}