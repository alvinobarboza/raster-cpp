#include <chrono>
#include <iostream>

#include "raylib.h"
#include "../cmake-build-release/_deps/raylib-src/src/raymath.h"
#include "camera/camera.h"
#include "engine/renderer.h"
#include "engine/resourse_manager.h"
#include "engine/scene.h"
#include "material/color_convertion.h"

int main() {
    constexpr auto width = 800;
    constexpr auto height = 800;

    CameraRaster camera = {
        width, height, 2.0f, 53, 0.2, 15,
        {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 0.0f}
    };

    SceneRaster scene = {
        camera,
        {
            nullptr,
            {},
            0.2f
        }
    };

    scene.lights.emplace_back(
        LightType::DIRECTIONAL,
        color_convertion::color_to_vec4(WHITE),
        1.0f,
        Vec3(-1.0f, -1.0f, 1.0f).normalized());

    RendererRaster renderer;

    ResourceManager rm;

    // scene.models.push_back(rm.load_model("../assets/cube.obj", true));
    // scene.models[0]->transforms.position = {0.0f, 0.0f, 1.5f};
    // scene.models[0]->transforms.scale = {1.0f, 1.0f, 1.0f};
    // scene.models[0]->update_transforms();

    scene.models.push_back(rm.load_model("../assets/polyhaven_rico_b3d/marble_bust.obj", true));
    scene.models[0]->transforms.position = {0.0f, 0.0f, 3.0f};
    scene.models[0]->update_transforms();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(width, height, "Hello window");
    SetTargetFPS(60);

    auto img = GenImageColor(camera.width, camera.height, RAYWHITE);
    auto render_texture = LoadTextureFromImage(img);

    DisableCursor();
    while (!WindowShouldClose()) {
        const auto w = GetScreenWidth();
        const auto h = GetScreenHeight();

        if (IsWindowResized())
        {
            camera.update_frame_buffer_size(w, h);
            UnloadTexture(render_texture);
            ImageResize(&img, camera.width, camera.height);
            render_texture = LoadTextureFromImage(img);
        }

        camera.handle_input();


        const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        renderer.render_scene(scene);
        const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        UpdateTexture(render_texture, camera.frame_buffer.data());

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

            DrawText("raster", w - 70, h - 20, 20, DARKGRAY);
            DrawFPS(10, 20);
            DrawText(
                TextFormat(
                    "Frame time: %d MS",
                    std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count()
                ),
                10, 40, 10, DARKGRAY);
            DrawText(
                TextFormat("Camera:\n %02.2f Y: %02.2f Z: %02.2f\n X: %02.2f' Y: %02.2f' Z: %02.2f'",
                    camera.transform.position.x, camera.transform.position.y, camera.transform.position.z,
                    camera.transform.rotation.x, camera.transform.rotation.y, camera.transform.rotation.z),
                10, 60, 20, DARKGRAY);
        EndDrawing();
        //break;
    }

    CloseWindow();
    UnloadTexture(render_texture);
    UnloadImage(img);

    return 0;
}
