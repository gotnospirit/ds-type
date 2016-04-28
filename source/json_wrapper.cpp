#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "json_wrapper.h"
#include "json.h"

extern "C" {

#include "structs.h"
#include "texture.h"
#include "entity.h"
#include "animation.h"
#include "utils.h"

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

static int process_frames(JsonValue const &root, texture_t * spritesheet)
{
    int nb_frames = Json::Size(root);

    frame_t * frames = (frame_t *)malloc(sizeof(frame_t) * nb_frames);
    if (NULL == frames)
    {
        printf("Failed to init frames\n");
        return 1;
    }
    spritesheet->frames = frames;

    frame_t * frame = NULL;

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

        if (width < 0 || height < 0)
        {
            printf("Invalid frame's width/height\n");
            free(frames);
            return 2;
        }

        frame_width_ratio = width * texture_width_ratio / texture_width;
        frame_height_ratio = height * texture_height_ratio / texture_height;

        left = (float)x / texture_real_width;
        top = (float)y / texture_real_height;

        frame = &frames[nb_frames];

        frame->left = left;
        frame->top = top;
        frame->right = left + frame_width_ratio;
        frame->bottom = top + frame_height_ratio;
        frame->width = width;
        frame->height = height;

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

static int create_tile(const char * name, int x, bool is_top_tile, bool flip_y, list_t * container, JsonValue const &frames_node, texture_t const * texture)
{
    auto const &frame_node = get_frame_node(frames_node, name);
    if (NULL == frame_node)
    {
        return 1;
    }

    int width = Json::GetNumber(frame_node->value, "width");
    if (-1 == width)
    {
        return 2;
    }

    int height = Json::GetNumber(frame_node->value, "height");
    if (-1 == height)
    {
        return 3;
    }

    frame_t const * frame = get_frame(texture, get_frame_index(frames_node, frame_node));
    if (NULL == frame)
    {
        return 4;
    }

    sprite_t * sprite = (sprite_t *)malloc(sizeof(sprite_t));
    if (NULL == sprite)
    {
        return 5;
    }

    sprite->x = x;
    sprite->y = 0;
    sprite->texture = texture;
    sprite->frame = frame;
    sprite->flip_x = is_top_tile ? 1 : 0;
    sprite->flip_y = flip_y ? 1 : 0;

    tile_t * tile = (tile_t *)list_alloc(container);
    if (NULL == tile)
    {
        free(sprite);
        return 6;
    }

    tile->x = x;
    tile->width = width;
    tile->height = height;
    tile->visible = 0;
    tile->anchor = is_top_tile ? TOP : BOTTOM;
    tile->sprite = sprite;
    return 0;
}

static int process_level_tiles(JsonValue const &root, list_t * container, JsonValue const &frames_node, texture_t const * texture)
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

        if (NULL != top_name && 0 != create_tile(top_name, x, true, flip_y, container, frames_node, texture))
        {
            printf("'%s' failed\n", top_name);
            return 3;
        }

        if (NULL != bottom_name && 0 != create_tile(bottom_name, x, false, flip_y, container, frames_node, texture))
        {
            printf("'%s' failed\n", bottom_name);
            return 4;
        }
    }
    return 0;
}

static int process_base_animations(JsonValue const &root)
{
    const char * name = NULL;
    const char * type = NULL;
    int start = 0, end = 0, loop = 0;
    uint16_t duration = 0;

    for (auto const &node : root)
    {
        auto const &value = node->value;

        name = Json::GetString(value, "id");
        type = Json::GetString(value, "type");
        start = Json::GetNumber(value, "start");
        end = Json::GetNumber(value, "end");
        loop = Json::GetNumber(value, "loop");
        duration = Json::GetNumber(value, "duration");

        if (NULL == name)
        {
            printf("Missing animation's id\n");
            return 1;
        }
        else if (NULL == type)
        {
            printf("Missing animation's type\n");
            return 2;
        }
        else if (start < 0 || end < 0 || duration < 0)
        {
            printf("Invalid animation parameters\n");
            return 3;
        }

        if (NULL == animation_template_new(name, start, end, duration, loop, type))
        {
            printf("Template not created\n");
            return 4;
        }
    }
    return 0;
}

