#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

bool is_pow_2(uint32_t i)
{
    return i && !(i & (i - 1));
}

uint32_t next_pow_2(uint32_t i)
{
    i--;
    i |= i >> 1;
    i |= i >> 2;
    i |= i >> 4;
    i |= i >> 8;
    i |= i >> 16;
    i++;

    return (i < 64)
        ? 64 : i;
}

float linear_ease_in(int t, int d)
{
    return (float)t / d;
}

int clamp(int value, int min, int max)
{
    return value < min ? min : value > max ? max : value;
}

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
