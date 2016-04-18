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

static entity_t * charge = NULL;

static uint8_t templates_size = 0;
static uint8_t entities_size = 0;

static entity_template_t * template_get(const char * name)
{
    entity_template_t * template = NULL;
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

    free(entity->data);

    sprite_t * sprite = entity->sprite;
    remove_from_rendering(sprite);
    list_dealloc(sprites, sprite);

    --entities_size;
    return list_dealloc(entities, entity);
}

static entity_t * spawn_entity(entity_template_t const * template)
{
    texture_t const * texture = template->texture;
    if (NULL == texture)
    {
        return NULL;
    }

    sprite_t * sprite = (sprite_t *)list_alloc(sprites);
    if (NULL == sprite)
    {
        return NULL;
    }

    frame_t const * frame = template->frame;

    sprite->x = 0;
    sprite->y = 0;
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
    result->width = frame->width;
    result->height = frame->height;
    result->sprite = sprite;
    result->logic = template->logic;
    result->data = NULL;
    return result;
}

static int init_ship_entity()
{
    entity_template_t * template = template_get("ship");
    if (NULL != template)
    {
        entity_t * entity = spawn_entity(template);
        if (NULL == entity)
        {
            return 1;
        }

        add_to_rendering(entity->sprite);
    }
    return 0;
}

static int init_charge_entity()
{
    entity_template_t * template = template_get("charge");
    if (NULL != template)
    {
        entity_t * entity = spawn_entity(template);
        if (NULL == entity)
        {
            return 1;
        }

        charge_t * info = malloc(sizeof(charge_t));
        if (NULL == info)
        {
            entity_free(entity);
            return 2;
        }
        info->strength = 0;
        entity->data = info;

        charge = entity;
    }
    return 0;
}

int init_entities()
{
    templates = list_new(sizeof(entity_template_t), 1);
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

    if (0 != init_animations())
    {
        return 4;
    }

    texture_t * texture = texture_new("base");
    if (NULL == texture)
    {
        return 5;
    }

    json_wrapper_t * json = json_new("base");
    if (NULL == json)
    {
        return 6;
    }
    else if (0 != parse_base(json, texture))
    {
        json_delete(json);
        return 7;
    }
    json_delete(json);

    return (0 != init_ship_entity() || 0 != init_charge_entity())
        ? 8 : 0;
}

void shutdown_entities()
{
    shutdown_animations();

    entity_t * entity = NULL;
    while (list_next(entities, (void **)&entity))
    {
        entity = entity_free(entity);
    }
    list_delete(&entities);
    entities_size = 0;

    list_delete(&sprites);

    entity_template_t * template = NULL;
    while (list_next(templates, (void **)&template))
    {
        free(template->name);
    }
    list_delete(&templates);
    templates_size = 0;
}

entity_template_t * entity_template_new(const char * name, uint8_t current_frame, texture_t const * texture, const char * logic_method)
{
    entity_template_t * result = (entity_template_t *)list_alloc(templates);
    if (NULL == result)
    {
        return NULL;
    }

    frame_t const * frame = get_frame(texture, current_frame);
    if (NULL == frame)
    {
        return NULL;
    }

    result->name = strdup(name);
    result->texture = texture;
    result->frame = frame;
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

void entities_logic(rectangle_t const * camera, uint16_t dt)
{
    printf("\x1b[3;0H%3d entities, %3d templates, beam %3d%%", entities_size, templates_size, ((charge_t *)charge->data)->strength);

    logic_t * logic = NULL;
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

entity_t * entity_start_charge()
{
    add_to_rendering(charge->sprite);
    return charge;
}

entity_t * entity_get_charge()
{
    return charge;
}

uint8_t entity_stop_charge()
{
    remove_from_rendering(charge->sprite);
    remove_from_animations(charge);

    charge_t * info = (charge_t *)charge->data;
    uint8_t result = info->strength;
    info->strength = 0;

    return result;
}

entity_t * entity_spawn_shot()
{
    entity_template_t * template = template_get("shot");
    if (NULL != template)
    {
        entity_t * result = spawn_entity(template);
        if (NULL != result)
        {
            add_to_rendering(result->sprite);
            return result;
        }
    }
    return NULL;
}