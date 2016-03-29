#include <stdlib.h>

#include <3ds.h>

#include "game_states.h"
#include "json_wrapper.h"
#include "structs.h"
#include "utils.h"
#include "render.h"

static Surface screen;

static u64 start_time;
static circlePosition circle_pad;

static Sprite * sprites;
static int nb_sprites;

static Sprite const * ship;

static Sprite * get_sprite(const char * name)
{
    for (int i = 0; i < nb_sprites; ++i)
    {
        if (0 == strcmp(sprites[i].name, name))
        {
            return &sprites[i];
        }
    }
    return NULL;
}

static void update_hero(Sprite * chip, Surface const * screen, uint64_t elapsed)
{
    int incr = 5;

    int dx = circle_pad.dx;
    int dy = circle_pad.dy;

    float x_incr = dx * incr / 160;
    float y_incr = dy * incr / 160;

    y_incr *= -1;

    chip->x += x_incr;
    chip->y += y_incr;

    // update chip position
    chip->x = clamp(chip->x, 0, screen->width - chip->width);
    chip->y = clamp(chip->y, 0, screen->height - chip->height);

    chip->current_frame = linear_ease_in(elapsed % 1000, 0, chip->nb_frames, 1000);
}

static int level_events()
{
    if (!aptMainLoop())
    {
        return 1;
    }

    hidScanInput();

    // Respond to user input
    u32 kDown = hidKeysDown();
    if (kDown & KEY_START)
    {
        return 2; // in order to return to hbmenu
    }

    // Read the CirclePad position
    hidCircleRead(&circle_pad);
    return 0;
}

static void level_one_update()
{
    u64 elapsed = osGetTime() - start_time;

    const char * method = NULL;
    for (int i = 0; i < nb_sprites; ++i)
    {
        method = sprites[i].method;
        if (NULL != method)
        {
            if (0 == strncmp(method, "UpdateHero", 10))
            {
                update_hero(&sprites[i], &screen, elapsed);
            }
        }
    }
}

void initialize(struct GameState * state)
{
    if (0 != init_rendering(&screen))
    {
        state->next = shutdown;
        return ;
    }

    JsonWrapper * json = json_create("sprites");
    if (NULL == json || 0 != load_sprites(json, &sprites, &nb_sprites))
    {
        state->next = shutdown;
    }
    else
    {
        start_time = osGetTime();
        ship = get_sprite("ship");
        state->next = level_one;
    }
    json_delete(json);
}

void level_one(struct GameState * state)
{
    if (0 != level_events())
    {
        state->next = shutdown;
    }
    else
    {
        level_one_update();

        // render(sprites, nb_sprites);
        render(ship, 1);
    }
}

void shutdown(struct GameState * state)
{
    state->next = NULL;

    for (int i = 0; i < nb_sprites; ++i)
    {
        free(sprites[i].name);
        free(sprites[i].method);
        free(sprites[i].texture);
    }
    free(sprites);
    sprites = NULL;

    shutdown_rendering();
}