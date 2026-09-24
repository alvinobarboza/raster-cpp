#include "transforms/constants.h"

float transforms::lerp(const float a, const float b, const float t) noexcept
{
    return a + (b - a) * t;
}