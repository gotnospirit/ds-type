#include <stdio.h>

#include "entity.h"
#include "render.h"
#include "texture.h"
#include "json_wrapper.h"
#include "logic.h"
#include "animation.h"
#include "list.h"

static list_t * templates = NULL;
static list_t * sprites = NULL;
static list_t * entities = NULL;

static ship_t * ship = NULL;

static uint8_t templates_size = 0;
static uint8_t entities_size = 0;

static template_t * template_get(const char * name)
{
    template_t * template = NULL;
    while (list_next(templates, (void **)&template))
    {
        if (0 == strcmp(template->name, name))
        {
            return template;
        }
    }
    return NULL;
}

static entity_t * entity_free(entity_t * entity)
{
    remove_from_animations(entity);
    sprite_t * sprite = entity->sprite;
    remove_from_rendering(sprite);
    list_dealloc(sprites, sprite);
    --entities_size;
    return list_dealloc(entities, entity);
}

static entity_t * spawn_entity(template_t const * template)
{
    uint16_t width = template->width;
    uint16_t height = template->height;
    uint8_t start_frame = template->start_frame;
    uint8_t current_frame = template->current_frame;
    texture_t const * texture = template->texture;
    if (NULL == texture)
    {
        return NULL;
    }

    frame_t const * frame = get_frame(texture, start_frame + current_frame);
    if (NULL == frame)
    {
        return NULL;
    }

    sprite_t * sprite = (sprite_t *)list_alloc(sprites);
    if (NULL == sprite)
    {
        return NULL;
    }

    sprite->x = 0;
    sprite->y = 0;
    sprite->width = width;
    sprite->height = height;
    sprite->texture = texture;
    sprite->frame = frame;
    sprite->flip_x = 0;
    sprite->flip_y = 0;

    entity_t * result = (entity_t *)list_alloc(entities);
    if (NULL == result)
    {
        free(sprite);
        return NULL;
    }
    ++entities_size;

    result->x = 0;
    result->y = 0;
    result->width = width;
    result->height = height;
    result->sprite = sprite;
    result->logic = template->logic;
    result->data = NULL;
    return result;
}

static int init_ship()
{
    template_t * template = template_get("ship");
    if (NULL != template)
    {
        entity_t * entity = spawn_entity(template);
        if (NULL == entity)
        {
            return 1;
        }

        ship = malloc(sizeof(ship_t));
        if (NULL == ship)
        {
            return 2;
        }

        ship->start_frame = template->start_frame;
        ship->nb_frames = template->nb_frames;
        ship->current_frame = template->current_frame;

        entity->data = ship;

        add_to_rendering(entity->sprite);
    }
    return 0;
}

int init_entities()
{
    templates = list_new(sizeof(template_t), 1);
    if (NULL == templates)
    {
        return 1;
    }

    sprites = list_new(sizeof(sprite_t), 1);
    if (NULL == sprites)
    {
        return 2;
    }

    entities = list_new(sizeof(entity_t), 1);
    if (NULL == entities)
    {
        return 3;
    }

    texture_t * texture = texture_new("base");
    if (NULL == texture)
    {
        return 4;
    }

    json_wrapper_t * json = json_new("base");
    if (NULL == json)
    {
        return 5;
    }
    else if (0 != parse_base(json, texture))
    {
        json_delete(json);
        return 6;
    }
    json_delete(json);

    if (0 != init_animations())
    {
        return 7;
    }
    return init_ship();
}

void shutdown_entities()
{
    shutdown_animations();

    list_delete(&entities);
    entities_size = 0;

    sprite_t * sprite = NULL;
    while (list_next(sprites, (void **)&sprite))
    {
        remove_from_rendering(sprite);
    }
    list_delete(&sprites);

    template_t * template = NULL;
    while (list_next(templates, (void **)&template))
    {
        free(template->name);
    }
    list_delete(&templates);
    templates_size = 0;
}

void entities_logic(rectangle_t const * camera, uint64_t dt)
{
    printf("\x1b[3;0Hentities: %3d, templates: %3d", entities_size, templates_size);

    logic_method_t * logic = NULL;
    entity_t * entity = NULL;
    while (list_next(entities, (void **)&entity))
    {
        logic = entity->logic;

        if (NULL != logic && !logic(entity, camera))
        {
            entity = entity_free(entity);
        }
    }

    process_animations(dt);
}

void sprites_update(rectangle_t const * camera)
{
    int camera_left = camera->left;
    int camera_top = camera->top;

    sprite_t * sprite = NULL;
    entity_t * entity = NULL;
    while (list_next(entities, (void **)&entity))
    {
        sprite = entity->sprite;
        sprite->x = entity->x - camera_left;
        sprite->y = entity->y - camera_top;
    }
}

template_t * template_new(const char * name, uint16_t width, uint16_t height, uint8_t start_frame, uint8_t nb_frames, uint8_t current_frame, texture_t const * texture, const char * logic_method)
{
    template_t * result = (template_t *)list_alloc(templates);
    result->name = strdup(name);
    result->width = width;
    result->height = height;
    result->start_frame = start_frame;
    result->nb_frames = nb_frames;
    result->current_frame = current_frame;
    result->texture = texture;
    result->logic = NULL;

    if (NULL != logic_method)
    {
        if (0 == strncmp(logic_method, "logic_hero", 10))
        {
            result->logic = logic_hero;
        }
        else if (0 == strncmp(logic_method, "logic_shot", 10))
        {
            result->logic = logic_shot;
        }
        else if (0 == strncmp(logic_method, "logic_charge", 12))
        {
            result->logic = logic_charge;
        }
        else
        {
            printf("Unsupported '%s'\n", logic_method);
            list_dealloc(templates, result);
            return NULL;
        }
    }
    ++templates_size;
    return result;
}

entity_t * entity_spawn_shot(int x, int y)
{
    template_t * template = template_get("shot");
    if (NULL != template)
    {
        entity_t * result = spawn_entity(template);
        if (NULL != result)
        {
            result->x = x;
            result->y = y;

            add_to_rendering(result->sprite);
        }
        return result;
    }
    return NULL;
}