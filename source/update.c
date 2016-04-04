#include "update.h"
#include "inputs.h"
#include "utils.h"

void update_hero(Sprite * sprite, Surface const * screen, uint64_t current_time)
{
    int incr = 5;

    float x_incr = stick_dx() * incr;
    float y_incr = stick_dy() * incr;

    int idle_frame = sprite->nb_frames / 2;
    int duration = 1000;

    int start_at = 0;
    int end_at = 0;

    if (x_incr)
    {
        sprite->x += x_incr;
        sprite->x = clamp(sprite->x, 0, screen->width - sprite->width);
    }

    if (y_incr)
    {
        sprite->y += y_incr;
        sprite->y = clamp(sprite->y, 0, screen->height - sprite->height);

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
            end_at = sprite->nb_frames - 1;
        }
    }
    else
    {
        // animate to idle
        start_at = sprite->current_frame;
        end_at = idle_frame;
    }

    if (start_at != end_at && sprite->current_frame != end_at)
    {
        if (!sprite->timestamp)
        {
            sprite->timestamp = current_time;
        }

        u64 elapsed = current_time - sprite->timestamp;
        if (elapsed >= duration)
        {
            sprite->current_frame = end_at;
        }
        else
        {
            float progress = linear_ease_in(elapsed, duration);
            if (progress > 1)
            {
                // avoid frame overflow
                progress = 1;
            }

            sprite->current_frame = start_at + progress * (end_at - start_at);
        }
    }
    else
    {
        sprite->timestamp = 0;
    }
}