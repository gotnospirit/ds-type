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

static List * templates = NULL;
static List * sprites = NULL;

static int basic_events()
{
    return (aptMainLoop() && read_inputs())
        ? 0 : 1;
}

static int spawn_sprite(const char * name)
{
    Template const * tpl = NULL;
    while (list_next(templates, (void **)&tpl))
    {
        if (0 == strcmp(tpl->name, name))
        {
            break;
        }
    }

    if (NULL != tpl)
    {
        Sprite * sprite = (Sprite *)list_alloc(sprites);
        sprite->x = 0;
        sprite->y = 0;
        sprite->current_frame = tpl->initial_frame;
        sprite->timestamp = 0;
        sprite->tpl = tpl;
        return 1;
    }
    return 0;
}

static void level_one_update()
{
    u64 current_time = osGetTime();

    Sprite * sprite = NULL;
    Template const * tpl = NULL;
    UpdateFunction * update = NULL;

    while (list_next(sprites, (void **)&sprite))
    {
        tpl = sprite->tpl;
        update = tpl->update;
        if (NULL != update)
        {
            update(sprite, tpl->nb_frames, screen.width - tpl->width, screen.height - tpl->height, current_time);
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

    templates = list_new(sizeof(Template), 1);
    if (NULL == templates)
    {
        state->next = loading_error;
        return ;
    }

    sprites = list_new(sizeof(Sprite), 1);
    if (NULL == sprites)
    {
        state->next = loading_error;
        return ;
    }

    JsonWrapper * json = json_new("sprites");
    if (NULL == json)
    {
        state->next = loading_error;
        return ;
    }
    else if (0 != load_templates(json, templates))
    {
        state->next = loading_error;
        json_delete(json);
        return ;
    }
    json_delete(json);

    if (!spawn_sprite("ship"))
    {
        state->next = loading_error;
        return ;
    }

    printf("\x1b[15;10HPress Start to exit.\n");
    state->next = level_one;
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

    list_delete(sprites);
    sprites = NULL;

    Template * tpl = NULL;
    while (list_next(templates, (void **)&tpl))
    {
        free(tpl->name);
        free(tpl->texture);
    }
    list_delete(templates);
    templates = NULL;

    shutdown_rendering();
}