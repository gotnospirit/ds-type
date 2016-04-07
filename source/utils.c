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