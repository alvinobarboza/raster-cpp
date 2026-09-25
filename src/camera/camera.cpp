#include "camera/camera.h"

#include <cmath>

#include "material/color_convertion.h"
#include "transforms/constants.h"

static float fov_scaling(const float angle) {
    return 1 / std::tan(angle*transforms::DEG_TO_RAD/2);
}

CameraRaster::CameraRaster(
    const float sensitivity, const float fov,
    const float near, const float far,
    const Vec3 &position, const Vec3 &rotation):
fov_angle(fov),
fov_scale(fov_scaling(fov)),
aspect_ratio(1.0f),
z_near(near),
z_far(far),
sensitivity(sensitivity)
{
    transform.position = position;
    transform.rotation = rotation;
    transform.scale = {1,1,1};
    transform.forward_direction = {0,0,1};

    transform.update_transforms(true);
    update_frustum();
}

void CameraRaster::update_aspect_ratio(const float new_aspect_ratio)
{
    aspect_ratio = new_aspect_ratio;
    projection_matrix.to_perspective(fov_scale, aspect_ratio, z_near, z_far);
    update_frustum();
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

void CameraRaster::move_forward_backwards(const float unit)
{
    // transformations are stored to apply during projection, not really for camera movement.
    const Matrix4x4 rot_mat = transform.rotation_matrix;

    const Vec3 direction = transform.forward_direction * rot_mat;
    const Vec3 normalized_dir = direction.normalized();

    transform.position += normalized_dir * unit;
    transform.update_transforms(true);
}

void CameraRaster::move_left_right(const float unit)
{
    const Matrix4x4 rot_mat = transform.rotation_matrix;

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

void CameraRaster::toggle_view_lock()
{
    update_view = !update_view;
}

void CameraRaster::update_frustum()
{
    const Vec3 cam_front = transform.forward_direction;
    const Vec3 cam_right = {1.0f, 0.0f, 0.0f};
    const Vec3 cam_up = {0.0f, 1.0f, 0.0f};

    const float half_v_side = z_far * std::tan(fov_angle * transforms::DEG_TO_RAD * 0.5f);
    const float half_h_side = half_v_side * aspect_ratio;
    const Vec3 cam_front_scaled = cam_front * z_far;

    frustum.planes[NEAR_PLANE] = {z_near, cam_front};
    frustum.planes[FAR_PLANE] = {-z_far, -cam_front};
    frustum.planes[RIGHT_PLANE] = {0.0f, (cam_front_scaled + cam_right * half_h_side).cross(cam_up)};
    frustum.planes[LEFT_PLANE] = {0.0f, cam_up.cross(cam_front_scaled - cam_right * half_h_side)};
    frustum.planes[TOP_PLANE] = {0.0f, (cam_front_scaled - cam_up * half_v_side).cross(cam_right)};
    frustum.planes[BOTTOM_PLANE] = {0.0f, cam_right.cross(cam_front_scaled + cam_up * half_v_side)};
}

void CameraRaster::handle_input()
{
    if (IsKeyPressed(KEY_TAB))
    {
        toggle_view_lock();
        update_view ? DisableCursor() : EnableCursor();
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