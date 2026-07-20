#include <iostream>

#include "raylib.h"

int main() {
    constexpr auto width = 800;
    constexpr auto height = 600;

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(width, height, "Hello window");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        const auto w = GetScreenWidth();
        const auto h = GetScreenHeight();
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTextEx({},"Hello window", {static_cast<float>(w)/2 - 6*11, static_cast<float>(h)/2-10}, 20, 5, DARKGRAY);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
