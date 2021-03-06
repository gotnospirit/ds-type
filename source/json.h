#ifndef _JSON_H_
#define _JSON_H_

#include "gason.h"

class Json
{
public:
    static JsonValue const * Find(JsonValue const &, const char *);

    static const char * GetString(JsonValue const &, const char *);
    static int GetNumber(JsonValue const &, const char *);
    static bool GetBoolean(JsonValue const &, const char *);

    static int Size(JsonValue const &);

    ~Json();

    bool parse(const char *);

    JsonValue value;

private:
    char * source = 0;
    char * endptr = 0;
    JsonAllocator allocator;
};

#endif