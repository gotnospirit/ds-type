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
#include "level.h"
#include "update.h"

static Surface screen;

static List * templates = NULL;
static List * entities = NULL;

static uint64_t last_time = 0;

static int basic_events()
{
    return (aptMainLoop() && read_inputs())
        ? 0 : 1;
}

static Template const * get_template(const char * name)
{
    Template const * template = NULL;
    while (list_next(templates, (void **)&template))
    {
        if (0 == strcmp(template->name, name))
        {
            return template;
        }
    }
    return NULL;
}

static Entity * spawn_entity(Template const * template)
{
    if (NULL != template)
    {
        // printf("spawn '%s' entity\n", template->name);
        Entity * entity = (Entity *)list_alloc(entities);
        entity->x = 0;
        entity->y = 0;
        entity->current_frame = template->initial_frame;
        entity->elapsed = 0;
        entity->tpl = template;
        entity->sprite = NULL;
        return entity;
    }
    return NULL;
}

static Sprite * spawn_entity_sprite(Entity * entity)
{
    if (NULL != entity && NULL == entity->sprite)
    {
        Template const * template = entity->tpl;
        // printf("spawn '%s' sprite\n", template->name);

        Sprite * sprite = malloc(sizeof(Sprite));
        sprite->x = entity->x;
        sprite->y = entity->y;

        sprite->width = template->width;
        sprite->height = template->height;

        sprite->texture = get_texture(template->texture);
        sprite->frame = get_frame(sprite->texture, template->start_frame + entity->current_frame);

        entity->sprite = sprite;
        return sprite;
    }
    return NULL;
}

static int load_ship()
{
    Template const * ship_template = NULL;
    Entity * ship = NULL;
    Sprite * sprite = NULL;

    if (NULL == load_texture("rtype"))
    {
        return 1;
    }
    else if (NULL == (ship_template = get_template("ship")))
    {
        return 2;
    }
    else if (NULL == (ship = spawn_entity(ship_template)))
    {
        return 3;
    }
    else if (NULL == (sprite = spawn_entity_sprite(ship)))
    {
        return 4;
    }
    return !add_to_rendering(sprite)
        ? 5 : 0;
}

static void game_update(Level * level)
{
    u64 current_time = osGetTime();
    u64 dt = current_time - last_time;

    Entity * entity = NULL;
    UpdateFunction * update = NULL;

    // move level
    level_update(level, &screen, dt);

    // move entities
    while (list_next(entities, (void **)&entity))
    {
        update = entity->tpl->update;

        if (NULL != update)
        {
            update(entity, &screen, dt);
        }

        // if entity outside screen -> clean sprite -> clean entity
    }

    // check collisions
    last_time = current_time;
}

void initialize(GameState * state)
{
    if (0 != init_rendering(&screen))
    {
        state->next = loading_error;
        return ;
    }

    if (NULL == load_texture("background"))
    {
        state->next = loading_error;
        state->data = "Background failed...";
        return ;
    }

    templates = list_new(sizeof(Template), 1);
    if (NULL == templates)
    {
        state->next = loading_error;
        return ;
    }

    entities = list_new(sizeof(Entity), 1);
    if (NULL == entities)
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
    else if (0 != load_templates(json, templates, "rtype"))
    {
        state->next = loading_error;
        state->data = "Templates failed...";
        json_delete(json);
        return ;
    }
    json_delete(json);

    if (0 != load_ship())
    {
        state->next = loading_error;
        state->data = "Ship not loaded...";
        return ;
    }

    printf("\x1b[15;10HPress Start to exit.");
    state->next = level_start;
    state->data = "level_one";
}

void loading_error(GameState * state)
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

void level_start(GameState * state)
{
    const char * level_name = state->data;

    if (NULL == level_name)
    {
        state->next = loading_error;
        state->data = "No level...";
        return ;
    }

    Texture * texture = prepare_texture(level_name);
    if (NULL == texture)
    {
        state->next = loading_error;
        state->data = "Level texture failed...";
        return ;
    }

    Level * level = level_new();
    if (NULL == level)
    {
        state->next = loading_error;
        state->data = "Level create failed...";
        return ;
    }

    level->texture = texture;

    JsonWrapper * json = json_new(level_name);
    if (NULL == json)
    {
        level_delete(level);

        state->next = loading_error;
        state->data = "Level data not found...";
        return ;
    }
    else if (0 != load_level(json, level, texture))
    {
        level_delete(level);

        state->next = loading_error;
        state->data = "Level init failed...";
    }
    else
    {
        state->next = level_run;
        state->data = level;
    }
    json_delete(json);
}

void level_run(GameState * state)
{
    if (0 != basic_events())
    {
        state->next = level_stop;
    }
    else
    {
        game_update((Level *)state->data);

        do_render();
    }
}

void level_stop(GameState * state)
{
    level_delete(state->data);
    state->data = NULL;

    // move to next level or credits
    state->next = shutdown;
}

void shutdown(GameState * state)
{
    state->next = NULL;

    shutdown_rendering();

    Entity * entity = NULL;
    while (list_next(entities, (void **)&entity))
    {
        remove_from_rendering(entity->sprite);
        free(entity->sprite);
    }
    list_delete(&entities);

    Template * tpl = NULL;
    while (list_next(templates, (void **)&tpl))
    {
        free(tpl->name);
        free(tpl->texture);
    }
    list_delete(&templates);
}