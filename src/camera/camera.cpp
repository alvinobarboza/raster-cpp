#include "camera/camera.h"

#include <cmath>

#include "material/color_convertion.h"
#include "transforms/constants.h"

CameraRaster::CameraRaster(
    const int width, const int height,
    const float sensitivity, const float fov,
    const float near, const float far,
    const Vec3 &position, const Vec3 &rotation)
{
    fov_angle = fov;
    fov_scale = fov_scaling(fov);
    z_near = near;
    z_far = far;
    this->sensitivity = sensitivity;
    update_view = false;
    render_depth = false;
    render_wireframe = false;

    transform.position = position;
    transform.rotation = rotation;
    transform.scale = {1,1,1};
    transform.forward_direction = {0,0,1};

    transform.update_transforms(true);
    update_frame_buffer_size(width, height);
}

void CameraRaster::update_frame_buffer_size(const int w, const  int h)
{
    width = w;
    height = h;
    half_width = static_cast<float>(width) / 2;
    half_height = static_cast<float>(height) / 2;
    aspect_ratio = static_cast<float>(width) / static_cast<float>(height);
    frame_buffer.resize(width*height, BLACK);
    depth_buffer.resize(width*height, 0.0f);

    projection_matrix.to_perspective(fov_scale, aspect_ratio, z_near, z_far);
    update_frustum();
}

void CameraRaster::clear_frame_buffer()
{
    for (int i = 0; i < frame_buffer.size(); i++) {
        frame_buffer[i] = BLACK;
        depth_buffer[i] = 0.0f;
    }
}

Vec3 CameraRaster::vertex_to_ndc(const Vec3 &vertex) const
{
    const Vec4 v4 = {vertex.x, vertex.y, vertex.z, 1.0f};
    const Vec4 clip = v4 * projection_matrix;;

    if (clip.w != 0.0f)
        return {
            clip.x/clip.w,
            clip.y/clip.w,
            ((clip.z/clip.w) + 1.0f) * 0.5f // normalizing to 0 <> 1
        };

    return {clip.x,clip.y,clip.z};
}

Vec2 CameraRaster::ndc_to_screen(const Vec3 &point) const
{
    return
    {
        (point.x + 1.0f) * half_width,
        (1.0f - point.y) * half_height
    };
}

FullTriangle CameraRaster::project_triangle(
    const Vertex &v1, const Vertex &v2, const Vertex &v3, const MaterialRaster& material) const
{
    FullTriangle tri = {v1, v2, v3, material};

    tri.ndc_points[0] = vertex_to_ndc(v1.point);
    tri.ndc_points[1] = vertex_to_ndc(v2.point);
    tri.ndc_points[2] = vertex_to_ndc(v3.point);

    tri.screen_points[0] = ndc_to_screen(tri.ndc_points[0]);
    tri.screen_points[1] = ndc_to_screen(tri.ndc_points[1]);
    tri.screen_points[2] = ndc_to_screen(tri.ndc_points[2]);

    tri.calculate_tri_aabb();

    return tri;
}

bool CameraRaster::depth_pass(const int x, const int y, const float depth)
{
    const int index = y * width + x;
    if (depth < depth_buffer[index]) return false;

    depth_buffer[index] = depth;
    return true;
}

void CameraRaster::put_pixel(const int x, const int y, const Vec4 &color)
{
    frame_buffer[y * width + x] = color_convertion::vec4_to_color(color);
}

void CameraRaster::move_forward_backwards(const float unit)
{
    // transformations are stored to apply during projection, not really for camera movement.
    const Matrix4x4 rot_mat = transform.rotation_matrix.transpose();

    const Vec3 direction = transform.forward_direction * rot_mat;
    const Vec3 normalized_dir = direction.normalized();

    transform.position += normalized_dir * unit;
    transform.update_transforms(true);
}

