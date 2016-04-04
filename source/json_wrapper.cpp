#include <stdlib.h>
#include <stdio.h>

#include "json_wrapper.h"
#include "json.h"
#include "utils.h"

extern "C" {

static char * get_json_data(const char * name)
{
    if (0 == strncmp(name, "rtype_sprites", 13))
    {
        return read_file("data/rtype_sprites.json");
    }
    else if (0 == strncmp(name, "rtype_frames", 12))
    {
        return read_file("data/rtype_frames.json");
    }
    else if (0 == strncmp(name, "level_one_sprites", 17))
    {
        return read_file("data/level_one_sprites.json");
    }
    else if (0 == strncmp(name, "level_one_frames", 16))
    {
        return read_file("data/level_one_frames.json");
    }
    return NULL;
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
    auto json = static_cast<Json *>(o);

    int nb_frames = Json::Size(json);
    printf("-- %d frames.\n", nb_frames);

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
    for (auto const &node : json->value)
    {
        x = Json::GetNumber(node, "x");
        y = Json::GetNumber(node, "y");
        width = Json::GetNumber(node, "width");
        height = Json::GetNumber(node, "height");

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

int load_sprites(JsonWrapper * o, const char * texture_name, List * sprites)
{
    auto json = static_cast<Json *>(o);

    Sprite * sprite = NULL;
    const char * name = NULL;
    const char * method = NULL;

    int width = 0, height = 0, start_frame = 0, nb_frames = 0;

    for (auto const &node : json->value)
    {
        name = Json::GetString(node, "id");
        method = Json::GetString(node, "update");
        width = Json::GetNumber(node, "width");
        height = Json::GetNumber(node, "height");
        start_frame = Json::GetNumber(node, "start_frame");
        nb_frames = Json::GetNumber(node, "nb_frames");

        if (NULL == name)
        {
            printf("Missing sprite id\n");
            return 2;
        }
        else if (width < 0 || height < 0)
        {
            printf("Missing dimension for %s\n", name);
            return 4;
        }
        else if (start_frame < 0 || nb_frames < 0)
        {
            printf("Missing frames for %s\n", name);
            return 5;
        }

        sprite = (Sprite *)list_alloc(sprites);

        sprite->x = 0;
        sprite->y = 0;
        sprite->width = width;
        sprite->height = height;
        sprite->name = strdup(name);
        sprite->method = NULL != method ? strdup(method) : NULL;
        sprite->texture = strdup(texture_name);
        sprite->start_frame = start_frame;
        sprite->nb_frames = nb_frames;
        sprite->current_frame = 0;
    }
    return 0;
}

}