#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

char * read_file(const char * filepath)
{
    char * result = 0;
	FILE * file = fopen(filepath, "rb");
    if (file)
    {
        // seek to end of file
        fseek(file, 0, SEEK_END);

        // file pointer tells us the size
        off_t size = ftell(file);

        // seek back to start
        fseek(file, 0, SEEK_SET);

        //allocate a buffer
        result = (char *)malloc(size);
        off_t bytesRead = 0;
        if (result)
        {
            //read contents !
            bytesRead = fread(result, 1, size, file);
        }

        //close the file because we like being nice and tidy
        fclose(file);

        if (size != bytesRead)
        {
            free(result);
            result = 0;
        }
    }
    return result;
}

int get_hitbox_surface(hitbox_t const * hitbox, surface_t * surface)
{
    if (NULL == hitbox)
    {
        return 0;
    }

    uint8_t nb_points = hitbox->nb_points;
    if (nb_points < 2)
    {
        return 0;
    }

    point_t const * points = hitbox->points;
    uint8_t i = 0;
    uint32_t min_x = 2 ^ 32, min_y = 2 ^ 32;
    uint32_t max_x = 0, max_y = 0;
    int x = 0, y = 0;

    for (; i < nb_points; ++i)
    {
        x = points[i].x;
        y = points[i].y;

        if (x < min_x)
        {
            min_x = x;
        }

        if (x > max_x)
        {
            max_x = x;
        }

        if (y < min_y)
        {
            min_y = y;
        }

        if (y > max_y)
        {
            max_y = y;
        }
    }

    surface->width = max_x - min_x;
    surface->height = max_y - min_y;
    return 1;
}

void apply_anchor(anchor_t anchor, int width_diff, int height_diff, int * x, int * y)
{
    switch (anchor)
    {
        case TOP_CENTER:
            *x += width_diff / 2;
            break;

        case TOP_RIGHT:
            *x += width_diff;
            break;

        case MIDDLE_LEFT:
            *y += height_diff / 2;
            break;

        case MIDDLE_CENTER:
            *x += width_diff / 2;
            *y += height_diff / 2;
            break;

        case MIDDLE_RIGHT:
            *x += width_diff;
            *y += height_diff / 2;
            break;

        case BOTTOM_LEFT:
            *y += height_diff;
            break;

        case BOTTOM_CENTER:
            *x += width_diff / 2;
            *y += height_diff;
            break;

        case BOTTOM_RIGHT:
            *x += width_diff;
            *y += height_diff;
            break;

        default:
            break;
    }
}