void CameraRaster::move_left_right(const float unit)
{
    const Matrix4x4 rot_mat = transform.rotation_matrix.transpose();

    const Vec3 direction = transform.forward_direction * rot_mat;
    const Vec3 cross_up = direction.cross({0.0f, 0.1f, 0.0f});
    const Vec3 normalized_dir = cross_up.normalized();

    transform.position += normalized_dir * unit;
    transform.update_transforms(true);
}

void CameraRaster::move_up_down(const float unit)
{
    transform.position.y += unit;
    transform.update_transforms(true);
}

void CameraRaster::update_rotation(const Vec2 &rotation)
{
    constexpr auto amplifier = 4.0f;

    transform.rotation.x -= rotation.y * sensitivity * amplifier;
    transform.rotation.y -= rotation.x * sensitivity * amplifier;

    if (transform.rotation.x > 89) transform.rotation.x = 89;
    if (transform.rotation.x < -89) transform.rotation.x = -89;

    transform.update_transforms(true);
}

void CameraRaster::toggle_render_depth()
{
    render_depth = !render_depth;
}

void CameraRaster::toggle_view_lock()
{
    update_view = !update_view;
}

void CameraRaster::toggle_wireframe()
{
    render_wireframe = !render_wireframe;
}

void CameraRaster::toggle_render_normal()
{
    render_normal = !render_normal;
}

void CameraRaster::update_frustum()
{
    const Vec3 cam_front = transform.forward_direction;
    const Vec3 cam_right = {1.0f, 0.0f, 0.0f};
    const Vec3 cam_up = {0.0f, 1.0f, 0.0f};
    const Vec3 cam_pos = {};

    const float half_v_side = z_far * std::tan(fov_angle * transforms::DEG_TO_RAD * 0.5f);
    const float half_h_side = half_v_side * aspect_ratio;
    const Vec3 cam_front_scaled = cam_front * z_far;

    frustum.planes[NEAR_PLANE] = {cam_pos + cam_front * z_near, cam_front};
    frustum.planes[FAR_PLANE] = {cam_pos + cam_front_scaled, -cam_front};
    frustum.planes[RIGHT_PLANE] = {cam_pos, (cam_front_scaled + cam_right * half_h_side).cross(cam_up)};
    frustum.planes[LEFT_PLANE] = {cam_pos, cam_up.cross(cam_front_scaled - cam_right * half_h_side)};
    frustum.planes[TOP_PLANE] = {cam_pos, (cam_front_scaled - cam_up * half_v_side).cross(cam_right)};
    frustum.planes[BOTTOM_PLANE] = {cam_pos, cam_right.cross(cam_front_scaled - cam_up * half_v_side)};
}

void CameraRaster::handle_input()
{
    if (IsKeyPressed(KEY_TAB))
    {
        toggle_view_lock();
        update_view ? DisableCursor() : EnableCursor();
    }

    if (IsKeyPressed(KEY_X))
    {
        toggle_wireframe();
    }

    if (IsKeyPressed(KEY_Z))
    {
        toggle_render_depth();
    }

    if (IsKeyPressed(KEY_N))
    {
        toggle_render_normal();
    }


    const float delta_time = GetFrameTime();

    if (IsKeyDown(KEY_SPACE))
    {
        move_up_down(sensitivity * delta_time);
    }

    if (IsKeyDown(KEY_LEFT_CONTROL))
    {
        move_up_down(-sensitivity * delta_time);
    }

    if (IsKeyDown(KEY_W))
    {
        move_forward_backwards(sensitivity * delta_time);
    }

    if (IsKeyDown(KEY_S))
    {
        move_forward_backwards(-sensitivity * delta_time);
    }

    if (IsKeyDown(KEY_A))
    {
        move_left_right(sensitivity * delta_time);
    }

    if (IsKeyDown(KEY_D))
    {
        move_left_right(-sensitivity * delta_time);
    }

    if (update_view) {
        const auto [x,y] = GetMouseDelta();
        const Vec2 mouse_delta{x*delta_time,y*delta_time};
        update_rotation(mouse_delta);
    }

}

static float fov_scaling(const float angle) {
    return 1 / std::tan(angle*transforms::DEG_TO_RAD/2);
}
