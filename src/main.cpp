#include <iostream>

#include "raylib.h"
#include "../cmake-build-release/_deps/raylib-src/src/raymath.h"
#include "camera/camera.h"
#include "engine/renderer.h"
#include "engine/resourse_manager.h"
#include "engine/scene.h"

int main() {
    constexpr auto width = 800;
    constexpr auto height = 600;

    CameraRaster camera = {
        width, height, 10.0f, 53, 0.1, 10,
        {0.0f, 1.5f, -1.0f}, {-20.0f, 0.0f, 0.0f}
    };

    SceneRaster scene = {
        camera,
        {}
    };

    const std::string path = "../assets/cube.obj";

    ResourceManager rm;
    scene.models.push_back(rm.LoadModel(path));
    scene.models[0]->transforms.position = {0.0f, 0.0f, 3.0f};
    scene.models[0]->update_transforms();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(width, height, "Hello window");
    SetTargetFPS(60);

    const auto img = GenImageColor(camera.width, camera.height, RAYWHITE);
    auto render_texture = LoadTextureFromImage(img);



    while (!WindowShouldClose()) {
        RendererRaster::render_scene(scene);

        UpdateTexture(render_texture, camera.frame_buffer.data());

        const auto w = GetScreenWidth();
        const auto h = GetScreenHeight();
        BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawTexturePro(
                render_texture,
                {0.0f, 0.0f, static_cast<float>(camera.width), static_cast<float>(camera.height)},
                {0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h)},
                Vector2Zero(),
                0,
                WHITE
            );

            DrawTextEx({},"Hello window", {static_cast<float>(w)/2 - 6*11, static_cast<float>(h)/2-10}, 20, 5, DARKGRAY);
        EndDrawing();
        //break;
    }

    CloseWindow();
    UnloadTexture(render_texture);
    UnloadImage(img);

    return 0;
}
