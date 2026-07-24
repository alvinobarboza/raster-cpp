#include "camera/camera.h"

#include <cmath>

#include "material/color_convertion.h"
#include "transforms/Constants.h"

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
    update_view = true;
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

    update_frustum();
}

void CameraRaster::clear_frame_buffer()
{
    for (int i = 0; i < frame_buffer.size(); i++) {
        frame_buffer[i] = BLACK;
        depth_buffer[i] = 0.0f;
    }
}

Vec2 CameraRaster::vertex_to_ndc(const Vec3 &vertex) const
{
    return
    {
        vertex.x * fov_scale / (vertex.z * aspect_ratio),
        vertex.y * fov_scale / vertex.z,
    };
}

Vec2 CameraRaster::ndc_to_screen(const Vec2 &point) const
{
    return
    {
        (point.x + 1.0f) * half_width,
        (1.0f - point.y) * half_height
    };
}

FullTriangle CameraRaster::project_triangle(
    const Vertex &v1, const Vertex &v2, const Vertex &v3,
    TextureRaster *albedo, TextureRaster *normal, TextureRaster *specular) const
{
    FullTriangle tri = {v1, v2, v3, albedo, normal, specular};

    const Vec2 va = vertex_to_ndc(v1.point);
    const Vec2 vb = vertex_to_ndc(v2.point);
    const Vec2 vc = vertex_to_ndc(v3.point);

    tri.screen_points[0] = ndc_to_screen(va);
    tri.screen_points[1] = ndc_to_screen(vb);
    tri.screen_points[2] = ndc_to_screen(vc);

    tri.calculate_tri_aabb();

    return tri;
}

bool CameraRaster::depth_pass(const int x, const int y, const float depth)
{
    if (x < 0 || x >= width || y < 0 || y >= height) return false;

    const int index = y * width + x;
    if (depth_buffer[index] > depth) return false;

    depth_buffer[index] = depth;
    return true;
}

void CameraRaster::put_pixel(const int x, const int y, const Vec4 &color, const float depth)
{
    const int index = y * width + x;

    if (index >= depth_buffer.size()) return;

    if (render_depth)
    {
        const float c = 255.0f * depth;
        const Vec4 color_f = {c,c,c,1};
        frame_buffer[index] = color_convertion::vec4_to_color(color_f);
        return;
    }
    frame_buffer[index] = color_convertion::vec4_to_color(color);
}

void CameraRaster::move_forward_backwards(const float unit)
{
    const Vec3 direction = transform.forward_direction * transform.rotation_matrix;
    const Vec3 normalized_dir = direction.normalized();

    transform.position += normalized_dir * unit;
    transform.update_transforms(true);
}

void CameraRaster::move_left_right(const float unit)
{
    const Vec3 direction = transform.forward_direction * transform.rotation_matrix;
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
    if (!update_view) return;

    transform.rotation.x -= rotation.y * sensitivity;
    transform.rotation.y += rotation.x * sensitivity;

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

static float fov_scaling(const float angle) {
    return 1 / std::tan(angle*transforms::DEG_TO_RAD/2);
}
