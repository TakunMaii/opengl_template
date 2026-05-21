#ifndef LINEAR_ALGEBRA_H
#define LINEAR_ALGEBRA_H

#define LA_PI 3.14159265358979323846f
#define LA_EPSILON 1.0e-6f

typedef struct {
    float x;
    float y;
} Vec2;

typedef struct {
    float x;
    float y;
    float z;
} Vec3;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} Vec4;

/*
x0 y0 z0 w0
x1 y1 z1 w1
x2 y2 z2 w2
x3 y3 z3 w3
 */
typedef struct {
    Vec4 row0;
    Vec4 row1;
    Vec4 row2;
    Vec4 row3;
} Mat4;

float radiansf(float degrees);
float clampf(float value, float min_value, float max_value);
float lerpf(float a, float b, float t);

Vec2 vec2(float x, float y);
Vec3 vec3(float x, float y, float z);
Vec4 vec4(float x, float y, float z, float w);

Vec2 add2(const Vec2 a, const Vec2 b);
Vec3 add3(const Vec3 a, const Vec3 b);
Vec4 add4(const Vec4 a, const Vec4 b);

Vec2 sub2(const Vec2 a, const Vec2 b);
Vec3 sub3(const Vec3 a, const Vec3 b);
Vec4 sub4(const Vec4 a, const Vec4 b);

Vec2 scale2(const Vec2 a, float scalar);
Vec3 scale3(const Vec3 a, float scalar);
Vec4 scale4(const Vec4 a, float scalar);

float dot2(const Vec2 a, const Vec2 b);
float dot3(const Vec3 a, const Vec3 b);
float dot4(const Vec4 a, const Vec4 b);

float length_squared2(const Vec2 a);
float length_squared3(const Vec3 a);
float length_squared4(const Vec4 a);

float length2(const Vec2 a);
float length3(const Vec3 a);
float length4(const Vec4 a);

Vec2 normalize2(const Vec2 a);
Vec3 normalize3(const Vec3 a);
Vec4 normalize4(const Vec4 a);

Vec3 cross(const Vec3 a, const Vec3 b);

Vec2 lerp2(const Vec2 a, const Vec2 b, float t);
Vec3 lerp3(const Vec3 a, const Vec3 b, float t);
Vec4 lerp4(const Vec4 a, const Vec4 b, float t);

Mat4 mat4_zero(void);
Mat4 mat4_identity(void);
Mat4 mat4_from_rows(Vec4 row0, Vec4 row1, Vec4 row2, Vec4 row3);

Mat4 mat4_transpose(const Mat4 m);
Mat4 mat4_mul(const Mat4 a, const Mat4 b);
Vec4 mat4_mul_vec4(const Mat4 m, const Vec4 v);

Mat4 mat4_translate(const Vec3 offset);
Mat4 mat4_scale(const Vec3 factors);
Mat4 mat4_rotate_x(float radians);
Mat4 mat4_rotate_y(float radians);
Mat4 mat4_rotate_z(float radians);

Mat4 mat4_ortho(float left, float right, float bottom, float top, float near_clip, float far_clip);
Mat4 mat4_perspective(float fov_y_radians, float aspect_ratio, float near_clip, float far_clip);
Mat4 mat4_look_at(Vec3 eye, Vec3 target, Vec3 up);

const float* vec2_data(const Vec2* vec2);
const float* vec3_data(const Vec3* vec3);
const float* vec4_data(const Vec4* vec4);
const float* mat4_data(const Mat4* matrix);

#endif /* LINEAR_ALGEBRA_H */