static int process_base_shots(JsonValue const &root)
{
    const char * name = NULL;
    int threshold = 0;

    for (auto const &node : root)
    {
        auto const &value = node->value;

        name = Json::GetString(value, "id");
        threshold = Json::GetNumber(value, "threshold");

        if (NULL == name)
        {
            printf("Missing shot's id\n");
            return 1;
        }
        else if (threshold < 0 || threshold > 100)
        {
            printf("Invalid shot's threshold\n");
            return 2;
        }

        if (NULL == entity_shot_new(name, threshold))
        {
            printf("Shot not created\n");
            return 3;
        }
    }
    return 0;
}

static int process_base_entities(JsonValue const &root, texture_t const * texture)
{
    const char * name = NULL;
    const char * logic_method = NULL;
    int frame = 0, anchor = 0, velocity = 0;

    for (auto const &node : root)
    {
        auto const &value = node->value;

        name = Json::GetString(value, "id");
        frame = Json::GetNumber(value, "frame");
        anchor = Json::GetNumber(value, "anchor");
        logic_method = Json::GetString(value, "logic");
        velocity = Json::GetNumber(value, "velocity");

        if (NULL == name)
        {
            printf("Missing entity's id\n");
            return 1;
        }
        else if (frame < 0)
        {
            printf("Missing idle frame for %s\n", name);
            return 2;
        }
        else if (anchor >= 9)
        {
            printf("Invalid anchor value for %s (max 8)\n", name);
            return 3;
        }
        else if (velocity > 255)
        {
            printf("Invalid velocity value for %s (max 255)\n", name);
            return 4;
        }

        if (anchor < 0)
        {
            anchor = 0;
        }

        if (velocity < 0)
        {
            velocity = 0;
        }

        if (NULL == get_frame(texture, frame))
        {
            printf("Frame %d not found\n", frame);
            return 5;
        }
        else if (NULL == entity_template_new(name, frame, texture, logic_method, (anchor_t)anchor, velocity))
        {
            printf("Template not created\n");
            return 6;
        }
    }
    return 0;
}

json_wrapper_t * json_new(const char * name)
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
    return (json_wrapper_t *)json;
}

void json_delete(json_wrapper_t * o)
{
    delete static_cast<Json *>(o);
}

int parse_base(json_wrapper_t * o, texture_t * spritesheet)
{
    auto json = static_cast<Json *>(o);

    JsonNode const * frames_node = NULL;
    JsonNode const * entities_node = NULL;
    JsonNode const * animations_node = NULL;
    JsonNode const * shots_node = NULL;

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
        else if (0 == strncmp(key, "animations", 10))
        {
            animations_node = node;
        }
        else if (0 == strncmp(key, "shots", 5))
        {
            shots_node = node;
        }
    }

    if (NULL == frames_node)
    {
        printf("No frame defined\n");
        return 1;
    }
    else if (NULL == entities_node)
    {
        printf("No entity defined\n");
        return 2;
    }
    else if (NULL == animations_node)
    {
        printf("No animation defined\n");
        return 3;
    }
    else if (NULL == shots_node)
    {
        printf("No shot defined\n");
        return 4;
    }

    if (0 != process_frames(frames_node->value, spritesheet))
    {
        return 5;
    }
    else if (0 != process_base_entities(entities_node->value, spritesheet))
    {
        return 6;
    }
    else if (0 != process_base_animations(animations_node->value))
    {
        return 7;
    }
    else if (0 != process_base_shots(shots_node->value))
    {
        return 8;
    }
    return 0;
}

int parse_level(json_wrapper_t * o, level_t * level, texture_t * spritesheet)
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
    else if (0 != process_level_tiles(tiles_node->value, level->tiles, frames_node->value, spritesheet))
    {
        return 4;
    }
    return 0;
}

}