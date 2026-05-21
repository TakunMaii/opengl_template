#include "linear_algebra.h"

#include <math.h>

static Vec4 mat4_get_column(const Mat4 m, int column)
{
    switch (column)
    {
    case 0:
        return vec4(m.row0.x, m.row1.x, m.row2.x, m.row3.x);
    case 1:
        return vec4(m.row0.y, m.row1.y, m.row2.y, m.row3.y);
    case 2:
        return vec4(m.row0.z, m.row1.z, m.row2.z, m.row3.z);
    case 3:
        return vec4(m.row0.w, m.row1.w, m.row2.w, m.row3.w);
    default:
        return vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }
}

float radiansf(float degrees)
{
    return degrees * (LA_PI / 180.0f);
}

float clampf(float value, float min_value, float max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

float lerpf(float a, float b, float t)
{
    return a + (b - a) * t;
}

Vec2 vec2(float x, float y)
{
    return (Vec2){ .x = x, .y = y };
}

Vec3 vec3(float x, float y, float z)
{
    return (Vec3){ .x = x, .y = y, .z = z };
}

Vec4 vec4(float x, float y, float z, float w)
{
    return (Vec4){ .x = x, .y = y, .z = z, .w = w };
}

Vec2 add2(const Vec2 a, const Vec2 b)
{
    return vec2(a.x + b.x, a.y + b.y);
}

Vec3 add3(const Vec3 a, const Vec3 b)
{
    return vec3(a.x + b.x, a.y + b.y, a.z + b.z);
}

Vec4 add4(const Vec4 a, const Vec4 b)
{
    return vec4(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

Vec2 sub2(const Vec2 a, const Vec2 b)
{
    return vec2(a.x - b.x, a.y - b.y);
}

Vec3 sub3(const Vec3 a, const Vec3 b)
{
    return vec3(a.x - b.x, a.y - b.y, a.z - b.z);
}

Vec4 sub4(const Vec4 a, const Vec4 b)
{
    return vec4(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

Vec2 scale2(const Vec2 a, float scalar)
{
    return vec2(a.x * scalar, a.y * scalar);
}

Vec3 scale3(const Vec3 a, float scalar)
{
    return vec3(a.x * scalar, a.y * scalar, a.z * scalar);
}

Vec4 scale4(const Vec4 a, float scalar)
{
    return vec4(a.x * scalar, a.y * scalar, a.z * scalar, a.w * scalar);
}

float dot2(const Vec2 a, const Vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

float dot3(const Vec3 a, const Vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float dot4(const Vec4 a, const Vec4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float length_squared2(const Vec2 a)
{
    return dot2(a, a);
}

float length_squared3(const Vec3 a)
{
    return dot3(a, a);
}

float length_squared4(const Vec4 a)
{
    return dot4(a, a);
}

float length2(const Vec2 a)
{
    return sqrtf(length_squared2(a));
}

float length3(const Vec3 a)
{
    return sqrtf(length_squared3(a));
}

float length4(const Vec4 a)
{
    return sqrtf(length_squared4(a));
}

Vec2 normalize2(const Vec2 a)
{
    float len = length2(a);
    if (len <= LA_EPSILON)
    {
        return vec2(0.0f, 0.0f);
    }
    return scale2(a, 1.0f / len);
}

Vec3 normalize3(const Vec3 a)
{
    float len = length3(a);
    if (len <= LA_EPSILON)
    {
        return vec3(0.0f, 0.0f, 0.0f);
    }
    return scale3(a, 1.0f / len);
}

Vec4 normalize4(const Vec4 a)
{
    float len = length4(a);
    if (len <= LA_EPSILON)
    {
        return vec4(0.0f, 0.0f, 0.0f, 0.0f);
    }
    return scale4(a, 1.0f / len);
}

Vec3 cross(const Vec3 a, const Vec3 b)
{
    return vec3(
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    );
}

Vec2 lerp2(const Vec2 a, const Vec2 b, float t)
{
    return vec2(
        lerpf(a.x, b.x, t),
        lerpf(a.y, b.y, t)
    );
}

Vec3 lerp3(const Vec3 a, const Vec3 b, float t)
{
    return vec3(
        lerpf(a.x, b.x, t),
        lerpf(a.y, b.y, t),
        lerpf(a.z, b.z, t)
    );
}

Vec4 lerp4(const Vec4 a, const Vec4 b, float t)
{
    return vec4(
        lerpf(a.x, b.x, t),
        lerpf(a.y, b.y, t),
        lerpf(a.z, b.z, t),
        lerpf(a.w, b.w, t)
    );
}

Mat4 mat4_zero(void)
{
    return (Mat4){
        .row0 = { 0.0f, 0.0f, 0.0f, 0.0f },
        .row1 = { 0.0f, 0.0f, 0.0f, 0.0f },
        .row2 = { 0.0f, 0.0f, 0.0f, 0.0f },
        .row3 = { 0.0f, 0.0f, 0.0f, 0.0f }
    };
}

Mat4 mat4_identity(void)
{
    return (Mat4){
        .row0 = { 1.0f, 0.0f, 0.0f, 0.0f },
        .row1 = { 0.0f, 1.0f, 0.0f, 0.0f },
        .row2 = { 0.0f, 0.0f, 1.0f, 0.0f },
        .row3 = { 0.0f, 0.0f, 0.0f, 1.0f }
    };
}

Mat4 mat4_from_rows(Vec4 row0, Vec4 row1, Vec4 row2, Vec4 row3)
{
    return (Mat4){
        .row0 = row0,
        .row1 = row1,
        .row2 = row2,
        .row3 = row3
    };
}

Mat4 mat4_transpose(const Mat4 m)
{
    return mat4_from_rows(
        vec4(m.row0.x, m.row1.x, m.row2.x, m.row3.x),
        vec4(m.row0.y, m.row1.y, m.row2.y, m.row3.y),
        vec4(m.row0.z, m.row1.z, m.row2.z, m.row3.z),
        vec4(m.row0.w, m.row1.w, m.row2.w, m.row3.w)
    );
}

Mat4 mat4_mul(const Mat4 a, const Mat4 b)
{
    Vec4 b0 = mat4_get_column(b, 0);
    Vec4 b1 = mat4_get_column(b, 1);
    Vec4 b2 = mat4_get_column(b, 2);
    Vec4 b3 = mat4_get_column(b, 3);

    return mat4_from_rows(
        vec4(dot4(a.row0, b0), dot4(a.row0, b1), dot4(a.row0, b2), dot4(a.row0, b3)),
        vec4(dot4(a.row1, b0), dot4(a.row1, b1), dot4(a.row1, b2), dot4(a.row1, b3)),
        vec4(dot4(a.row2, b0), dot4(a.row2, b1), dot4(a.row2, b2), dot4(a.row2, b3)),
        vec4(dot4(a.row3, b0), dot4(a.row3, b1), dot4(a.row3, b2), dot4(a.row3, b3))
    );
}

Vec4 mat4_mul_vec4(const Mat4 m, const Vec4 v)
{
    return vec4(
        dot4(m.row0, v),
        dot4(m.row1, v),
        dot4(m.row2, v),
        dot4(m.row3, v)
    );
}

Mat4 mat4_translate(const Vec3 offset)
{
    return mat4_from_rows(
        vec4(1.0f, 0.0f, 0.0f, offset.x),
        vec4(0.0f, 1.0f, 0.0f, offset.y),
        vec4(0.0f, 0.0f, 1.0f, offset.z),
        vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
}

Mat4 mat4_scale(const Vec3 factors)
{
    return mat4_from_rows(
        vec4(factors.x, 0.0f, 0.0f, 0.0f),
        vec4(0.0f, factors.y, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, factors.z, 0.0f),
        vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
}

Mat4 mat4_rotate_x(float radians)
{
    float c = cosf(radians);
    float s = sinf(radians);

    return mat4_from_rows(
        vec4(1.0f, 0.0f, 0.0f, 0.0f),
        vec4(0.0f, c, -s, 0.0f),
        vec4(0.0f, s, c, 0.0f),
        vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
}

Mat4 mat4_rotate_y(float radians)
{
    float c = cosf(radians);
    float s = sinf(radians);

    return mat4_from_rows(
        vec4(c, 0.0f, s, 0.0f),
        vec4(0.0f, 1.0f, 0.0f, 0.0f),
        vec4(-s, 0.0f, c, 0.0f),
        vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
}

Mat4 mat4_rotate_z(float radians)
{
    float c = cosf(radians);
    float s = sinf(radians);

    return mat4_from_rows(
        vec4(c, -s, 0.0f, 0.0f),
        vec4(s, c, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, 1.0f, 0.0f),
        vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
}

Mat4 mat4_ortho(float left, float right, float bottom, float top, float near_clip, float far_clip)
{
    float width = right - left;
    float height = top - bottom;
    float depth = far_clip - near_clip;

    if (fabsf(width) <= LA_EPSILON || fabsf(height) <= LA_EPSILON || fabsf(depth) <= LA_EPSILON)
    {
        return mat4_identity();
    }

    return mat4_from_rows(
        vec4(2.0f / width, 0.0f, 0.0f, -(right + left) / width),
        vec4(0.0f, 2.0f / height, 0.0f, -(top + bottom) / height),
        vec4(0.0f, 0.0f, -2.0f / depth, -(far_clip + near_clip) / depth),
        vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
}

Mat4 mat4_perspective(float fov_y_radians, float aspect_ratio, float near_clip, float far_clip)
{
    float tan_half_fov;
    float depth = far_clip - near_clip;

    if (fabsf(aspect_ratio) <= LA_EPSILON || fabsf(depth) <= LA_EPSILON)
    {
        return mat4_identity();
    }

    tan_half_fov = tanf(fov_y_radians * 0.5f);
    if (fabsf(tan_half_fov) <= LA_EPSILON)
    {
        return mat4_identity();
    }

    return mat4_from_rows(
        vec4(1.0f / (aspect_ratio * tan_half_fov), 0.0f, 0.0f, 0.0f),
        vec4(0.0f, 1.0f / tan_half_fov, 0.0f, 0.0f),
        vec4(0.0f, 0.0f, -(far_clip + near_clip) / depth, -(2.0f * far_clip * near_clip) / depth),
        vec4(0.0f, 0.0f, -1.0f, 0.0f)
    );
}

Mat4 mat4_look_at(Vec3 eye, Vec3 target, Vec3 up)
{
    Vec3 forward = normalize3(sub3(target, eye));
    Vec3 right = normalize3(cross(forward, up));
    Vec3 camera_up = cross(right, forward);

    if (length_squared3(forward) <= LA_EPSILON || length_squared3(right) <= LA_EPSILON)
    {
        return mat4_identity();
    }

    return mat4_from_rows(
        vec4(right.x, right.y, right.z, -dot3(right, eye)),
        vec4(camera_up.x, camera_up.y, camera_up.z, -dot3(camera_up, eye)),
        vec4(-forward.x, -forward.y, -forward.z, dot3(forward, eye)),
        vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
}

const float* vec2_data(const Vec2* vec2)
{
    return &vec2->x;
}

const float* vec3_data(const Vec3* vec3)
{
    return &vec3->x;
}

const float* vec4_data(const Vec4* vec4)
{
    return &vec4->x;
}

const float* mat4_data(const Mat4* matrix)
{
    return &matrix->row0.x;
}
