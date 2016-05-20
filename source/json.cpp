#include <stdlib.h>
#include <string.h>

#include "json.h"

Json::~Json()
{
    free(source);
}

bool Json::parse(const char * data)
{
    free(source);
    source = strdup(data);
    return jsonParse(source, &endptr, &value, allocator) == JSON_OK;
}

JsonValue const * Json::Find(JsonValue const &root, const char * key)
{
    for (auto const &leaf : root)
    {
        if (0 == strcmp(leaf->key, key))
        {
            return &leaf->value;
        }
    }
    return NULL;
}

const char * Json::GetString(JsonValue const &root, const char * key)
{
    auto const &node = Json::Find(root, key);
    if (NULL != node)
    {
        return node->toString();
    }
    return NULL;
}

int Json::GetNumber(JsonValue const &root, const char * key)
{
    auto const &node = Json::Find(root, key);
    if (NULL != node)
    {
        return node->toNumber();
    }
    return -1;
}

bool Json::GetBoolean(JsonValue const &root, const char * key)
{
    auto const &node = Json::Find(root, key);
    return NULL != node && JSON_TRUE == node->getTag();
}

int Json::Size(JsonValue const &value)
{
    int result = 0;
    auto iter_end = end(value);
    for (auto iter = begin(value); iter != iter_end; ++iter)
    {
        ++result;
    }
    return result;
}