#include <stdlib.h>
#include <stdio.h>

#include <3ds.h>

#include "state.h"
#include "json_wrapper.h"
#include "list.h"
#include "structs.h"
#include "input.h"
#include "texture.h"
#include "render.h"
#include "update.h"

static Surface screen;

static List * sprites = NULL;

static int basic_events()
{
    return (aptMainLoop() && read_inputs())
        ? 0 : 1;
}

static void level_one_update()
{
    u64 current_time = osGetTime();

    const char * method = NULL;
    Sprite * sprite = NULL;
    while (list_next(sprites, (void **)&sprite))
    {
        method = sprite->method;
        if (NULL != method)
        {
            if (0 == strncmp(method, "UpdateHero", 10))
            {
                update_hero(sprite, &screen, current_time);
            }
        }
    }
}

void initialize(struct GameState * state)
{
    if (0 != init_rendering(&screen))
    {
        state->next = loading_error;
        return ;
    }

    if (NULL == load_texture("rtype"))
    {
        state->next = loading_error;
        return ;
    }

    if (NULL == load_texture("background"))
    {
        state->next = loading_error;
        return ;
    }

    JsonWrapper * json = json_new("rtype_sprites");
    if (NULL == json || NULL == (sprites = list_new(sizeof(Sprite), 1)) || 0 != load_sprites(json, "rtype", sprites))
    {
        state->next = loading_error;
    }
    else
    {
        printf("\x1b[15;10HPress Start to exit.\n");
        state->next = level_one;
    }
    json_delete(json);
}

void level_one(struct GameState * state)
{
    if (0 != basic_events())
    {
        state->next = shutdown;
    }
    else
    {
        level_one_update();

        render(sprites);
    }
}

void loading_error(struct GameState * state)
{
    if (0 != basic_events())
    {
        state->next = shutdown;
    }

    printf("\x1b[14;12HLoading failed!");
    printf("\x1b[15;10HPress Start to exit.");
}

void shutdown(struct GameState * state)
{
    state->next = NULL;

    Sprite * sprite = NULL;
    while (list_next(sprites, (void **)&sprite))
    {
        free(sprite->name);
        free(sprite->method);
        free(sprite->texture);
    }
    list_delete(sprites);
    sprites = NULL;

    shutdown_rendering();
}