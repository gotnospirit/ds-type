#ifndef _JSON_H_
#define _JSON_H_

#include "gason.h"

class Json
{
public:
    static JsonValue Find(JsonNode const *, const char *);
    static const char * GetString(JsonNode const *, const char *);
    static int GetNumber(JsonNode const *, const char *);
    static int Size(JsonNode const *);
    static int Size(Json const *);

    ~Json();

    bool parse(const char *);

    JsonValue value;

private:
    char * source = 0;
    char * endptr = 0;
    JsonAllocator allocator;
};

#endif