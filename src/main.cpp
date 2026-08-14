#include <iostream>

#include "raylib.h"
#include "../cmake-build-release/_deps/raylib-src/src/raymath.h"
#include "camera/camera.h"
#include "engine/renderer.h"
#include "engine/resourse_manager.h"
#include "engine/scene.h"

int main() {
    constexpr auto width = 800;
    constexpr auto height = 800;

    CameraRaster camera = {
        width, height, 10.0f, 70, 0.2, 15,
        {2.16f, 1.12f, -0.67f}, {-23.71f, 44.57f, 0.0f}
    };

    SceneRaster scene = {
        camera,
        {}
    };

    ResourceManager rm;

    scene.models.push_back(rm.LoadModel("../assets/cube.obj"));
    scene.models[0]->transforms.position = {0.0f, 0.0f, 1.5f};
    scene.models[0]->transforms.scale = {1.0f, 1.0f, 1.0f};
    scene.models[0]->update_transforms();

    scene.models.push_back(rm.LoadModel("../assets/polyhaven_rico_b3d/marble_bust.obj"));
    scene.models[1]->transforms.position = {0.0f, 0.3f, 3.0f};
    scene.models[1]->update_transforms();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(width, height, "Hello window");
    SetTargetFPS(60);

    const auto img = GenImageColor(camera.width, camera.height, RAYWHITE);
    auto render_texture = LoadTextureFromImage(img);

    DisableCursor();
    while (!WindowShouldClose()) {
        camera.handle_input();

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

            DrawText("raster", GetScreenWidth() - 70, GetScreenHeight()-20, 20, DARKGRAY);
            DrawFPS(10, 20);
            DrawText(
                TextFormat("Camera:\n %02.2f Y: %02.2f Z: %02.2f\n X: %02.2f' Y: %02.2f' Z: %02.2f'",
                    camera.transform.position.x, camera.transform.position.y, camera.transform.position.z,
                    camera.transform.rotation.x, camera.transform.rotation.y, camera.transform.rotation.z),
                10, 40, 20, DARKGRAY);

        EndDrawing();
        //break;
    }

    CloseWindow();
    UnloadTexture(render_texture);
    UnloadImage(img);

    return 0;
}
