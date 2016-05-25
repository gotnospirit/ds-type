#include <stdio.h>

#include "entity.h"
#include "render.h"
#include "texture.h"
#include "json_wrapper.h"
#include "logic.h"
#include "animation.h"
#include "list.h"
#include "utils.h"
#include "input.h"

static list_t * templates = NULL;
static list_t * hitboxes = NULL;
static list_t * sprites = NULL;
static list_t * entities = NULL;
static list_t * shots = NULL;

static entity_t * charge = NULL;

static uint8_t templates_size = 0;
static uint8_t hitboxes_size = 0;
static uint8_t entities_size = 0;

static uint8_t show_hitbox_debug = 0;

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

static void hitbox_set(const char * name, entity_t * entity)
{
    if (NULL == entity)
    {
        return ;
    }

    hitbox_t * hitbox = NULL;
    while (list_next(hitboxes, (void **)&hitbox))
    {
        if (0 == strcmp(hitbox->name, name))
        {
            entity->hitbox = hitbox;
            return ;
        }
    }
}

static entity_t * entity_free(entity_t * entity)
{
    remove_from_animations(entity);

    entity->type = NULL;
    entity->hitbox = NULL;
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

    result->type = template->name;
    result->x = 0;
    result->y = 0;
    result->width = frame->width;
    result->height = frame->height;
    result->sprite = sprite;
    result->logic = template->logic;
    result->data = NULL;
    result->anchor = template->anchor;
    result->velocity = template->velocity;
    result->newly = 1;
    result->hitbox = NULL;
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

        hitbox_set("ship", entity);

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

static void debug_entities_hitbox(rectangle_t const * camera)
{
    entity_t * entity = NULL;
    hitbox_t const * hitbox = NULL;
    while (list_next(entities, (void **)&entity))
    {
        hitbox = entity->hitbox;

        if (NULL != hitbox)
        {
            render_entity_hitbox(hitbox, entity, camera);
        }
    }
}

int init_entities()
{
    templates = list_new(sizeof(entity_template_t), 1);
    if (NULL == templates)
    {
        return 1;
    }

    hitboxes = list_new(sizeof(hitbox_t), 1);
    if (NULL == hitboxes)
    {
        return 2;
    }

    sprites = list_new(sizeof(sprite_t), 1);
    if (NULL == sprites)
    {
        return 3;
    }

    entities = list_new(sizeof(entity_t), 1);
    if (NULL == entities)
    {
        return 4;
    }

    shots = list_new(sizeof(shot_t), 1);
    if (NULL == shots)
    {
        return 5;
    }

    if (0 != init_animations())
    {
        return 6;
    }

    texture_t * texture = texture_new("base");
    if (NULL == texture)
    {
        return 7;
    }

    json_wrapper_t * json = json_new("base");
    if (NULL == json)
    {
        return 8;
    }
    else if (0 != parse_base(json, texture))
    {
        json_delete(json);
        return 9;
    }
    json_delete(json);

    return (0 != init_ship_entity() || 0 != init_charge_entity())
        ? 10 : 0;
}

void shutdown_entities()
{
    shutdown_animations();

    shot_t * shot = NULL;
    while (list_next(shots, (void **)&shot))
    {
        free(shot->name);
    }
    list_delete(&shots);

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

    hitbox_t * hitbox = NULL;
    while (list_next(hitboxes, (void **)&hitbox))
    {
        free(hitbox->name);
        free(hitbox->points);
    }
    list_delete(&hitboxes);
}

hitbox_t * entity_hitbox_new(const char * name, point_t * points, uint8_t nb_points, hitbox_shape_t shape, anchor_t anchor, rectangle_t boundaries)
{
    hitbox_t * hitbox = (hitbox_t *)list_alloc(hitboxes);
    if (NULL != hitbox)
    {
        hitbox->name = strdup(name);
        hitbox->shape = shape;
        hitbox->points = points;
        hitbox->nb_points = nb_points;
        hitbox->anchor = anchor;
        hitbox->boundaries = boundaries;
        ++hitboxes_size;
    }
    return hitbox;
}

entity_template_t * entity_template_new(const char * name, int current_frame, texture_t const * texture, const char * logic_method, anchor_t anchor, uint8_t velocity)
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
    result->anchor = anchor;
    result->velocity = velocity;

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

shot_t * entity_shot_new(const char * name, int threshold)
{
    shot_t * result = (shot_t *)list_alloc(shots);
    if (NULL != result)
    {
        result->name = strdup(name);
        result->threshold = threshold;
    }
    return result;
}

void entities_logic(rectangle_t const * camera, uint16_t dt)
{
    printf("\x1b[3;0Hent: %2d, tpl: %2d, hbx: %2d, beam %3d%%", entities_size, templates_size, hitboxes_size, ((charge_t *)charge->data)->strength);

    if (pressed(KEY_SELECT))
    {
        show_hitbox_debug = show_hitbox_debug ? 0 : 1;
    }

    logic_t * logic = NULL;
    entity_t * entity = NULL;
    while (list_next(entities, (void **)&entity))
    {
        if (entity->newly)
        {
            entity->newly = 0;
            continue;
        }

        logic = entity->logic;

        if (NULL != logic && !logic(entity, camera))
        {
            entity = entity_free(entity);
        }
    }

    process_animations(dt);
}

void entities_hittest(level_t const * level)
{
    if (show_hitbox_debug)
    {
        debug_entities_hitbox(&level->camera);
    }


    entity_t * entity = NULL;
    while (list_next(entities, (void **)&entity))
    {
        if (NULL != entity->hitbox)
        {
            // collisions with any level's hitbox?
            // collisions with another entity?
        }
    }
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

const char * entity_stop_charge()
{
    remove_from_rendering(charge->sprite);
    remove_from_animations(charge);

    charge_t * info = (charge_t *)charge->data;
    uint8_t strength = info->strength;
    info->strength = 0;

    shot_t * shot = NULL;
    while (list_next(shots, (void **)&shot))
    {
        if (strength > shot->threshold)
        {
            return shot->name;
        }
    }
    return "shot";
}

entity_t * entity_spawn_shot(const char * type)
{
    entity_template_t * template = template_get("shot");
    if (NULL != template)
    {
        entity_t * result = spawn_entity(template);
        if (NULL != result)
        {
            hitbox_set(type, result);

            add_to_rendering(result->sprite);
            return result;
        }
    }
    return NULL;
}

void entity_update_surface(entity_t * entity, uint16_t new_width, uint16_t new_height)
{
    apply_anchor(
        entity->anchor,
        entity->width - new_width,
        entity->height - new_height,
        &entity->x,
        &entity->y
    );

    entity->width = new_width;
    entity->height = new_height;
}

void entity_anchor(entity_t * target, entity_t * base, anchor_t anchor)
{
    if (TOP_LEFT == anchor || TOP_CENTER == anchor || TOP_RIGHT == anchor)
    {
        target->y = base->y;
    }
    else if (BOTTOM_LEFT == anchor || BOTTOM_CENTER == anchor || BOTTOM_RIGHT == anchor)
    {
        target->y = base->y + base->height;
    }
    else
    {
        target->y = base->y + (base->height - target->height) / 2;
    }

    if (TOP_LEFT == anchor || MIDDLE_LEFT == anchor || BOTTOM_LEFT == anchor)
    {
        target->x = base->x;
    }
    else if (TOP_RIGHT == anchor || MIDDLE_RIGHT == anchor || BOTTOM_RIGHT == anchor)
    {
        target->x = base->x + base->width;
    }
    else
    {
        target->x = base->x + (base->width - target->width) / 2;
    }
}