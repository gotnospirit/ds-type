#include "update.h"
#include "input.h"
#include "texture.h"

static float linear_ease_in(int t, int d)
{
    float b = 0.0f;
    float c = 1.0f;
    float result = c * t / d + b;
    if (result > 1)
    {
        result = 1;
    }
    return result;
}

static int clamp(int value, int min, int max)
{
    return value < min ? min : value > max ? max : value;
}

void update_hero(Entity * entity, Surface const * screen, uint64_t dt)
{
    Template const * template = entity->tpl;
    uint8_t nb_frames = template->nb_frames;

    int const incr = 5;

    float const x_incr = stick_dx() * incr;
    float const y_incr = stick_dy() * incr;

    int const idle_frame = nb_frames / 2;
    int const duration = 1000;

    int current_frame = entity->current_frame;
    int x = entity->x;
    int y = entity->y;
    int start_at = 0;
    int end_at = 0;

    if (x_incr)
    {
        x = clamp(x + x_incr, 0, screen->width - template->width);
    }

    if (y_incr)
    {
        y = clamp(y + y_incr, 0, screen->height - template->height);

        if (y_incr > 0)
        {
            // animate from idle to 0
            start_at = idle_frame;
            end_at = 0;
        }
        else
        {
            // animate from idle to nb_frames
            start_at = idle_frame;
            end_at = nb_frames - 1;
        }
    }
    else
    {
        // animate to idle
        start_at = current_frame;
        end_at = idle_frame;
    }

    if (start_at != end_at && current_frame != end_at)
    {
        uint16_t elapsed = entity->elapsed + dt;

        if (elapsed >= duration)
        {
            current_frame = end_at;
        }
        else
        {
            float progress = linear_ease_in(elapsed, duration);
            current_frame = start_at + progress * (end_at - start_at);
        }

        entity->elapsed = (current_frame == end_at)
            ? 0 : elapsed;
    }

    Sprite * sprite = entity->sprite;
    if (NULL != sprite)
    {
        sprite->x = x;
        sprite->y = y;

        if (entity->current_frame != current_frame)
        {
            sprite->frame = get_frame(sprite->texture, template->start_frame + current_frame);
        }
    }

    entity->x = x;
    entity->y = y;
    entity->current_frame = current_frame;
}