#include <stdlib.h>
#include <stdio.h>

#include "json_wrapper.h"
#include "json.h"

extern "C" {

#include "structs.h"
#include "update.h"
#include "texture.h"
#include "utils.h"

static char * get_json_data(const char * name)
{
    if (0 == strncmp(name, "sprites", 7))
    {
        return read_file("data/sprites.json");
    }
    else if (0 == strncmp(name, "rtype_frames", 12))
    {
        return read_file("data/rtype_frames.json");
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

    float const texture_width_ratio = texture_width / spritesheet->real_width;
    float const texture_height_ratio = texture_height / spritesheet->real_height;

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

        left = (float)x / spritesheet->real_width;
        top = (float)y / spritesheet->real_height;

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

static int create_tile(const char * name, int x, List * container, JsonValue const &frames_node, Texture const * texture)
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

    // @TODO(james) handle error
    Sprite * sprite = (Sprite *)malloc(sizeof(Sprite));
    sprite->width = width;
    sprite->height = height;
    sprite->texture = texture;
    sprite->frame = frame;

    Tile * tile = (Tile *)list_alloc(container);
    tile->x = x;
    tile->visible = 0;
    tile->sprite = sprite;
    return 1;
}

static int process_level_tiles(JsonValue const &root, Level * level, JsonValue const &frames_node, Texture const * texture)
{
    int x = -1;
    const char * top_name = NULL;
    const char * bottom_name = NULL;

    for (auto const &node : root)
    {
        auto const &value = node->value;

        x = Json::GetNumber(value, "x");
        top_name = Json::GetString(value, "top");
        bottom_name = Json::GetString(value, "bottom");

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

        if (!create_tile(top_name, x, level->top_tiles, frames_node, texture))
        {
            printf("'%s' failed\n", top_name);
            return 3;
        }

        if (!create_tile(bottom_name, x, level->bottom_tiles, frames_node, texture))
        {
            printf("'%s' failed\n", top_name);
            return 4;
        }
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

int load_frames(JsonWrapper * o, Texture * spritesheet)
{
    return process_frames(static_cast<Json *>(o)->value, spritesheet);
}

int load_templates(JsonWrapper * o, List * templates, const char * texture)
{
    auto json = static_cast<Json *>(o);

    Template * tpl = NULL;
    const char * name = NULL;
    const char * method = NULL;

    int width = 0, height = 0, start_frame = 0, current_frame = 0, nb_frames = 0;

    for (auto const &node : json->value)
    {
        auto const &value = node->value;

        name = Json::GetString(value, "id");
        width = Json::GetNumber(value, "width");
        height = Json::GetNumber(value, "height");
        method = Json::GetString(value, "update");
        start_frame = Json::GetNumber(value, "start_frame");
        current_frame = Json::GetNumber(value, "current_frame");
        nb_frames = Json::GetNumber(value, "nb_frames");

        if (NULL == name)
        {
            printf("Missing sprite id\n");
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

        tpl = (Template *)list_alloc(templates);

        tpl->width = width;
        tpl->height = height;
        tpl->name = strdup(name);
        tpl->texture = strdup(texture);
        tpl->update = NULL;
        tpl->start_frame = start_frame;
        tpl->nb_frames = nb_frames;
        tpl->initial_frame = current_frame >= 0 ? current_frame : 0;

        if (NULL != method && 0 == strncmp(method, "UpdateHero", 10))
        {
            tpl->update = update_hero;
        }
    }
    return 0;
}

int load_level(JsonWrapper * o, Level * level, Texture * spritesheet)
{
    auto json = static_cast<Json *>(o);

    JsonNode const * frames = NULL;
    JsonNode const * tiles = NULL;

    const char * key = 0;

    for (auto const &node : json->value)
    {
        key = node->key;

        if (0 == strncmp(key, "frames", 6))
        {
            frames = node;
        }
        else if (0 == strncmp(key, "tiles", 5))
        {
            tiles = node;
        }
        else if (0 == strncmp(key, "stop_scroll_at", 14))
        {
            level->max_camera = node->value.toNumber();
        }
    }

    if (NULL == frames)
    {
        printf("No frames defined\n");
        return 1;
    }
    else if (NULL == tiles)
    {
        printf("No tiles defined\n");
        return 2;
    }

    if (0 != process_frames(frames->value, spritesheet))
    {
        return 3;
    }
    else if (0 != process_level_tiles(tiles->value, level, frames->value, spritesheet))
    {
        return 4;
    }
    return 0;
}

}