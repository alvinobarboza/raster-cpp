#include <chrono>
#include <iostream>

#include "raylib.h"
#include "camera/camera.h"
#include "engine/renderer.h"
#include "engine/resourse_manager.h"
#include "engine/scene.h"
#include "material/color_convertion.h"

int main() {
    constexpr auto width = 1600;
    constexpr auto height = 900;

    constexpr auto resolution_factor = 1;

    CameraRaster camera{
         2.0f, 53, 0.2, 15,
        {0.0f, 1.6f, -2.4f}, {-18.0f, 0.0f, 0.0f}
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
        Vec3(-30.0f, 45.0f, 0.0f), Vec3(5.0f,5.0f,0.0f),
        2.0f,
        20.0f,
        true);

    RendererRaster renderer{width, height, resolution_factor};
    camera.update_aspect_ratio(renderer.viewport.aspect_ratio());

    ResourceManager rm;

    scene.models.push_back(rm.load_model("../assets/sample_normal/sample_normal.obj", true));
    scene.models[0]->transforms.position = {0.0f, 0.0f, 0.0f};
    scene.models[0]->transforms.rotation = {-90.0f, 0.0f, 0.0f};
    scene.models[0]->update_transforms();

    scene.models.push_back(rm.load_model("../assets/cube.obj", true));
    scene.models[1]->transforms.position = {1.0f, 0.5f, 1.0f};
    scene.models[1]->transforms.scale = {.5f, .5f, .5f};
    scene.models[1]->update_transforms();

    scene.models.push_back(rm.load_model("../assets/polyhaven_rico_b3d/marble_bust.obj", true));
    scene.models[2]->transforms.position = {0.0f, 0.0f, 1.0f};
    scene.models[2]->update_transforms();
    //
    // scene.models.push_back(rm.load_model("../assets/polyhaven_rico_b3d/marble_bust.obj", true));
    // scene.models[3]->transforms.position = {0.0f, 0.0f, 1.5f};
    // scene.models[3]->update_transforms();
    //
    // scene.models.push_back(rm.load_model("../assets/polyhaven_rico_b3d/marble_bust.obj", true));
    // scene.models[4]->transforms.position = {-0.5f, 0.0f, 1.0f};
    // scene.models[4]->update_transforms();
    //
    // scene.models.push_back(rm.load_model("../assets/polyhaven_rico_b3d/marble_bust.obj", true));
    // scene.models[5]->transforms.position = {-0.5f, 0.0f, 1.5f};
    // scene.models[5]->update_transforms();

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(width, height, "Software renderer");
    SetTargetFPS(60);

    auto img = GenImageColor(renderer.viewport.width, renderer.viewport.height, RAYWHITE);
    auto render_texture = LoadTextureFromImage(img);
    auto roboto = LoadFont("../assets/fonts/RobotoMono-Regular.ttf");

    long long avg_time{};
    long long min_time{};
    long long max_time{};
    int frame_count {1};
    while (!WindowShouldClose())
    {
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
        const auto time = std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count();
        avg_time += time;
        if (max_time < time)
        {
            max_time = time;
        }
        if (min_time > time)
        {
            min_time = time;
        } else if (min_time == 0)
        {
            min_time = time;
        }

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

        const auto font_size {static_cast<float>(roboto.baseSize)};
        DrawTextEx(
            roboto,
            "raster",
            {static_cast<float>(w - 90), static_cast<float>(h - 30)},
            font_size,0.0f, RAYWHITE);

        Vector2 pos {10.0f, font_size*2.0f};

        DrawTextEx(
            roboto,
            TextFormat("Canvas: %dx%d Screen: %dx%d", renderer.viewport.width, renderer.viewport.height, w, h),
            {5.0f,static_cast<float>(h - roboto.baseSize)}, font_size, 0.0f, RAYWHITE);

        DrawFPS(10, 20);

        DrawTextEx( roboto,
            TextFormat("Camera:\n %02.2f Y: %02.2f Z: %02.2f\n X: %02.2f' Y: %02.2f' Z: %02.2f'",
                camera.transform.position.x, camera.transform.position.y, camera.transform.position.z,
                camera.transform.rotation.x, camera.transform.rotation.y, camera.transform.rotation.z),
            pos, font_size, 0.0f, RAYWHITE);
        pos.y += font_size * 3;
        DrawTextEx( roboto,
            TextFormat("RenderMode: %s", renderer.renderer_mode().c_str() ),
            pos, font_size, 0.0f, RAYWHITE
            );
        pos.y += font_size;
        DrawTextEx(roboto,
            TextFormat(
                "Frame time: %2dms \nAVG: %2dms \nMIN: %2dms \nMAX: %2dms ",
                time,
                 avg_time / frame_count,
                 min_time, max_time
            ),
            pos, font_size, 0.0f, RAYWHITE);
        EndDrawing();
        if (frame_count % 240 == 0) {
            avg_time = 0;
            frame_count = 0;
            min_time = 0;
            max_time = 0;
        };
        ++frame_count;
    }

    CloseWindow();
    UnloadTexture(render_texture);
    UnloadImage(img);
    UnloadFont(roboto);

    return 0;
}
