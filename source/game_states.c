#include <stdlib.h>
#include <stdio.h>

#include <3ds.h>

#include "game_states.h"
#include "json_wrapper.h"
#include "list.h"
#include "structs.h"
#include "inputs.h"
#include "textures.h"
#include "render.h"
#include "utils.h"

static Surface screen;

static u64 start_time;

static List * sprites = NULL;

static void update_hero(Sprite * sprite, Surface const * screen, uint64_t elapsed)
{
    int incr = 5;

    float x_incr = stick_dx() * incr;
    float y_incr = stick_dy() * incr;

    y_incr *= -1;

    sprite->x += x_incr;
    sprite->y += y_incr;

    sprite->x = clamp(sprite->x, 0, screen->width - sprite->width);
    sprite->y = clamp(sprite->y, 0, screen->height - sprite->height);

    sprite->current_frame = linear_ease_in(elapsed % 1000, 0, sprite->nb_frames, 1000);
}

static int basic_events()
{
    return (aptMainLoop() && read_inputs())
        ? 0 : 1;
}

static void level_one_update()
{
    u64 elapsed = osGetTime() - start_time;

    const char * method = NULL;
    Sprite * sprite = NULL;
    while (list_next(sprites, (void **)&sprite))
    {
        method = sprite->method;
        if (NULL != method)
        {
            if (0 == strncmp(method, "UpdateHero", 10))
            {
                update_hero(sprite, &screen, elapsed);
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
        start_time = osGetTime();
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