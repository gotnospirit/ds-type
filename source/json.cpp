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

JsonValue Json::Find(JsonNode const * root, const char * key)
{
    for (auto const &leaf : root->value)
    {
        if (0 == strcmp(leaf->key, key))
        {
            return leaf->value;
        }
    }
    return JsonValue(JSON_NULL);
}

const char * Json::GetString(JsonNode const * root, const char * key)
{
    auto const &node = Json::Find(root, key);
    if (node.getTag() != JSON_NULL)
    {
        return node.toString();
    }
    return NULL;
}

int Json::GetNumber(JsonNode const * root, const char * key)
{
    auto const &node = Json::Find(root, key);
    if (node.getTag() != JSON_NULL)
    {
        return node.toNumber();
    }
    return -1;
}

int Json::Size(JsonNode const * node)
{
    int result = 0;
    auto iter_end = end(node->value);
    for (auto iter = begin(node->value); iter != iter_end; ++iter)
    {
        ++result;
    }
    return result;
}

int Json::Size(Json const * o)
{
    int result = 0;
    auto iter_end = end(o->value);
    for (auto iter = begin(o->value); iter != iter_end; ++iter)
    {
        ++result;
    }
    return result;
}