#pragma once

#include "raylib.h"
#include "transforms/vec4.h"

namespace color_convertion {
    Color vec4_to_color(const vec4 &vec);
    vec4 color_to_vec4(const Color &color);
}
