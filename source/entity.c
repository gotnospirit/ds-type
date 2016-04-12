#include "entity.h"
#include "render.h"
#include "texture.h"
#include "json_wrapper.h"
#include "list.h"

static list_t * container = NULL;

int init_entities()
{
    container = list_new(sizeof(entity_t), 1);
    if (NULL == container)
    {
        return 1;
    }

    texture_t * texture = texture_new("base");
    if (NULL == texture)
    {
        return 2;
    }

    json_wrapper_t * json = json_new("base");
    if (NULL == json)
    {
        return 3;
    }
    else if (0 != parse_base(json, container, texture))
    {
        json_delete(json);
        return 4;
    }
    json_delete(json);
    return 0;
}

void shutdown_entities()
{
    entity_t * entity = NULL;
    while (list_next(container, (void **)&entity))
    {
        remove_from_rendering(entity->sprite);
        free(entity->name);
        free(entity->sprite);
    }
    list_delete(&container);
}

entity_t * entity_get(const char * name)
{
    entity_t * entity = NULL;
    while (list_next(container, (void **)&entity))
    {
        if (0 == strcmp(entity->name, name))
        {
            return entity;
        }
    }
    return NULL;
}