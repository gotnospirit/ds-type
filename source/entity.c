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

#define BROAD_HIT_TEST(r1, r2) (((r1.right >= r2.left) && (r1.left <= r2.right)) && ((r1.bottom >= r2.top) && (r1.top <= r2.bottom)))

static list_t * templates = NULL;
static list_t * hitboxes = NULL;
static list_t * sprites = NULL;
static list_t * entities = NULL;
static list_t * shots = NULL;

static entity_t * ship = NULL;
static entity_t * charge = NULL;

static uint8_t templates_size = 0;
static uint8_t hitboxes_size = 0;
static uint8_t entities_size = 0;

static uint8_t show_hitbox_debug = 0;
static uint8_t lowest_charge_threshold = 255;

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
    sprite->visible = 0;

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

        ship = entity;
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

static uint8_t entity_collides_level(entity_t const * entity, hitbox_t const * level_hitbox, rectangle_t const * camera)
{
    uint8_t result = 0;

    // broad phase

    //  get a rectangle for entity's hitbox
    hitbox_t const * entity_hitbox = entity->hitbox;
    rectangle_t entity_hitbox_bounds, level_hitbox_bounds;
    rectangle_t const * boundaries = &entity_hitbox->boundaries;
    int x = entity->x, y = entity->y;

    entity_hitbox_bounds.left = x + boundaries->left;
    entity_hitbox_bounds.right = x + boundaries->right;
    entity_hitbox_bounds.top = y + boundaries->top;
    entity_hitbox_bounds.bottom = y + boundaries->bottom;

    if (BOTTOM == level_hitbox->anchor)
    {
        //  get a rectangle for level's hitbox
        boundaries = &level_hitbox->boundaries;

        level_hitbox_bounds.left = boundaries->left;
        level_hitbox_bounds.right = boundaries->right;
        level_hitbox_bounds.top = camera->bottom - boundaries->top;
        level_hitbox_bounds.bottom = camera->bottom - boundaries->bottom;

        level_hitbox_bounds.top ^= level_hitbox_bounds.bottom;
        level_hitbox_bounds.bottom ^= level_hitbox_bounds.top;
        level_hitbox_bounds.top ^= level_hitbox_bounds.bottom;

        if (BROAD_HIT_TEST(entity_hitbox_bounds, level_hitbox_bounds))
        {
            result = 1;
        }
    }
    else if (BROAD_HIT_TEST(entity_hitbox_bounds, level_hitbox->boundaries))
    {
        result = 1;
    }

    if (result)
    {
        // need narrow phase?
        if (RECTANGLE != entity_hitbox->shape || RECTANGLE != level_hitbox->shape)
        {
            // result = 0;
            printf("\x1b[2;0Hdo perform SAT");
        }
        else
        {
            printf("\x1b[2;0Hcollides with level");
        }
    }
    return result;
}

static void entities_hittest(level_t const * level)
{
    rectangle_t const * camera = &level->camera;

    if (show_hitbox_debug)
    {
        debug_entities_hitbox(camera);
    }

    printf("\x1b[2;0H                   ");

    entity_t * entity = NULL;
    hitbox_t * hitbox = NULL;
    while (list_next(entities, (void **)&entity))
    {
        if (NULL != entity->hitbox)
        {
            // collisions with any level's hitbox?
            while (list_next(level->hitboxes, (void **)&hitbox))
            {
                if (entity_collides_level(entity, hitbox, camera))
                {
                    // react !
                    break;
                }
            }

            // collisions with another entity?
        }
    }
}

static void sprites_update(rectangle_t const * camera)
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

    ship = NULL;
    charge = NULL;

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

entity_template_t * entity_template_new(const char * name, int current_frame, texture_t const * texture, const char * logic_method, uint8_t velocity)
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

shot_t * entity_shot_new(const char * name, int threshold)
{
    shot_t * result = (shot_t *)list_alloc(shots);
    if (NULL != result)
    {
        result->name = strdup(name);
        result->threshold = threshold;

        if (threshold < lowest_charge_threshold)
        {
            lowest_charge_threshold = threshold;
        }
    }
    return result;
}

void entities_logic(level_t const * level, uint16_t dt)
{
    printf("\x1b[3;0Hent: %2d, tpl: %2d, hbx: %2d, beam %3d%%", entities_size, templates_size, hitboxes_size, ((charge_t *)charge->data)->strength);

    rectangle_t const * camera = &level->camera;

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

    // collision
    entities_hittest(level);

    process_animations(dt);

    // reflect entities' updates to their sprite
    sprites_update(camera);
}

entity_t const * entity_get(const char * type)
{
    if (0 == strncmp(type, "ship", 4))
    {
        return ship;
    }
    else if (0 == strncmp(type, "charge", 6))
    {
        return charge;
    }
    return NULL;
}

void entity_spawn_shot()
{
    const char * type = "shot";

    charge_t * info = (charge_t *)charge->data;
    int strength = info->strength;
    info->strength = 0;

    shot_t * shot = NULL;
    while (list_next(shots, (void **)&shot))
    {
        if (strength > shot->threshold)
        {
            type = shot->name;
            break;
        }
    }

    entity_template_t * template = template_get("shot");
    if (NULL != template)
    {
        entity_t * entity = spawn_entity(template);
        if (NULL != entity)
        {
            hitbox_set(type, entity);

            add_to_rendering(entity->sprite);

            entity_anchor(entity, ship, MIDDLE_RIGHT);
            add_animation(type, entity);
        }
    }
}

void entity_move_ship(float dx, float dy)
{
    ship->x += dx;
    ship->y += dy;

    charge->x += dx;
    charge->y += dy;
}

void entity_increment_charge()
{
    charge_t * info = (charge_t *)charge->data;
    if (NULL != info)
    {
        uint8_t value = info->strength + charge->velocity;
        if (value >= lowest_charge_threshold && add_to_rendering(charge->sprite))
        {
            entity_anchor(charge, ship, MIDDLE_RIGHT);
            add_animation("charge", charge);
        }

        if (value > 100)
        {
            value = 100;
        }

        info->strength = value;
    }
}

void entity_stop_charge()
{
    remove_from_rendering(charge->sprite);
    remove_from_animations(charge);
}

void entity_update_sprite(entity_t * entity, int frame_index, anchor_t anchor)
{
    sprite_t * sprite = entity->sprite;
    frame_t const * frame = get_frame(sprite->texture, frame_index);
    if (NULL != frame)
    {
        uint16_t old_width = entity->width, old_height = entity->height;
        uint16_t new_width = frame->width, new_height = frame->height;

        if (old_width != new_width || old_height != new_height)
        {
            apply_anchor(
                anchor,
                old_width - new_width,
                old_height - new_height,
                &entity->x,
                &entity->y
            );

            entity->width = new_width;
            entity->height = new_height;
        }

        sprite->frame = frame;
    }
}