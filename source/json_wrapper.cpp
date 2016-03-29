#include <stdlib.h>
#include <stdio.h>

#include "json_wrapper.h"
#include "json.h"

#ifndef CITRA
#include "utils.h"
#endif

extern "C" {

static char * get_json_data(const char * name)
{
    if (0 == strncmp(name, "rtype", 11))
    {
        return
#ifdef CITRA
        // strdup("[{\"x\":0,\"y\":0,\"width\":45,\"height\":45},{\"x\":45,\"y\":0,\"width\":45,\"height\":45},{\"x\":90,\"y\":0,\"width\":45,\"height\":45},{\"x\":135,\"y\":0,\"width\":45,\"height\":45},{\"x\":180,\"y\":0,\"width\":45,\"height\":45},{\"x\":225,\"y\":0,\"width\":45,\"height\":45},{\"x\":0,\"y\":45,\"width\":45,\"height\":45},{\"x\":45,\"y\":45,\"width\":45,\"height\":45},{\"x\":90,\"y\":45,\"width\":45,\"height\":45},{\"x\":135,\"y\":45,\"width\":45,\"height\":45},{\"x\":180,\"y\":45,\"width\":45,\"height\":45},{\"x\":225,\"y\":45,\"width\":45,\"height\":45},{\"x\":0,\"y\":90,\"width\":45,\"height\":45},{\"x\":45,\"y\":90,\"width\":45,\"height\":45},{\"x\":90,\"y\":90,\"width\":45,\"height\":45},{\"x\":135,\"y\":90,\"width\":45,\"height\":45},{\"x\":180,\"y\":90,\"width\":45,\"height\":45},{\"x\":225,\"y\":90,\"width\":45,\"height\":45}]")
        strdup("[{\"x\":96,\"y\":0,\"width\":32,\"height\":16},{\"x\":128,\"y\":0,\"width\":32,\"height\":16},{\"x\":160,\"y\":0,\"width\":32,\"height\":16},{\"x\":192,\"y\":0,\"width\":32,\"height\":16},{\"x\":224,\"y\":0,\"width\":32,\"height\":16},{\"x\":80,\"y\":0,\"width\":16,\"height\":16},{\"x\":64,\"y\":16,\"width\":16,\"height\":16},{\"x\":80,\"y\":16,\"width\":16,\"height\":16},{\"x\":96,\"y\":16,\"width\":16,\"height\":16},{\"x\":112,\"y\":16,\"width\":16,\"height\":16},{\"x\":128,\"y\":16,\"width\":16,\"height\":16},{\"x\":144,\"y\":16,\"width\":16,\"height\":16},{\"x\":160,\"y\":16,\"width\":16,\"height\":16},{\"x\":176,\"y\":16,\"width\":16,\"height\":16},{\"x\":192,\"y\":16,\"width\":16,\"height\":16},{\"x\":208,\"y\":16,\"width\":16,\"height\":16},{\"x\":224,\"y\":16,\"width\":16,\"height\":16},{\"x\":240,\"y\":16,\"width\":16,\"height\":16},{\"x\":64,\"y\":32,\"width\":32,\"height\":32},{\"x\":96,\"y\":32,\"width\":32,\"height\":32},{\"x\":128,\"y\":32,\"width\":32,\"height\":32},{\"x\":160,\"y\":32,\"width\":32,\"height\":32},{\"x\":192,\"y\":32,\"width\":32,\"height\":32},{\"x\":224,\"y\":32,\"width\":32,\"height\":32},{\"x\":0,\"y\":168,\"width\":20,\"height\":20},{\"x\":20,\"y\":168,\"width\":20,\"height\":20},{\"x\":40,\"y\":168,\"width\":20,\"height\":20},{\"x\":0,\"y\":188,\"width\":20,\"height\":20},{\"x\":20,\"y\":188,\"width\":20,\"height\":20},{\"x\":40,\"y\":188,\"width\":20,\"height\":20},{\"x\":66,\"y\":64,\"width\":26,\"height\":22},{\"x\":92,\"y\":64,\"width\":26,\"height\":22},{\"x\":118,\"y\":64,\"width\":26,\"height\":22},{\"x\":66,\"y\":86,\"width\":26,\"height\":22},{\"x\":92,\"y\":86,\"width\":26,\"height\":22},{\"x\":118,\"y\":86,\"width\":26,\"height\":22},{\"x\":0,\"y\":0,\"width\":64,\"height\":40},{\"x\":0,\"y\":40,\"width\":64,\"height\":32},{\"x\":0,\"y\":72,\"width\":64,\"height\":48},{\"x\":0,\"y\":120,\"width\":64,\"height\":16},{\"x\":0,\"y\":136,\"width\":64,\"height\":16},{\"x\":0,\"y\":152,\"width\":64,\"height\":16},{\"x\":144,\"y\":64,\"width\":112,\"height\":96},{\"x\":64,\"y\":160,\"width\":192,\"height\":96}]")
#else
        read_file("data/rtype.json")
#endif
        ;
    }
    else if (0 == strncmp(name, "sprites", 7))
    {
        return
#ifdef CITRA
        // strdup("[{\"id\":\"ship\",\"width\":32,\"height\":16,\"update\":\"UpdateHero\",\"texture\":\"rtype\",\"start_frame\":0,\"nb_frames\":5}]")
        // strdup("[{\"id\":\"ship\",\"width\":32,\"height\":16,\"update\":\"UpdateHero\",\"texture\":\"rtype\",\"start_frame\":0,\"nb_frames\":5},{\"id\":\"ship\",\"width\":32,\"height\":16,\"update\":\"UpdateHero\",\"texture\":\"rtype\",\"start_frame\":0,\"nb_frames\":5}]")
        strdup("[{\"id\":\"ship\",\"width\":32,\"height\":16,\"update\":\"UpdateHero\",\"texture\":\"rtype\",\"start_frame\":0,\"nb_frames\":5},{\"id\":\"shot\",\"width\":16,\"height\":16,\"texture\":\"rtype\",\"start_frame\":5,\"nb_frames\":1},{\"id\":\"orb\",\"width\":16,\"height\":16,\"texture\":\"rtype\",\"start_frame\":6,\"nb_frames\":12},{\"id\":\"ship_explode\",\"width\":32,\"height\":32,\"texture\":\"rtype\",\"start_frame\":18,\"nb_frames\":6},{\"id\":\"module_1\",\"width\":20,\"height\":20,\"texture\":\"rtype\",\"start_frame\":24,\"nb_frames\":6},{\"id\":\"module_2\",\"width\":26,\"height\":22,\"texture\":\"rtype\",\"start_frame\":30,\"nb_frames\":6},{\"id\":\"surface_1\",\"width\":64,\"height\":40,\"texture\":\"rtype\",\"start_frame\":36,\"nb_frames\":1},{\"id\":\"surface_2\",\"width\":64,\"height\":32,\"texture\":\"rtype\",\"start_frame\":37,\"nb_frames\":1},{\"id\":\"surface_3\",\"width\":64,\"height\":48,\"texture\":\"rtype\",\"start_frame\":38,\"nb_frames\":1},{\"id\":\"surface_4\",\"width\":64,\"height\":16,\"texture\":\"rtype\",\"start_frame\":39,\"nb_frames\":1},{\"id\":\"surface_5\",\"width\":64,\"height\":16,\"texture\":\"rtype\",\"start_frame\":40,\"nb_frames\":1},{\"id\":\"surface_6\",\"width\":64,\"height\":16,\"texture\":\"rtype\",\"start_frame\":41,\"nb_frames\":1},{\"id\":\"surface_7\",\"width\":112,\"height\":96,\"texture\":\"rtype\",\"start_frame\":42,\"nb_frames\":1},{\"id\":\"surface_8\",\"width\":192,\"height\":96,\"texture\":\"rtype\",\"start_frame\":43,\"nb_frames\":1}]")
#else
        read_file("data/sprites.json")
#endif
        ;
    }
    return NULL;
}

JsonWrapper * json_create(const char * name)
{
    Json * json = new Json();
    char * data = get_json_data(name);
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

int load_sprites(JsonWrapper * o, Sprite ** container, int * size)
{
    auto json = static_cast<Json *>(o);

    int nb_sprites = Json::Size(json);
    printf("-- %d sprites.\n", nb_sprites);

    Sprite * sprites = (Sprite *)malloc(sizeof(Sprite) * nb_sprites);
    if (NULL == sprites)
    {
        printf("Failed to init sprites\n");
        return 1;
    }
    *container = sprites;
    *size = nb_sprites;

    Sprite * sprite = NULL;
    const char * name = NULL;
    const char * method = NULL;
    const char * texture = NULL;

    int width = 0, height = 0, start_frame = 0, nb_frames = 0;

    nb_sprites = 0;
    for (auto const &node : json->value)
    {
        name = Json::GetString(node, "id");
        method = Json::GetString(node, "update");
        texture = Json::GetString(node, "texture");
        width = Json::GetNumber(node, "width");
        height = Json::GetNumber(node, "height");
        start_frame = Json::GetNumber(node, "start_frame");
        nb_frames = Json::GetNumber(node, "nb_frames");

        if (NULL == name)
        {
            printf("Missing sprite id\n");
            return 2;
        }
        else if (NULL == texture)
        {
            printf("Missing texture name for %s\n", name);
            return 3;
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

        sprite = &sprites[nb_sprites];

        sprite->x = 0;
        sprite->y = 0;
        sprite->width = width;
        sprite->height = height;
        sprite->name = strdup(name);
        sprite->method = NULL != method ? strdup(method) : NULL;
        sprite->texture = strdup(texture);
        sprite->start_frame = start_frame;
        sprite->nb_frames = nb_frames;
        sprite->current_frame = 0;

        ++nb_sprites;
    }
    return 0;
}

}