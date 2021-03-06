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

void entity_anchor(entity_t * target, entity_t const * base, anchor_t const anchor)
{
    if (TOP_LEFT == anchor || TOP_CENTER == anchor || TOP_RIGHT == anchor)
    {
        target->y = base->y;
    }
    else if (BOTTOM_LEFT == anchor || BOTTOM_CENTER == anchor || BOTTOM_RIGHT == anchor)
    {
        target->y = base->y + base->height;
    }
    else
    {
        target->y = base->y + (base->height - target->height) / 2;
    }

    if (TOP_LEFT == anchor || MIDDLE_LEFT == anchor || BOTTOM_LEFT == anchor)
    {
        target->x = base->x;
    }
    else if (TOP_RIGHT == anchor || MIDDLE_RIGHT == anchor || BOTTOM_RIGHT == anchor)
    {
        target->x = base->x + base->width;
    }
    else
    {
        target->x = base->x + (base->width - target->width) / 2;
    }
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