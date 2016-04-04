#include "update.h"
#include "input.h"
#include "utils.h"

static float linear_ease_in(int t, int d)
{
    return (float)t / d;
}

static int clamp(int value, int min, int max)
{
    return value < min ? min : value > max ? max : value;
}

void update_hero(Sprite * sprite, uint8_t nb_frames, uint32_t max_width, uint32_t max_height, uint64_t current_time)
{
    int const incr = 5;

    float const x_incr = stick_dx() * incr;
    float const y_incr = stick_dy() * incr;

    int const idle_frame = nb_frames / 2;
    int const duration = 1000;

    int current_frame = sprite->current_frame;
    uint64_t timestamp = sprite->timestamp;
    int x = sprite->x;
    int y = sprite->y;
    int start_at = 0;
    int end_at = 0;

    if (x_incr)
    {
        x = clamp(x + x_incr, 0, max_width);
    }

    if (y_incr)
    {
        y = clamp(y + y_incr, 0, max_height);

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
        if (!timestamp)
        {
            timestamp = current_time;
        }

        u64 elapsed = current_time - timestamp;
        if (elapsed >= duration)
        {
            current_frame = end_at;
        }
        else
        {
            float progress = linear_ease_in(elapsed, duration);
            if (progress > 1)
            {
                // avoid frame overflow
                progress = 1;
            }

            current_frame = start_at + progress * (end_at - start_at);
        }
    }
    else
    {
        timestamp = 0;
    }

    sprite->x = x;
    sprite->y = y;
    sprite->current_frame = current_frame;
    sprite->timestamp = timestamp;
}