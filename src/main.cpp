#include <chrono>
#include <iostream>

#include "raylib.h"
#include "camera/camera.h"
#include "engine/renderer.h"
#include "engine/resourse_manager.h"
#include "engine/scene.h"
#include "material/color_convertion.h"

int main() {
    constexpr auto width = 1000;
    constexpr auto height = 1000;

    constexpr auto resolution_factor = 2;

    CameraRaster camera{
         2.0f, 53, 0.2, 15,
        {0.0f, .25f, 0.75f}, {-18.0f, 0.0f, 0.0f}
    };

    SceneRaster scene = {
        camera,
        {
            nullptr,
            {},
            0.1f
        }
    };

    scene.lights.emplace_back(
        LightType::DIRECTIONAL,
        color_convertion::color_to_vec4(WHITE),
        4.0f,
        Vec3(-1.0f, -1.0f, 1.0f).normalized());

    RendererRaster renderer{width, height, resolution_factor};
    camera.update_aspect_ratio(renderer.viewport.aspect_ratio());

    ResourceManager rm;

    scene.models.push_back(rm.load_model("../assets/sample_normal/sample_normal.obj", true));
    scene.models[0]->transforms.position = {0.0f, -0.25f, 2.5f};
    scene.models[0]->transforms.scale = {.5f, .5f, .5f};
    scene.models[0]->update_transforms();

    scene.models.push_back(rm.load_model("../assets/polyhaven_rico_b3d/marble_bust.obj", true));
    scene.models[1]->transforms.position = {0.0f, -0.25f, 1.5f};
    scene.models[1]->update_transforms();

    scene.models.push_back(rm.load_model("../assets/cube.obj", true));
    scene.models[2]->transforms.position = {2.0f, -0.25f, 1.5f};
    scene.models[2]->transforms.scale = {.5f, .5f, .5f};
    scene.models[2]->update_transforms();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(width, height, "Software renderer");
    SetTargetFPS(45);

    auto img = GenImageColor(renderer.viewport.width, renderer.viewport.height, RAYWHITE);
    auto render_texture = LoadTextureFromImage(img);

    while (!WindowShouldClose()) {
        const auto w = GetScreenWidth();
        const auto h = GetScreenHeight();

        if (IsWindowResized())
        {
            renderer.viewport.update_frame_buffer_size(w, h);
            UnloadTexture(render_texture);
            ImageResize(&img, renderer.viewport.width, renderer.viewport.height);
            render_texture = LoadTextureFromImage(img);
            camera.update_aspect_ratio(renderer.viewport.aspect_ratio());
        }

        renderer.handle_input();
        camera.handle_input();

        const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        renderer.render_scene(&scene);
        const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

        UpdateTexture(render_texture, renderer.viewport.frame_buffer_data());

        BeginDrawing();
            ClearBackground(RAYWHITE);

            DrawTexturePro(
                render_texture,
                {0.0f, 0.0f, static_cast<float>(renderer.viewport.width), static_cast<float>(renderer.viewport.height)},
                {0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h)},
                { 0.0f, 0.0f },
                0,
                WHITE
            );

            DrawText("raster", w - 70, h - 20, 20, DARKGRAY);
            DrawText(
                TextFormat("Canvas: %dx%d Screen: %dx%d", renderer.viewport.width, renderer.viewport.height, w, h),
                0,h - 20, 20, DARKGRAY);
            DrawFPS(10, 20);

            DrawText(
                TextFormat("Camera:\n %02.2f Y: %02.2f Z: %02.2f\n X: %02.2f' Y: %02.2f' Z: %02.2f'",
                    camera.transform.position.x, camera.transform.position.y, camera.transform.position.z,
                    camera.transform.rotation.x, camera.transform.rotation.y, camera.transform.rotation.z),
                10, 60, 20, DARKGRAY);
            DrawText(
                TextFormat("RenderMode: %s", renderer.renderer_mode().c_str() ),
                10, 125, 20, DARKGRAY
                );
            DrawText(
                TextFormat(
                    "Frame time: %d MS",
                    std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count()
                ),
                10, 150, 20, DARKGRAY);
        EndDrawing();
        //break;
    }

    CloseWindow();
    UnloadTexture(render_texture);
    UnloadImage(img);

    return 0;
}
