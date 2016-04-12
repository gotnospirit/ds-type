#include "entity.h"
#include "render.h"
#include "texture.h"
#include "json_wrapper.h"
#include "list.h"

static list_t * templates = NULL;
static list_t * entities = NULL;

static entity_t * entity_get(const char * type)
{
    entity_t * entity = NULL;
    while (list_next(entities, (void **)&entity))
    {
        if (0 == strcmp(entity->type, type))
        {
            return entity;
        }
    }
    return NULL;
}

int init_entities()
{
    templates = list_new(sizeof(template_t), 1);
    if (NULL == templates)
    {
        return 1;
    }

    entities = list_new(sizeof(entity_t), 1);
    if (NULL == entities)
    {
        return 2;
    }

    texture_t * texture = texture_new("base");
    if (NULL == texture)
    {
        return 3;
    }

    json_wrapper_t * json = json_new("base");
    if (NULL == json)
    {
        return 4;
    }
    else if (0 != parse_base(json, texture))
    {
        json_delete(json);
        return 5;
    }
    json_delete(json);
    return 0;
}

void shutdown_entities()
{
    sprite_t * sprite = NULL;
    entity_t * entity = NULL;
    while (list_next(entities, (void **)&entity))
    {
        sprite = entity->sprite;
        remove_from_rendering(sprite);
        free(sprite);
    }
    list_delete(&entities);

    template_t * template = NULL;
    while (list_next(templates, (void **)&template))
    {
        free(template->name);
    }
    list_delete(&templates);
}

sprite_t * sprite_new(int x, int y, uint16_t width, uint16_t height, texture_t const * texture, frame_t const * frame)
{
    // @TODO(james) handle error
    sprite_t * result = (sprite_t *)malloc(sizeof(sprite_t));
    result->x = x;
    result->y = y;
    result->width = width;
    result->height = height;
    result->texture = texture;
    result->frame = frame;
    result->flip_x = 0;
    result->flip_y = 0;
    return result;
}

template_t * template_new(const char * name, uint16_t width, uint16_t height, uint8_t start_frame, uint8_t nb_frames, uint8_t current_frame, texture_t const * texture)
{
    template_t * result = (template_t *)list_alloc(templates);
    result->name = strdup(name);
    result->width = width;
    result->height = height;
    result->start_frame = start_frame;
    result->nb_frames = nb_frames;
    result->current_frame = current_frame;
    result->texture = texture;
    return result;
}

entity_t * entity_new(const char * type)
{
    template_t * template = NULL;
    while (list_next(templates, (void **)&template))
    {
        if (0 == strcmp(template->name, type))
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

            sprite_t * sprite = sprite_new(0, 0, width, height, texture, frame);
            if (NULL == sprite)
            {
                return NULL;
            }

            entity_t * result = (entity_t *)list_alloc(entities);
            if (NULL == result)
            {
                free(sprite);
                return NULL;
            }

            result->type = template->name;
            result->x = 0;
            result->y = 0;
            result->width = width;
            result->height = height;
            result->start_frame = start_frame;
            result->nb_frames = template->nb_frames;
            result->current_frame = current_frame;
            result->sprite = sprite;
            return result;
        }
    }
    return NULL;
}

int entity_delete(entity_t const ** entity)
{
    entity_t * ptr = NULL;
    while (list_next(entities, (void **)&ptr))
    {
        if (*entity == ptr)
        {
            sprite_t * sprite = ptr->sprite;
            remove_from_rendering(sprite);
            free(sprite);
            list_dealloc(entities, ptr);
            *entity = NULL;
            return 1;
        }
    }
    return 0;
}

void update_sprites(rectangle_t const * camera)
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

entity_t * entity_get_ship()
{
    entity_t * result = entity_get("ship");
    if (NULL == result)
    {
        result = entity_new("ship");
    }
    return result;
}

entity_t * entity_spawn_shot(int x, int y)
{
    entity_t * result = entity_new("shot");
    if (NULL != result)
    {
        result->x = x;
        result->y = y;

        add_to_rendering(result->sprite);
    }
    return result;
}
