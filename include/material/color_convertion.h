#pragma once

#include "raylib.h"
#include "transforms/Vec4.h"

namespace color_convertion {
    Color vec4_to_color(const Vec4 &vec);
    Vec4 color_to_vec4(const Color &color);
}
