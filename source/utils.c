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