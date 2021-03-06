#include "easing.h"

float linear_ease_in(int t, int d)
{
    float b = 0.0f;
    float c = 1.0f;
    float result = c * t / d + b;
    if (result > 1)
    {
        result = 1;
    }
    return result;
}

float short_ease_in(int t, int d)
{
    return (t > (d / 2))
        ? 0.0f : 1.0f;
}