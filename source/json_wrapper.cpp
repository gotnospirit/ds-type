#include <stdlib.h>
#include <stdio.h>

#include "json_wrapper.h"
#include "json.h"

extern "C" {

#include "structs.h"
#include "update.h"
#include "texture.h"
#include "utils.h"

static Sprite * sprite_new(int x, int y, uint16_t width, uint16_t height, Texture const * texture, Frame const * frame)
{
    // @TODO(james) handle error
    Sprite * result = (Sprite *)malloc(sizeof(Sprite));
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

static char * get_json_data(const char * name)
{
    if (0 == strncmp(name, "base", 4))
    {
        return read_file("data/base.json");
    }
    else if (0 == strncmp(name, "level_one", 9))
    {
        return read_file("data/level_one.json");
    }
    return NULL;
}

static int process_frames(JsonValue const &root, Texture * spritesheet)
{
    int nb_frames = Json::Size(root);

    Frame * frames = (Frame *)malloc(sizeof(Frame) * nb_frames);
    if (NULL == frames)
    {
        printf("Failed to init frames\n");
        return 1;
    }
    spritesheet->frames = frames;

    Frame * frame = NULL;

    float const texture_width = (float)spritesheet->width;
    float const texture_height = (float)spritesheet->height;

    float const texture_real_width = (float)spritesheet->real_width;
    float const texture_real_height = (float)spritesheet->real_height;

    float const texture_width_ratio = texture_width / texture_real_width;
    float const texture_height_ratio = texture_height / texture_real_height;

    float frame_width_ratio = 0;
    float frame_height_ratio = 0;
    float left = 0, top = 0;
    int x = 0, y = 0, width = 0, height = 0;

    nb_frames = 0;
    for (auto const &node : root)
    {
        auto const &value = node->value;

        x = Json::GetNumber(value, "x");
        y = Json::GetNumber(value, "y");
        width = Json::GetNumber(value, "width");
        height = Json::GetNumber(value, "height");

        frame_width_ratio = width * texture_width_ratio / texture_width;
        frame_height_ratio = height * texture_height_ratio / texture_height;

        left = (float)x / texture_real_width;
        top = (float)y / texture_real_height;

        frame = &frames[nb_frames];

        frame->left = left;
        frame->top = top;
        frame->right = left + frame_width_ratio;
        frame->bottom = top + frame_height_ratio;

        ++nb_frames;
    }
    return 0;
}

static JsonNode const * get_frame_node(JsonValue const &root, const char * id)
{
    for (auto const &node : root)
    {
        const char * tile_id = Json::GetString(node->value, "id");

        if (NULL != tile_id && 0 == strcmp(tile_id, id))
        {
            return node;
        }
    }
    return NULL;
}

static int get_frame_index(JsonValue const &root, JsonNode const * target)
{
    int index = 0;
    for (auto const &node : root)
    {
        if (target == node)
        {
            return index;
        }
        ++index;
    }
    return -1;
}

static int create_tile(const char * name, int x, bool flip_x, bool flip_y, List * container, JsonValue const &frames_node, Texture const * texture)
{
    if (NULL == name)
    {
        return 1;
    }

    auto const &frame_node = get_frame_node(frames_node, name);
    if (NULL == frame_node)
    {
        return 0;
    }

    int width = Json::GetNumber(frame_node->value, "width");
    if (-1 == width)
    {
        return 0;
    }

    int height = Json::GetNumber(frame_node->value, "height");
    if (-1 == height)
    {
        return 0;
    }

    Frame const * frame = get_frame(texture, get_frame_index(frames_node, frame_node));
    if (NULL == frame)
    {
        return 0;
    }

    Sprite * sprite = sprite_new(0, 0, width, height, texture, frame);
    sprite->flip_x = flip_x ? 1 : 0;
    sprite->flip_y = flip_y ? 1 : 0;

    // @TODO(james) handle error
    Tile * tile = (Tile *)list_alloc(container);
    tile->x = x;
    tile->width = width;
    tile->height = height;
    tile->visible = 0;
    tile->sprite = sprite;
    return 1;
}

static int process_level_tiles(JsonValue const &root, Level * level, JsonValue const &frames_node, Texture const * texture)
{
    int x = -1;
    const char * top_name = NULL;
    const char * bottom_name = NULL;
    bool flip_y = false;

    for (auto const &node : root)
    {
        auto const &value = node->value;

        x = Json::GetNumber(value, "x");
        top_name = Json::GetString(value, "top");
        bottom_name = Json::GetString(value, "bottom");
        flip_y = Json::GetBoolean(value, "flip_y");

        if (-1 == x)
        {
            printf("Missing tile's x axis\n");
            return 1;
        }
        else if (NULL == top_name && NULL == bottom_name)
        {
            printf("Missing tile's position\n");
            return 2;
        }

        if (!create_tile(top_name, x, true, flip_y, level->top_tiles, frames_node, texture))
        {
            printf("'%s' failed\n", top_name);
            return 3;
        }

        if (!create_tile(bottom_name, x, false, flip_y, level->bottom_tiles, frames_node, texture))
        {
            printf("'%s' failed\n", bottom_name);
            return 4;
        }
    }
    return 0;
}

static int process_base_entities(JsonValue const &root, List * entities, Texture const * texture)
{
    const char * name = NULL;
    int width = 0, height = 0, start_frame = 0, current_frame = 0, nb_frames = 0;

    Entity * entity = NULL;
    Frame const * frame = NULL;
    for (auto const &node : root)
    {
        auto const &value = node->value;

        name = Json::GetString(value, "id");
        width = Json::GetNumber(value, "width");
        height = Json::GetNumber(value, "height");
        start_frame = Json::GetNumber(value, "start_frame");
        current_frame = Json::GetNumber(value, "current_frame");
        nb_frames = Json::GetNumber(value, "nb_frames");

        if (NULL == name)
        {
            printf("Missing entity id\n");
            return 1;
        }
        else if (width < 0 || height < 0)
        {
            printf("Missing dimension for %s\n", name);
            return 3;
        }
        else if (start_frame < 0 || nb_frames < 0)
        {
            printf("Missing frames for %s\n", name);
            return 4;
        }

        if (-1 == current_frame)
        {
            current_frame = 0;
        }
        else if (current_frame > nb_frames)
        {
            current_frame = nb_frames;
        }

        frame = get_frame(texture, start_frame + current_frame);
        if (NULL == frame)
        {
            printf("Frame %d not found\n", current_frame);
            return 5;
        }

        // @TODO(james) handle error
        entity = (Entity *)list_alloc(entities);
        entity->name = strdup(name);
        entity->x = 0;
        entity->y = 0;
        entity->width = width;
        entity->height = height;
        entity->start_frame = start_frame;
        entity->nb_frames = nb_frames;
        entity->current_frame = current_frame;
        entity->elapsed = 0;
        entity->sprite = sprite_new(0, 0, width, height, texture, frame);
    }
    return 0;
}

JsonWrapper * json_new(const char * name)
{
    char * data = get_json_data(name);
    if (NULL == data)
    {
        return NULL;
    }

    Json * json = new Json();
    if (!json->parse(data))
    {
        json_delete(json);
        free(data);
        printf("Failed to load '%s'\n", name);
        return NULL;
    }
    free(data);
    return (JsonWrapper *)json;
}

void json_delete(JsonWrapper * o)
{
    delete static_cast<Json *>(o);
}

int parse_base(JsonWrapper * o, List * entities, Texture * spritesheet)
{
    auto json = static_cast<Json *>(o);

    JsonNode const * frames_node = NULL;
    JsonNode const * entities_node = NULL;

    const char * key = 0;

    for (auto const &node : json->value)
    {
        key = node->key;

        if (0 == strncmp(key, "frames", 6))
        {
            frames_node = node;
        }
        else if (0 == strncmp(key, "entities", 8))
        {
            entities_node = node;
        }
    }

    if (NULL == frames_node)
    {
        printf("No frames defined\n");
        return 1;
    }
    else if (NULL == entities_node)
    {
        printf("No entities defined\n");
        return 2;
    }

    if (0 != process_frames(frames_node->value, spritesheet))
    {
        return 3;
    }
    else if (0 != process_base_entities(entities_node->value, entities, spritesheet))
    {
        return 4;
    }
    return 0;
}

int parse_level(JsonWrapper * o, Level * level, Texture * spritesheet)
{
    auto json = static_cast<Json *>(o);

    JsonNode const * frames_node = NULL;
    JsonNode const * tiles_node = NULL;

    const char * key = 0;

    for (auto const &node : json->value)
    {
        key = node->key;

        if (0 == strncmp(key, "frames", 6))
        {
            frames_node = node;
        }
        else if (0 == strncmp(key, "tiles", 5))
        {
            tiles_node = node;
        }
        else if (0 == strncmp(key, "stop_scroll_at", 14))
        {
            level->max_camera_left = node->value.toNumber();
        }
    }

    if (NULL == frames_node)
    {
        printf("No frames defined\n");
        return 1;
    }
    else if (NULL == tiles_node)
    {
        printf("No tiles defined\n");
        return 2;
    }

    if (0 != process_frames(frames_node->value, spritesheet))
    {
        return 3;
    }
    else if (0 != process_level_tiles(tiles_node->value, level, frames_node->value, spritesheet))
    {
        return 4;
    }
    return 0;
}

}