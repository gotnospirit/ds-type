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

void update_hero(Level const * level, Entity * entity, uint64_t dt)
{
    uint8_t nb_frames = entity->nb_frames;

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

    x += x_incr;

    if (y_incr)
    {
        y += y_incr;

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

    entity->x = x;
    entity->y = y;

    if (entity->current_frame != current_frame)
    {
        Sprite * sprite = entity->sprite;
        sprite->frame = get_frame(sprite->texture, entity->start_frame + current_frame);
        entity->current_frame = current_frame;
    }
}